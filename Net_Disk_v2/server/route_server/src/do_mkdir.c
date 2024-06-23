/**
 * @file do_mkdir.c
 * @author zryk (130990107@qq.com)
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

void do_mkdir(client_t *client, char *cmd)
{
    printf("> %s\n", cmd);
    char *target_dir = strtok(cmd, " ");
    target_dir = strtok(NULL, " ");
    
    train_t server_msg = {0};

    if(sql_do_mkdir(client, target_dir) == 0)
    {
        server_msg.state = CMD_MKDIR;
        sprintf(server_msg.data_buf, "SUCCEED");
        server_msg.data_len = strlen(server_msg.data_buf);
    }
    else
    {
        server_msg.state = CMD_MKDIR;
        sprintf(server_msg.data_buf, "FAILED");
        server_msg.data_len = strlen(server_msg.data_buf);
    }
    // 发送结果给客户端

    sendn(client->fd, &server_msg.data_len, sizeof(server_msg.data_len));
    sendn(client->fd, server_msg.data_buf, server_msg.data_len);
    sendn(client->fd, &server_msg.state, sizeof(server_msg.state));

    printf("mkdir command finished.\n");
}

int sql_do_mkdir(const client_t *client, const char *target_dir)
{
    char query[256] = {0};
    int pre_code = client->code == 0 ? -1 : client->code;
    memset(query, 0, sizeof(query));
    
    sprintf(query,
            "INSERT INTO VFS(pre_code, file_name, file_type, file_size, owner_uid)"
            " VALUES (%d, '%s', 'd', 0, %d)",
            pre_code, target_dir, client->uid);

    if(mysql_query(sql_conn, query))
    {
        printf("> Error! making query: %s failed.\n", mysql_error(sql_conn));
        return -1;
    } else {
        printf("> mkdir %s succeed.\n", target_dir);
    }

    return 0;    
}