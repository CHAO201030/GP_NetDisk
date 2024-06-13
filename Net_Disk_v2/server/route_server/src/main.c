#include "../include/net_disk.h"

#define THREAD_NUM "5"
#define MAX_CAPACITY 1024

int exit_pipe[2];

int epfd;

void sig_func(int sig_num)
{
    char exit_flag = 1;
    write(exit_pipe[1], &exit_flag, 1);
    printf("INFO : capture signal SIGUSR1\n");
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

    int sfd = tcp_init(ROUTE_IP, ROUTE_PORT);

    epfd = epoll_create1(0);

    int thread_num = atoi(THREAD_NUM);

    thread_pool_t *p_manager = thread_pool_init(thread_num);

    struct epoll_event evs[MAX_CAPACITY] = {0};

    epoll_add(epfd, sfd);
    epoll_add(epfd, exit_pipe[0]);

    sql_connect_to_database();

    while(1)
    {
        int ready_fd_num = epoll_wait(epfd, evs, MAX_CAPACITY, -1);

        for(int i = 0; i < ready_fd_num; i++)
        {
            if(evs[i].data.fd == sfd)
            {
                // 有新客户端连接
                client_t new_client = {0};
                
                new_client.fd = tcp_accept(sfd);
                
                if(new_client.fd != -1)
                {
                    /*  客户端成功连接到服务器
                     *    1. epoll监听客户端的fd
                     *    2. 客户端结构放进用户管理表中 
                     *    3. 让子线程去处理短命令的请求
                     */
                    epoll_add(epfd, new_client.fd);
                    distribute_task(p_manager, new_client);
                }
            }
            else if(evs[i].data.fd == exit_pipe[0])
            {    
                char exit_flag;
                read(exit_pipe[1], &exit_flag, 1);

                thread_pool_exit(p_manager);

                sql_disconnect_to_database();
                pthread_exit(0);
            }
        }// end of search evs[i].data.fd

    }// end of while(1)
    
    sql_disconnect_to_database();

    return 0;
}