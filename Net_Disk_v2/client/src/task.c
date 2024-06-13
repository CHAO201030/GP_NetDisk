#include "../include/task.h"

extern char cur_path[256];
extern char *token_key;

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
        do_gets(cmd_train);
    }
    else if(strncmp(cmd_train->data_buf, "puts ", 5) == 0)
    {
        do_puts();
    }
    else if(strncmp(cmd_train->data_buf, "exit", 4) == 0)
    {
        send_cmd(cmd_train, CMD_EXIT, route_sfd);
        exit(0);
    }
    else
    {
        printf("INFO : ERROR command reinput...\n");
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
        printf("[INFO] : Route Server shut down client will cose after GETS/PUTS finish\n");
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
    // case CMD_GETS       :do_gets(server_msg);       break;
    // case CMD_PUTS       :do_puts(server_msg);       break;
    case CMD_MKDIR      :do_mkdir(server_msg);      break;
    case CMD_RMDIR      :do_rmdir(server_msg);      break;
    case CMD_TOKEN      :do_token(server_msg);      break;
    default:
        break;
    }

    return 1;
}

void do_ls(train_t server_msg)
{

}

void do_cd(train_t server_msg)
{

}

void do_rm(train_t server_msg)
{

}

void do_pwd(train_t server_msg)
{

}

void *pth_func(void *args)
{


}

void do_gets(train_t* cmd_train)
{


}

void do_puts()
{

}

void do_mkdir(train_t server_msg)
{

}

void do_rmdir(train_t server_msg)
{

}

void do_token(train_t server_msg)
{

}

void do_login(int route_sfd)
{

}

void do_register(int route_sfd)
{
   
}