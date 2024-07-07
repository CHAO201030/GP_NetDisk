#include "../include/route_task.h"
#include "../include/client_manager.h"

extern MYSQL *sql_conn;

extern int log_fd;

void do_cd(client_t *client, char *cmd)
{
    /*
        CD 操作
            1. cd到上一级目录
                (1) cur_path : ~/
                            根目录不能再向上cd了
                (2) cur_path : ~/dir1 --> ~/
                            cd到根目录
                (3) cur_path : ~/dir1/dir2 --> ~/dir1
                            cd到上一级目录 需要查询数据库获得 code 和 pre_code 信息
                            
            2. cd到下一级目录
                (1) cur_path : ~/ --> ~/dir1
                (2) cur_path : ~/dir1 --> ~/dir1/dir2
                            cd到下一级目录

            3. cd到不存在的目录   
    */

    char *target_dir = strtok(cmd, " ");
    target_dir = strtok(NULL, " ");

    train_t server_msg = {0};
    server_msg.state = CMD_CD;

    if(sql_do_cd(client, target_dir) == 0)
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
        
        // 打印日志
        LOG_INFO("user %s cd %s\n", client->name, client->path);
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

    return;
}

int sql_do_cd(client_t *client, const char *target_dir)
{
    MYSQL_RES *sql_res = NULL;
    MYSQL_ROW sql_row;

    char query[512] = {0};

    int classify = -1;

    if(strcmp(target_dir, "..") == 0)
    {
        // 去上一级目录
        if(client->code == 0)
        {
            // 当前为根 不能再向上了 path : ~
            return -1;
        }
        else
        {
            // 当前不是根 上一级是根 path : ~/dir1 --> ~/
            if(client->pre_code == 0)
            {
                client->pre_code = -1;
                client->code = 0;

                return 0;
            }
            else
            {
                // 当前不是根 上一级也不是根需要查数据库获得code信息 path : ~/dir1/dir2 --> ~/dir1
                sprintf(query, 
                        "SELECT code, pre_code FROM VFS WHERE owner_uid = %d AND code = %d AND file_type = 'd'",
                        client->uid, client->pre_code);
                classify = 3;
            }

        }
    }
    else
    {
        // 去下一级目录
        if(client->code == 0)
        {
            // 当前为根 path : ~/ --> ~/dir1
            client->pre_code = 0;
            sprintf(query,
                    "SELECT code FROM VFS WHERE "
                    "owner_uid = %d AND file_name = '%s' AND pre_code < 0 AND file_type = 'd'",
                    client->uid, target_dir);
            classify = 4;
        }
        else
        {
            // 当前不是根
            sprintf(query,
                    "SELECT code, pre_code FROM VFS WHERE "
                    "owner_uid = %d AND pre_code = %d AND file_name = '%s' AND file_type = 'd'",
                    client->uid, client->code, target_dir);
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
                    client->code = atoi(sql_row[0]);
                    client->pre_code = atoi(sql_row[1]);

                    // 这个目录是根目录的直接子目录
                    if(client->pre_code == -1)client->pre_code = 0;
                    
                    break;
                }
                case 4:
                {
                    // 当前为根
                    // path : ~/ --> ~/dir1
                    client->code = atoi(sql_row[0]);
                    break;
                }
                case 5:
                {
                    // 当前不是根
                    client->code = atoi(sql_row[0]);
                    client->pre_code = atoi(sql_row[1]);
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