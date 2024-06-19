#include "../include/task.h"
#include "../include/net.h"
#include "../include/thread_func.h"

extern char cur_path[256];

extern char *TOKEN;

extern pthread_t pthid;

void start_menu(void)
{
    system("clear");
    printf("======================================================================\n"
           "|                                                                    |\n"
           "|                         Net Disk v5.0                              |\n"
           "|                                                                    |\n"
           "|       1.LOGIN                                                      |\n"
           "|       2.REGISTER                                                   |\n"
           "|       3.EXIT                                                       |\n"
           "|                                                                    |\n"
           "|                  Hongshan District Children's Programming Team one |\n"
           "======================================================================\n");
}

void send_cmd(train_t* cmd_train, CMD_TYPE type, int route_sfd)
{
    cmd_train->state = type;

    // 发送命令长度
    sendn(route_sfd, &cmd_train->data_len, sizeof(cmd_train->data_len));

    // 发送命令类型
    sendn(route_sfd, &cmd_train->state, sizeof(cmd_train->state));

    // 发送命令内容
    sendn(route_sfd, cmd_train->data_buf, cmd_train->data_len);
}

void command_analyse(train_t* cmd_train, int route_sfd)
{
    if(strncmp(cmd_train->data_buf, "ls", 2) == 0)
    {
        send_cmd(cmd_train, CMD_LS, route_sfd);
    }
    else if(strncmp(cmd_train->data_buf, "cd ", 3) == 0)
    {
        send_cmd(cmd_train, CMD_CD, route_sfd);
    }
    else if(strncmp(cmd_train->data_buf, "rm ", 3) == 0)
    {
        send_cmd(cmd_train, CMD_RM, route_sfd);
    }
    else if(strncmp(cmd_train->data_buf, "pwd", 3) == 0)
    {
        send_cmd(cmd_train, CMD_PWD, route_sfd);
    }
    else if(strncmp(cmd_train->data_buf, "mkdir ", 6) == 0)
    {
        send_cmd(cmd_train, CMD_MKDIR, route_sfd);
    }
    else if(strncmp(cmd_train->data_buf, "rmdir ", 6) == 0)
    {
        send_cmd(cmd_train, CMD_RMDIR, route_sfd);
    }
    else if(strncmp(cmd_train->data_buf, "gets ", 5) == 0)
    {
        /**
         * 1. 创建子线程 让子线程connect服务器
         * 2. 子线程发送token验证
         * 3. 子线程发送gets命令火车
         * 4. 子线程收取文件服务器地址
         * 5. 子线程后台下载文件
        */

        thread_args pth_args = {0};
        strncpy(pth_args.cur_cmd, cmd_train->data_buf, cmd_train->data_len);
        pth_args.state = CMD_GETS;

        pthread_create(&pthid, NULL, pth_func, (void *)&pth_args);   
        
        printf("%s", cur_path);
        fflush(stdout);
    }
    else if(strncmp(cmd_train->data_buf, "puts ", 5) == 0)
    {
        /**
         * 1. 创建子线程 让子线程connect服务器
         * 2. 子线程发送token验证
         * 3. 子线程发送puts命令火车
         * 4. 子线程收取文件服务器地址
         * 5. 子线程后台上传文件
        */

        thread_args pth_args = {0};
        strncpy(pth_args.cur_cmd, cmd_train->data_buf, cmd_train->data_len);
        pth_args.state = CMD_PUTS;

        pthread_create(&pthid, NULL, pth_func, (void *)&pth_args);   

        printf("%s", cur_path);
        fflush(stdout);
    }
    else if(strncmp(cmd_train->data_buf, "exit", 4) == 0)
    {
        send_cmd(cmd_train, CMD_EXIT, route_sfd);
        exit(0);
    }
    else
    {
        printf("[INFO] : ERROR command reinput...\n");
        printf("%s", cur_path);
        fflush(stdout);
    }
}

int recv_server_msg(int route_sfd)
{
    train_t server_msg = {0};
    
    // 接收消息长度
    int ret = recvn(route_sfd, &server_msg.data_len, sizeof(server_msg.data_len));

    if(ret == 0)
    {
        printf("\n[INFO] : Route Server shut down client will cose after GETS/PUTS finish\n");
        return 0;
    }

    recvn(route_sfd, &server_msg.state, sizeof(server_msg.state));
    recvn(route_sfd, server_msg.data_buf, server_msg.data_len);

    switch (server_msg.state)
    {
    case CMD_LS         :do_ls(server_msg);         break;
    case CMD_CD         :do_cd(server_msg);         break;
    case CMD_RM         :do_rm(server_msg);         break;
    case CMD_PWD        :do_pwd(server_msg);        break;
    case CMD_MKDIR      :do_mkdir(server_msg);      break;
    case CMD_RMDIR      :do_rmdir(server_msg);      break;
    default:
        break;
    }

    return 1;
}
