/**
 * @file main.c
 * @author CHAO https://github.com/CHAO201030
 * 
 * @brief Net_Disk
 * @version 5.1
 * @date 2024-07-07
 * 
 * @copyright Copyright (c) 2024 Wangdao programmer training camp CPP 58th
 * 
 */

#include "../include/net_disk.h"

#define THREAD_NUM "thread_num"
#define MAX_CAPACITY 1024

int log_fd = -1;
int epfd = -1;
int exit_pipe[2];

config_table_t *cfg_table = NULL;

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
    /* ---------------------------------------- 服务器有序退出机制 ---------------------------------------- */

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


    /* ---------------------------------------- 服务器业务流程 ---------------------------------------- */

    if(signal(SIGPIPE, SIG_IGN) == SIG_ERR)
    {
        error(-1, errno, "[INFO] : signal SIGPIPE register failed\nReason");
    }


    // 读取配置文件
    cfg_table = config_table_create();
    read_config(argv[1], cfg_table);


    // 打开日志文件
    if((log_fd = open("../log/route.log", O_RDWR|O_CREAT|O_APPEND, 0666)) == -1)
    {
        error(-1, errno, "[INFO] : log_file open failed\nReason");
    }


    // 初始化TCP监听
    int sfd = tcp_init(config_table_find(cfg_table, ROUTE_IP), 
                       config_table_find(cfg_table, ROUTE_PORT));
    

    // 连接数据库
    sql_connect_to_database();


    // 初始化epoll
    epfd = epoll_create1(0);
    struct epoll_event evs[MAX_CAPACITY] = {0};

    epoll_add(epfd, sfd);
    epoll_add(epfd, exit_pipe[0]);


    // 启动线程池
    int thread_num = atoi(config_table_find(cfg_table, THREAD_NUM));

    thread_pool_t *p_manager = thread_pool_init(thread_num);


    // 创建用户管理结构以及超时队列
    client_manage_map = hashmap_create();
    time_queue = tq_create();


    // 启动定时器
    // timer_start();


    // 事件循环 Event-Loop
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
                // 获取触发epoll的客户端
                client_t *cur_client  = get_cur_client(client_manage_map, evs[i].data.fd);

                // 接收任务信息
                train_t cmd_train = {0};
                if(recv_cmd(cur_client->fd, &cmd_train))
                {
                    // 更新客户端交互时间
                    update_client(client_manage_map, time_queue, cur_client);

                    // 构建任务结构体
                    task_t cur_task = {0};
                    cur_task.p_client = cur_client;
                    cur_task.state = cmd_train.state;
                    strcpy(cur_task.task_info, cmd_train.data_buf);

                    // 分发任务
                    epoll_del(epfd, cur_client->fd);
                    distribute_task(p_manager, cur_task);
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
