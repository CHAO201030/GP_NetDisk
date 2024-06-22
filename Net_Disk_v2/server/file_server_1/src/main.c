/**
 * @file main.c
 * @author CHAO https://github.com/CHAO201030
 *         PENG https://github.com/fanyukino
 * 
 * @brief Net_Disk -> file_server1
 * @version 5.0
 * @date 2024-06-19
 * 
 * @copyright Copyright (c) 2024 Wangdao programmer training camp CPP 58th
 * 
 */

#include "../include/file_server.h"

#define THREAD_NUM "10"

int exit_pipe[2];

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

    int sfd = tcp_init(CLUSTER_1_IP, CLUSTER_1_PORT);

    int epfd = epoll_create1(0);

    int thread_num = atoi(THREAD_NUM);

    thread_pool_t *p_manager = thread_pool_init(thread_num);

    struct epoll_event evs[5] = {0};

    epoll_add(epfd, sfd);
    epoll_add(epfd, exit_pipe[0]);

    while(1)
    {
        int ready_fd_num = epoll_wait(epfd, evs, 5, -1);

        for(int i = 0; i < ready_fd_num; i++)
        {
            if(evs[i].data.fd == sfd)
            {
                /**
                 * 1. 有新客户端连接 证明该客户端已经通过了route的TOKEN验证
                 * 2. 文件服务器只是一个无情的文件收发机器
                 * 3. 目前还存在安全问题 比如有客户端通过抓包直接连文件服务器的IP和PORT
                */
                
                int new_client_fd = tcp_accept(sfd);
                if(new_client_fd == -1)continue;

                distribute_task(p_manager, new_client_fd);
                
            }
            else if(evs[i].data.fd == exit_pipe[0])
            {    
                // 接收到SIGUSR1信号 准备有序退出
                char exit_flag;
                read(exit_pipe[1], &exit_flag, 1);

                thread_pool_exit(p_manager);

                pthread_exit(0);
            }

        }// end of search evs[i].data.fd

    }// end of while(1)

    return 0;
}