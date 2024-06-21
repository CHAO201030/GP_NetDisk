#include "../include/route_task.h"
#include "../include/client_manager.h"

extern MYSQL *sql_conn;

extern int log_fd;

void do_rmdir(client_t *client, char *cmd)
{
    char *target_dir = strtok(cmd, " ");
    target_dir = strtok(NULL, " ");

    train_t server_msg = {0};
    server_msg.state = CMD_RMDIR;

    switch (sql_do_rmdir(client, target_dir))
    {
    case 0:
    {
        // 成功删除
        sprintf(server_msg.data_buf,"rmdir %s success", target_dir);
        server_msg.data_len = strlen(server_msg.data_buf);
        
        // 打印日志
        LOG_INFO("user %s rmdir %s in %s\n", client->name, target_dir, client->path);
        break;
    }
    case 1:
    {
        // 目录不为空
        sprintf(server_msg.data_buf,"rmdir %s failed : directory not empty", target_dir);
        server_msg.data_len = strlen(server_msg.data_buf); 
        break;
    }
    case 2:
    {
        // 没有该目录
        sprintf(server_msg.data_buf,"rmdir %s failed : no such directory", target_dir);
        server_msg.data_len = strlen(server_msg.data_buf);
        break;
    }
    default:
    {
        sprintf(server_msg.data_buf, "Unknow error");
        server_msg.data_len = strlen(server_msg.data_buf);
        break;
    }
    }

    // 发送结果给客户端
    sendn(client->fd, &server_msg.data_len, sizeof(server_msg.data_len));
    sendn(client->fd, &server_msg.state, sizeof(server_msg.state));
    sendn(client->fd, server_msg.data_buf, server_msg.data_len);
}

int sql_do_rmdir(const client_t *client, const char *target_dir)
{
    MYSQL_RES *sql_res = NULL;
    MYSQL_ROW sql_row;

    char query[512] = {0};
    int pre_code = client->code == 0 ? -1 : client->code;

    // 先对目录的存在性进行判断
    sprintf(query,
            "SELECT code FROM VFS"
            " WHERE pre_code = %d AND file_name = '%s' AND owner_uid = %d AND file_type = 'd' AND file_count = 1",
            pre_code, target_dir, client->uid);

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
            int target_dir_code = atoi(sql_row[0]);
            mysql_free_result(sql_res);
            
            // 有该目录 判断是否为空
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
                    // 目录不为空
                    printf("[INFO] : <%s> rmdir %s failed directory not empty\n", client->name, target_dir);
                    mysql_free_result(sql_res);
                    return 1;
                }
                else
                {
                    // 目录为空直接删除
                    mysql_free_result(sql_res);
                    bzero(query, sizeof(query));
                    sprintf(query,
                        "DELETE FROM VFS"
                        " WHERE pre_code = %d AND owner_uid = %d AND file_name = '%s' AND file_type = 'd'",
                        pre_code, client->uid, target_dir);
                    if(mysql_query(sql_conn, query) == 0)
                    {
                        printf("[INFO] : <%s> rmdir %s success\n", client->name, target_dir);
                        return 0;
                    }
                    else
                    {
                        printf("[INFO] : Error making query: %s\n", mysql_error(sql_conn));
                        return -1;
                    }
                }
            }
        }
        else
        {
            // 没有该目录
            printf("[INFO] : <%s> rmdir %s failed no such directory\n", client->name, target_dir);
            mysql_free_result(sql_res);
            return 2;
        }
    }
}
