#include "../include/client.h"

#define ROUTE_IP "192.168.7.121"
#define ROUTE_PORT "9527"

char cur_path[256] = {0};
char *token_key = NULL;

int main(int argc, char *argv[])
{
    start_menu();

    printf("[INFO] : connect to ROUTE_SERVER...\n");

    int route_sfd = tcp_connect(ROUTE_IP, ROUTE_PORT);

    printf("[INFO] : connect established...\n");

    // 选择功能 1.登录 2.注册 3.退出
CHOSE:    
    int target = 0;
    scanf("%d", &target);

    switch (target)
    {
    case 1:do_login(route_sfd);      break;
    case 2:
    {
        do_register(route_sfd);
        printf("[INFO] : Input cmd 1.LOGIN 2.REGISTER 3.EXIT\n");
        goto CHOSE;
    }
    case 3:
        {
            close(route_sfd);
            exit(0);
        }
    default:
        {
            printf("Invaild chose please re-enter\n");
            goto CHOSE;
        }
    }    

    int epfd = epoll_create1(0);

    epoll_add(epfd, STDIN_FILENO);
    epoll_add(epfd, route_sfd);

    struct epoll_event evs[64] = {0};

    train_t cmd_train = {0};
    
    while(1)
    {
        int ready_fd_num = epoll_wait(epfd, evs, 64, -1);

        for(int i = 0; i < ready_fd_num; i++)
        {
            if(evs[i].data.fd == STDIN_FILENO)
            {
                // 发送命令
                bzero(&cmd_train, sizeof(cmd_train));
                cmd_train.data_len = read(STDIN_FILENO, cmd_train.data_buf, BUFFER_SIZE);
                cmd_train.data_buf[--cmd_train.data_len] = '\0';  // '\n' to '\0'

                command_analyse(&cmd_train, route_sfd);
            }
            else if(evs[i].data.fd == route_sfd)
            {
                // 接收服务器返回数据
                int ret = recv_server_msg(route_sfd);

                // 对端断开处理
                if(ret == 0)
                {
                    /* 进入到这个区域表示路由服务器已经宕机 客户端把当前的GETS/PUTS任务完成后要退出进程
                     *     1. epoll取消监听route_sfd
                     *     2. close(route_sfd)
                     */
                    epoll_del(epfd, route_sfd);
                    close(route_sfd);
                }
            }
        }// end of search evs[i].data.fd
    }// end of while(1)


    /*
    char cluster_1_ip[32] = {0};
    char cluster_1_port[32] = {0};

    recv_cluster_info(route_sfd, cluster_1_ip, cluster_1_port);
    
    printf("INFO : GET Cluster Server INFO...\n"
           "       %s : %s\n", cluster_1_ip, cluster_1_port);
    
    printf("INFO : connect to CLUSTER_1...\n");
    
    int cluster_1_sfd = tcp_connect(cluster_1_ip, cluster_1_port);
    
    printf("INFO : connect established...\n");

    recv_file(cluster_1_sfd);

    printf("INFO : download success..\n");
    // ---------------------------------------------------------------
    printf("INFO : connect to CLUSTER_2...\n");    

    int cluster_2_sfd = tcp_connect("192.168.7.121", "9529");

    printf("INFO : connect established...\n");

    send_file(cluster_2_sfd);

    printf("INFO : upload success..\n");
    
    close(route_sfd);
    close(cluster_1_sfd);
    close(cluster_2_sfd);
    */
   
    return 0;
}