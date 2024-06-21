/**
 * @file main.c
 * @author CHAO https://github.com/CHAO201030
 *         PENG https://github.com/fanyukino
 * 
 * @brief Net_Disk
 * @version 5.0
 * @date 2024-06-19
 * 
 * @copyright Copyright (c) 2024 Wangdao programmer training camp CPP 58th
 * 
 */

#include "../include/net_disk.h"

#define THREAD_NUM "5"
#define MAX_CAPACITY 1024

int log_fd = -1;

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

    if((log_fd = open("../log/log.txt", O_RDWR|O_CREAT|O_APPEND, 0666)) == -1)
    {
        error(-1, errno, "[INFO] : log_file open failed\nReason");
    }

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
                        // 长命令
                        
                        /**
                         * 1. 构造task_t
                         * 2. 子线程 该干活啦! 你不干有的是线程干!
                         * 3. 取消对客户端子线程的sfd的监听
                         * 4. 由服务器子线程将客户端子线程踢出服务器
                        */
                        epoll_del(epfd, cur_client->fd);

                        task_t cur_task = {0};
                        cur_task.fd = cur_client->fd;
                        cur_task.state = cmd_train.state;
                        strncpy(cur_task.task_info, cmd_train.data_buf, sizeof(cur_task.task_info));
                        
                        distribute_task(p_manager, cur_task);
                    }
                    else
                    {
                        // 短命令

                        int ret = cmd_analyse(cur_client, cmd_train);
                        if(ret == CLIENT_EXIT)
                        {
                            // 客户端exit命令
                            LOG_INFO("user %s %s\n", cur_client->name, "exit");
                            epoll_del(epfd, cur_client->fd);
                            close(cur_client->fd);
                            del_client(client_manage_map, time_queue, cur_client);
                        }
                        else if(ret == TOKEN_FAILED)
                        {
                            // 客户端子线程TOKEN验证失败
                            epoll_del(epfd, cur_client->fd);
                            close(cur_client->fd);
                            del_client(client_manage_map, time_queue, cur_client);
                        }
                    }
                }
                else
                {
                    // 对端断开了
                    LOG_INFO("user %s %s\n", cur_client->name, "exit");
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
