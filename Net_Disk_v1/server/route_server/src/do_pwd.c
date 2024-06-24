#include "../include/route_task.h"
#include "../include/client_manager.h"

extern MYSQL *sql_conn;

extern int log_fd;

void do_pwd(client_t *client, char *cmd)
{
    /*
        PWD 操作
            返回 client->path 的内容给客户端
    */

    train_t server_msg = {0};
    server_msg.state = CMD_PWD;
    server_msg.data_len = strlen(client->path);
    strncpy(server_msg.data_buf, client->path, server_msg.data_len);

    sendn(client->fd, &server_msg.data_len, sizeof(server_msg.data_len));
    sendn(client->fd, &server_msg.state, sizeof(server_msg.state));
    sendn(client->fd, server_msg.data_buf, server_msg.data_len);

    // 打印日志
    LOG_INFO("user %s pwd\n", client->name);
}
