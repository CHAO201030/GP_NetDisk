#include "../include/route_task.h"
#include "../include/client_manager.h"

extern HashMap *client_manage_map;

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

int cmd_analyse(client_t *cur_client, train_t cmd_train)
{

    switch (cmd_train.state)
    {
    case CMD_LS         :do_ls      (cur_client, cmd_train.data_buf);   break;
    case CMD_CD         :do_cd      (cur_client, cmd_train.data_buf);   break;
    case CMD_RM         :do_rm      (cur_client, cmd_train.data_buf);   break;
    case CMD_PWD        :do_pwd     (cur_client, cmd_train.data_buf);   break;
    case CMD_MKDIR      :do_mkdir   (cur_client, cmd_train.data_buf);   break;
    case CMD_RMDIR      :do_rmdir   (cur_client, cmd_train.data_buf);   break;
    case CMD_LOGIN      :do_login   (cur_client, cmd_train.data_buf);   break;
    case CMD_REGISTER   :do_register(cur_client, cmd_train.data_buf);   break;
    case CMD_TOKEN      :
    {
        if(do_token(cur_client, cmd_train.data_buf) == -1)
        {
            return TOKEN_FAILED;
        }
        break;
    }
    case CMD_EXIT       :return CLIENT_EXIT;
    default:
        break;
    }

    return NORMAL_RETURN;
}
