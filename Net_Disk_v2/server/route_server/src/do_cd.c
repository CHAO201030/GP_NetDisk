/**
 * @file do_cd.c
 * @author liuhuicong(you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-06-20
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "../include/route_task.h"
#include "../include/client_manager.h"

extern MYSQL *sql_conn;

void do_cd(client_t *client, char *cmd)
{

    //  1. 支持cd到当前目录的子目录 或者到上一级目录
    // 2. cd后要把路径拼接到当前client的 path[128] 路径中

    printf("[INFO] : %s\n", cmd);

    char *target_dir = strtok(cmd, " ");
    target_dir = strtok(NULL, " ");

    train_t server_msg = {0};
    server_msg.state = CMD_CD;

    if (sql_do_cd(client->uid, &client->code, &client->pre_code, target_dir) == 0)
    {
        if (strcmp(target_dir, "..") == 0)
        {
            int path_len = strlen(client->path);

            // 截断目录
            int i = 0;
            for (i = path_len - 1; i > 0; i--)
            {
                if (client->path[i] == '/')
                    break;
            }
            for (int j = i; j < path_len; j++)
            {
                client->path[j] = '\0';
            }
        }
        else
        {
            // cd 到子目录

            // 拼接目录
            strcat(client->path, "/");
            strcat(client->path, target_dir);
        }

        server_msg.data_len = strlen(client->path);
        strncpy(server_msg.data_buf, client->path, server_msg.data_len);
    }
    else
    {
        // 不合法的路径
        sprintf(server_msg.data_buf, "ERROR");
        server_msg.data_len = strlen(server_msg.data_buf);
        printf("%s\n", server_msg.data_buf);
    }
    // 发送当前路径给客户端
    sendn(client->fd, &server_msg.data_len, sizeof(server_msg.data_len));
    sendn(client->fd, &server_msg.state, sizeof(server_msg.state));
    sendn(client->fd, server_msg.data_buf, server_msg.data_len);
}

int sql_do_cd(int uid, int *code, int *pre_code, const char *target_dir)
{
    MYSQL_RES *sql_res = NULL;
    MYSQL_ROW sql_row;

    char query[512] = {0};
    int cd_type = -1;

    // cd到上级目录
    if (strcmp(target_dir, "..") == 0)
    {
        // 0.precode=-1 code=0 # ~
        if (*code == 0)
        {
            cd_type = 0;
            return -1;
        }
        else
        {
            // 1.precode=0 # ~/dir1 -> ~
            if (*pre_code == 0)
            {
                cd_type = 1;
                *code = 0;
                *pre_code = -1;
                return 0;
            }

            // 2. precode!=0 # ~/dir1/dir2 ->~/dir1
            else
            {
                cd_type = 2;
                sprintf(query,
                        "SELECT code, pre_code FROM VFS WHERE owner_uid = %d AND code = %d",
                        uid, *pre_code);
            }
        }
    }
    else // cd到下级目录
    {
        // 3.code=0 # ~/ -> ~/dir1
        if (*code == 0)
        {
            cd_type = 3;
            *pre_code = 0;
            sprintf(query,
                    "SELECT code FROM VFS WHERE owner_uid = %d AND pre_code = -1 AND file_name='%s'",
                    uid, target_dir);
        }
        // 4.code!=0 # ~/dir1 -> ~/dir1/dir2
        else
        {
            cd_type = 4;
            sprintf(query,
                    "SELECT code,pre_code,file_name FROM VFS WHERE owner_uid = %d AND file_name = '%s'",
                    uid, target_dir);
        }
    }
    // printf("query:%s\n",query);
    if (mysql_query(sql_conn, query))
    {
        printf("query error: %s\n", mysql_error(sql_conn)); // 打印错误信息
        return -1;
    }

    sql_res = mysql_store_result(sql_conn);
    if (sql_res == NULL)
    {
        printf("store result error\n");
        mysql_free_result(sql_res);
        return -1;
    }

    if ((sql_row = mysql_fetch_row(sql_res)) != NULL)
    {
        switch (cd_type)
        {
        case 2:
            *code = atoi(sql_row[0]);
            *pre_code = atoi(sql_row[1]);
            if (*pre_code == -1)
            {
                *pre_code = 0;
            }
            break;
        case 3:
            *code = atoi(sql_row[0]);
            break;
        case 4:
            *code = atoi(sql_row[0]);
            *pre_code = atoi(sql_row[1]);
            if (strcmp(target_dir, sql_row[2]) == 0)
            {
                printf("[ERROR]: dir repeatable\n");
                mysql_free_result(sql_res);
                return -1;
            }
            break;
        }
    }
    else
    {
        printf("ERROR: mysql fetch fail\n");
        mysql_free_result(sql_res);
        return -1;
    }
    mysql_free_result(sql_res);

    return 0;
}
