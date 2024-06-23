/**
 * @file do_pwd.c
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-06-19
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "../include/route_task.h"
#include "../include/client_manager.h"

extern MYSQL *sql_conn;

void do_pwd(client_t *client, char *cmd)
{
    printf(">password:%s\n", cmd);

    train_t server_msg = {0};
    server_msg.state = CMD_PWD;
    server_msg.data_len = strlen(client->path);
    strncpy(server_msg.data_buf, client->path, server_msg.data_len);

    sendn(client->fd, &server_msg.data_len, sizeof(server_msg.data_len));
    sendn(client->fd, &server_msg.state, sizeof(server_msg.state));
    sendn(client->fd, server_msg.data_buf, server_msg.data_len);
}
