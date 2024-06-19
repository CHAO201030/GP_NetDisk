#include "../include/route_task.h"
#include "../include/client_manager.h"

extern MYSQL *sql_conn;

void do_cd(client_t *client, char *cmd)
{
    /*
        1. 支持cd到当前目录的子目录 或者到上一级目录
        2. cd后要把路径拼接到当前client的 path[128] 路径中
    */

    printf("[INFO] : %s\n", cmd);

    char *target_dir = strtok(cmd, " ");
    target_dir = strtok(NULL, " ");

    train_t server_msg = {0};
    server_msg.state = CMD_CD;

    if(sql_do_cd(client->uid, &client->code, &client->pre_code, target_dir) == 0)
    {
        if(strcmp(target_dir, "..") == 0)
        {
            int path_len = strlen(client->path);

            // 截断目录
            int i = 0;
            for(i = path_len - 1; i > 0; i--)
            {
                if(client->path[i] == '/')break;
            }
            for(int j = i; j < path_len; j++)
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

    int classify = -1;

    if(strcmp(target_dir, "..") == 0)
    {
        // 去上一级目录
        if(*code == 0)
        {
            // printf("分类1\n");
            // 当前为根 不能再向上了 path : ~
            return -1;
        }
        else
        {
            // 当前不是根 并且上一级是根 path : ~/dir1 --> ~/
            if(*pre_code == 0)
            {
                // printf("分类2\n");
                *pre_code = -1;
                *code = 0;
                return 0;
            }
            else
            {
                // 当前不是根 上一级也不是根需要查数据库获得code信息 path : ~/dir1/dir2 --> ~/dir1
                // printf("分类3\n");
                sprintf(query, 
                        "SELECT code, pre_code FROM VFS WHERE owner_uid = %d AND code = %d",
                        uid, *pre_code);
                classify = 3;
            }

        }

    }
    else
    {
        // 去下一级目录
        if(*code == 0)
        {
            // 当前为根 path : ~/ --> ~/dir1
            // printf("分类4\n");
            *pre_code = 0;
            sprintf(query,
                    "SELECT code FROM VFS WHERE "
                    "owner_uid = %d AND file_name = '%s' AND pre_code < 0",
                    uid, target_dir);
            classify = 4;
        }
        else
        {
            // 当前不是根
            // printf("分类5\n");
            sprintf(query,
                    "SELECT code, pre_code FROM VFS WHERE "
                    "owner_uid = %d AND pre_code = %d AND file_name = '%s'",
                    uid, *code, target_dir);
            classify = 5;
        }
    }
    
    if(mysql_query(sql_conn, query))
    {
        printf("[INFO] : Error making query: %s\n", mysql_error(sql_conn));
    }
    else
    {
        sql_res = mysql_use_result(sql_conn);
        if(sql_res)
        {
            if((sql_row = mysql_fetch_row(sql_res)) != NULL)
            {
                switch(classify)
                {
                case 3:
                {
                    // 当前不是根 上一级也不是根需要查数据库获得code信息
                    // path : ~/dir1/dir2 --> ~/dir1
                    *code = atoi(sql_row[0]);
                    *pre_code = atoi(sql_row[1]);
                    // printf("code = %d, pre_code = %d\n",*code, *pre_code);

                    // 这个目录是根目录的直接孩子
                    if(*pre_code == -1)*pre_code = 0;
                    
                    break;
                }
                case 4:
                {
                    // 当前为根
                    // path : ~/ --> ~/dir1
                    *code = atoi(sql_row[0]);
                    break;
                }
                case 5:
                {
                    // 当前不是根
                    *code = atoi(sql_row[0]);
                    *pre_code = atoi(sql_row[1]);
                    break;
                }
                }
            }
            else
            {
                printf("[INFO] : cd %s no such directory\n", target_dir);
                mysql_free_result(sql_res);
                return -1;               
            }
        }
    }

    mysql_free_result(sql_res);

    return 0;
}
