#include "../include/route_task.h"
#include "../include/client_manager.h"
#include "../include/net.h"

extern HashMap *client_manage_map;
extern time_out_queue *time_queue;
extern int epfd;

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

int cmd_analyse(client_t *cur_client, task_t cur_task)
{
    switch (cur_task.state)
    {
    case CMD_LS         :do_ls      (cur_client, cur_task.task_info);   break;
    case CMD_CD         :do_cd      (cur_client, cur_task.task_info);   break;
    case CMD_RM         :do_rm      (cur_client, cur_task.task_info);   break;
    case CMD_PWD        :do_pwd     (cur_client, cur_task.task_info);   break;
    case CMD_MKDIR      :do_mkdir   (cur_client, cur_task.task_info);   break;
    case CMD_RMDIR      :do_rmdir   (cur_client, cur_task.task_info);   break;
    case CMD_LOGIN      :do_login   (cur_client, cur_task.task_info);   break;
    case CMD_REGISTER   :do_register(cur_client, cur_task.task_info);   break;
    case CMD_TOKEN      :
    {
        if(do_token(cur_client, cur_task.task_info) == -1)
        {
            return TOKEN_FAILED;
        }
        break;
    }
    case CMD_GETS:
    {
        do_gets(cur_client, cur_task.task_info);

        // 服务器不再对客户端子线程进行管理
        del_client(client_manage_map, time_queue, cur_client);
        epoll_del(epfd, cur_client->fd);
        close(cur_client->fd);

        break;
    }
    case CMD_PUTS:
    {
        do_puts(cur_client, cur_task.task_info);

        // 服务器不再对客户端子线程进行管理
        del_client(client_manage_map, time_queue, cur_client);
        epoll_del(epfd, cur_client->fd);
        close(cur_client->fd);
        break;
    }
    case CMD_EXIT:return CLIENT_EXIT;
    default:
        break;
    }

    return NORMAL_RETURN;
}
