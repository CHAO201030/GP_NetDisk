#include "../include/route_task.h"
#include "../include/client_manager.h"

extern MYSQL *sql_conn;

extern int log_fd;

void do_rm(client_t *client, char *cmd)
{
    /*
        1. 将数据库中该文件的引用计数变为0
    */
    printf("[INFO] : %s\n", cmd);
    char *target_file = strtok(cmd, " ");
    target_file = strtok(NULL, " ");

    train_t server_msg = {0};
    server_msg.state = CMD_RM;

    if(sql_do_rm(client, target_file) == 0)
    {
        sprintf(server_msg.data_buf, "rm %s success", target_file);
        server_msg.data_len = strlen(server_msg.data_buf);

        // 打印日志
        LOG_INFO("user %s rm %s in %s\n", client->name, target_file, client->path);
    }
    else
    {
        sprintf(server_msg.data_buf, "rm %s failed no such file", target_file);
        server_msg.data_len = strlen(server_msg.data_buf);
    }

    sendn(client->fd, &server_msg.data_len, sizeof(server_msg.data_len));
    sendn(client->fd, &server_msg.state, sizeof(server_msg.state));
    sendn(client->fd, server_msg.data_buf, server_msg.data_len);
}

int sql_do_rm(const client_t *client, const char *target_file)
{
    MYSQL_RES *sql_res = NULL;
    MYSQL_ROW sql_row;

    char query[512] = {0};
    int pre_code = client->code == 0 ? -1 : client->code;

    // 先对文件的存在性进行判断
    sprintf(query,
            "SELECT file_name FROM VFS"
            " WHERE pre_code = %d AND file_name = '%s' AND owner_uid = %d AND file_type = 'f' AND file_count = 1",
            pre_code, target_file, client->uid);

    if(mysql_query(sql_conn, query))
    {
        printf("[INFO] : Error making query: %s\n", mysql_error(sql_conn));
        return -1;
    }
    else
    {
        sql_res = mysql_use_result(sql_conn);
        if(sql_res)
        {
            if((sql_row = mysql_fetch_row(sql_res)) != NULL)
            {
                // 同一个连接 第二次QUERY之前一定要释放掉上一次的结果
                // 不然会出现 Error making query: Commands out of sync; you can't run this command now
                mysql_free_result(sql_res);
                // 用户目录下存在该文件 将引用计数变为0标记为已删除
                bzero(query, sizeof(query));
                sprintf(query,
                        "UPDATE VFS SET file_count = 0"
                        " WHERE pre_code = %d AND file_name = '%s' AND owner_uid = %d AND file_type = 'f'",
                        pre_code, target_file, client->uid);

                if(mysql_query(sql_conn, query))
                {
                    printf("[INFO] : Error making query: %s\n", mysql_error(sql_conn));
                    return -1;
                }
                else
                {
                    printf("[INFO] : <%s> rm %s success\n", client->name, target_file);
                    return 0;
                }
            }
            else
            {
                printf("[INFO] : <%s> rm failed no such file\n", client->name);
                mysql_free_result(sql_res);
                return -1;
            }
            
        }
    }

    mysql_free_result(sql_res);
    return -1;
}
