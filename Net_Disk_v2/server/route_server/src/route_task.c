#include "../include/route_task.h"

int recv_cmd(int client_fd, train_t *cmd_train)
{
    // 接收命令长度
    int ret = recvn(client_fd, &cmd_train->data_len, sizeof(cmd_train->data_len));

    if(ret == 0)return 0;   // 对端断开处理
    
    // 接收命令类型
    recvn(client_fd, &cmd_train->state, sizeof(cmd_train->state));

    // 接收命令内容
    recvn(client_fd, cmd_train->data_buf, cmd_train->data_len);

    return 1;
}

void command_analyse(client_t *client)
{
    while(1)
    {
        train_t cmd_train = {0};
        int ret = recv_cmd(client->fd, &cmd_train);

        if(ret == 0)return;

        switch (cmd_train.state)
        {
        case CMD_LS         :do_ls      (client, cmd_train.data_buf);   break;
        case CMD_CD         :do_cd      (client, cmd_train.data_buf);   break;
        case CMD_RM         :do_rm      (client, cmd_train.data_buf);   break;
        case CMD_PWD        :do_pwd     (client, cmd_train.data_buf);   break;
        case CMD_GETS       :do_gets    (client, cmd_train.data_buf);   break;
        case CMD_PUTS       :do_puts    (client, cmd_train.data_buf);   break;
        case CMD_MKDIR      :do_mkdir   (client, cmd_train.data_buf);   break;
        case CMD_RMDIR      :do_rmdir   (client, cmd_train.data_buf);   break;
        case CMD_TOKEN      :do_token   (client, cmd_train.data_buf);   break;
        case CMD_LOGIN      :do_login   (client, cmd_train.data_buf);   break;
        case CMD_REGISTER   :do_register(client, cmd_train.data_buf);   break;
        case CMD_EXIT       :return;
        default:
            break;
        }
    }
}

void do_ls(client_t *client, char *cmd)
{

}

void do_cd(client_t *client, char *cmd)
{
 
}

void do_rm(client_t *client, char *cmd)
{

}

void do_pwd(client_t *client, char *cmd)
{

}

void do_gets(client_t *client, char *cmd)
{

}

void do_puts(client_t *client, char *cmd)
{


}

void do_mkdir(client_t *client, char *cmd)
{

}

void do_rmdir(client_t *client, char *cmd)
{

}

void do_token(client_t *client, char *cmd)
{

}

void do_login(client_t *client, char *cmd)
{
 
}

void generate_salt(char *salt)
{
    // 用户注册使用
    char str[9] = {0};
    int i, flag;
    srand(time(NULL));
    for(i = 0; i < 8; i++)
    {
        flag = rand()%3;
        switch(flag)
        {
        case 0:str[i] = rand() % 26 + 'a'; break;
        case 1:str[i] = rand() % 26 + 'A'; break;
        case 2:str[i] = rand() % 10 + '0'; break;

        }
    }
    strcat(salt, "$6$");
    strcat(salt, str);
    strcat(salt, "$");
}

void do_register(client_t *client, char *cmd)
{

}