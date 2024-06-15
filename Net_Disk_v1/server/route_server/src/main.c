#include "../include/net_disk.h"

#define THREAD_NUM "5"
#define MAX_CAPACITY 1024

int exit_pipe[2];

HashMap *client_manage_map = NULL;

time_out_queue *time_queue = NULL;

void sig_func(int sig_num)
{
    char exit_flag = 1;
    write(exit_pipe[1], &exit_flag, 1);
    printf("[INFO] : capture signal SIGUSR1\n");
}

int main(int argc, char* argv[])
{
    /* -------------------- 服务器有序退出机制 -------------------- */

    pipe(exit_pipe);

    if(fork() != 0)
    {
        // 父进程接收退出信号 异步拉同步
        close(exit_pipe[0]);

        if(signal(SIGUSR1, sig_func) == SIG_ERR)
        {
            error(-1, errno, "[INFO] : signal SIGUSR1 register failed\nReason");
        }

        wait(NULL);

        exit(0);
    }
    
    close(exit_pipe[1]);


    /* -------------------- 服务器业务流程 -------------------- */

    if(signal(SIGPIPE, SIG_IGN) == SIG_ERR)
    {
        error(-1, errno, "[INFO] : signal SIGPIPE register failed\nReason");
    }

    timer_start();

    int sfd = tcp_init(ROUTE_IP, ROUTE_PORT);

    int epfd = epoll_create1(0);

    int thread_num = atoi(THREAD_NUM);

    thread_pool_t *p_manager = thread_pool_init(thread_num);

    struct epoll_event evs[MAX_CAPACITY] = {0};

    epoll_add(epfd, sfd);
    epoll_add(epfd, exit_pipe[0]);

    sql_connect_to_database();

    client_manage_map = hashmap_create();
    time_queue = tq_create();

    while(1)
    {
        int ready_fd_num = epoll_wait(epfd, evs, MAX_CAPACITY, -1);

        for(int i = 0; i < ready_fd_num; i++)
        {
            if(evs[i].data.fd == sfd)
            {
                // 有新客户端连接 分配到堆上
                client_t *new_client = calloc(1, sizeof(client_t));

                new_client->fd = tcp_accept(sfd);

                if(new_client->fd == -1)continue;

                add_client(client_manage_map, time_queue, new_client);

                epoll_add(epfd, new_client->fd);
            }
            else if(evs[i].data.fd == exit_pipe[0])
            {    
                // 接收到SIGUSR1信号 准备有序退出
                char exit_flag;
                read(exit_pipe[1], &exit_flag, 1);

                thread_pool_exit(p_manager);

                sql_disconnect_to_database();

                pthread_exit(0);
            }
            else
            {
                client_t *cur_client  = get_cur_client(client_manage_map, evs[i].data.fd);

                update_client(client_manage_map, time_queue, cur_client);

                train_t cmd_train = {0};
                if(recv_cmd(cur_client->fd, &cmd_train))
                {
                    if(cmd_train.state == CMD_GETS || cmd_train.state == CMD_PUTS)
                    {
                        distribute_task(p_manager, *cur_client);
                    }
                    else
                    {
                        if(cmd_analyse(cur_client, cmd_train))
                        {
                            // 客户端exit命令
                            printf("client fd = %d exit\n", cur_client->fd);
                            epoll_del(epfd, cur_client->fd);
                            close(cur_client->fd);
                            del_client(client_manage_map, time_queue, cur_client);
                        }
                    }
                }
                else
                {
                    // 对端断开了
                    printf("client fd = %d exit\n", cur_client->fd);
                    epoll_del(epfd, cur_client->fd);
                    close(cur_client->fd);
                    del_client(client_manage_map, time_queue, cur_client);
                }
            }
        }// end of search evs[i].data.fd

    }// end of while(1)
    
    sql_disconnect_to_database();

    return 0;
}