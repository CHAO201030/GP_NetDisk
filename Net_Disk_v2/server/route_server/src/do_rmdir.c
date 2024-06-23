/**
 * @file do_rmdir.c
 * @author zryk（130990107@qq.com)
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

void do_rmdir(client_t *client, char *cmd)
{
    printf("> %s\n", cmd);
    // 分解字符串
    char *target_dir = strtok(cmd, " ");
    target_dir = strtok(NULL, " ");
    
    train_t server_msg = {0};
    server_msg.state = CMD_RMDIR;

    switch(sql_do_rmdir(client, target_dir)){
    case 0: {
        // 成功删除
        sprintf(server_msg.data_buf, "SUCCEED");
        server_msg.data_len = strlen(server_msg.data_buf);
        break;
    }
    case 1: {
        // 目录非空
        sprintf(server_msg.data_buf, "FAILED: directory not empty!");
        server_msg.data_len = strlen(server_msg.data_buf);
        break;
    }
    case 2: {
        // 目录不存在
        sprintf(server_msg.data_buf, "FAILED: no such directory!");
        server_msg.data_len = strlen(server_msg.data_buf);
        break;
    }
    default: {
        // 其它
        sprintf(server_msg.data_buf, "Error!");
        server_msg.data_len = strlen(server_msg.data_buf);
        break;
    }
    }
    // 发送结果给客户端
    
    sendn(client->fd, &server_msg.data_len, sizeof(server_msg.data_len));
    sendn(client->fd, server_msg.data_buf, server_msg.data_len);
    sendn(client->fd, &server_msg.state, sizeof(server_msg.state));

    printf("rmdir command finished.\n");
}

int sql_do_rmdir(const client_t *client, const char *target_dir)
{
    MYSQL_RES *sql_res = NULL;
    MYSQL_ROW sql_row;
    char query[256] = {0};
    int pre_code = client->code == 0 ? -1 : client->code;
    memset(query, 0, sizeof(query));
    
    // 判断目录是否存在
    sprintf(query,
            "SELECT code FROM VFS"
            " WHERE pre_code = %d AND file_name = '%s' AND owner_uid = %d AND file_type = 'd' AND file_count = 1",
            pre_code, target_dir, client->uid);

    if(mysql_query(sql_conn, query)) {
        printf("> Error! making query: %s failed.\n", mysql_error(sql_conn));
        return -1;
    } else {
        sql_res = mysql_use_result(sql_conn);
        if((sql_row = mysql_fetch_row(sql_res)) != NULL) {
    int target_dir_code = atoi(sql_row[0]);
            mysql_free_result(sql_res);
            
            // 有该目录，判断是否为空
            bzero(query, sizeof(query));
            sprintf(query,
                    "SELECT code FROM VFS"
                    " WHERE pre_code = %d AND owner_uid = %d",
                    target_dir_code, client->uid);

            if(mysql_query(sql_conn, query))
            {
                printf("[INFO] : Error making query: %s\n", mysql_error(sql_conn));
                return -1;
            }
            else
            {
                sql_res = mysql_use_result(sql_conn);
                if((sql_row = mysql_fetch_row(sql_res)) != NULL)
                {
                    // 目录非空
                    printf("> rmdir %s failed! Directory not empty\n", target_dir);
                    mysql_free_result(sql_res);
                    return 1;
                }
                else
                {
                    // 目录为空，直接删除
                    mysql_free_result(sql_res);
                    bzero(query, sizeof(query));
                    sprintf(query,
                        "DELETE FROM VFS"
                        " WHERE pre_code = %d AND owner_uid = %d AND file_name = '%s' AND file_type = 'd'",
                        pre_code, client->uid, target_dir);
                    if(mysql_query(sql_conn, query) == 0)
                    {
                        printf("> rmdir %s succeed.\n", target_dir);
                        return 0;
                    }
                    else
                    {
                        printf("> Error making query: %s\n", mysql_error(sql_conn));
                        return -1;
                    }
                }
            }
        }
        else
        {
            // 没有该目录
            printf("> rmdir %s failed! No such directory.\n", target_dir);
            mysql_free_result(sql_res);
            return 2;
        }
    }
}
