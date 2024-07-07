#include "../include/route_task.h"
#include "../include/client_manager.h"

extern MYSQL *sql_conn;

extern int log_fd;

void do_ls(client_t *client, char *cmd)
{
    /*
        LS 操作
            查询数据库 获得 VFS 中所有 pre_code 为当前 client->code 且 is_valid = 1 的所有表项
    */

    // 发送消息 客户端跳转到recv_server_msg函数执行
    train_t server_msg = {0};
    server_msg.state = CMD_LS;
    if((sql_do_ls(client, server_msg.data_buf)) == 0)
    {
        server_msg.data_len = strlen(server_msg.data_buf);

        // 发送结果给客户端
        sendn(client->fd, &server_msg.data_len, sizeof(server_msg.data_len));
        sendn(client->fd, &server_msg.state, sizeof(server_msg.state));
        sendn(client->fd, server_msg.data_buf, server_msg.data_len);

        // 打印日志
        LOG_INFO("user %s ls %s\n", client->name, client->path);
    }

    return;
}

int sql_do_ls(const client_t* client, char *server_msg)
{
    MYSQL_RES *sql_res = NULL;
    MYSQL_ROW sql_row;

    int pre_code = client->code == 0 ? -1 : client->code;

    char query[512] = {0};
    
    sprintf(query, 
            "SELECT time, file_name, file_type, file_size FROM VFS WHERE"
            " owner_uid = %d AND pre_code = %d AND file_count = 1",
            client->uid, pre_code);

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
            while((sql_row = mysql_fetch_row(sql_res)) != NULL)
            {
                // time name type size
                strcat(server_msg, sql_row[0]);
                strcat(server_msg, "  ");

                // 目录高亮显示
                if(strcmp(sql_row[2], "d") == 0)
                {
                    strcat(server_msg, "\033[32m");
                    strcat(server_msg, sql_row[1]);
                    strcat(server_msg, "\033[0m");
                }
                else
                {
                    strcat(server_msg, sql_row[1]);
                }

                strcat(server_msg, "\t");
                strcat(server_msg, sql_row[2]);
                strcat(server_msg, "  ");
                strcat(server_msg, sql_row[3]);
                strcat(server_msg, "\n");
            }
            server_msg[strlen(server_msg) - 1] = '\0';
        }
    }

    mysql_free_result(sql_res);

    return 0;    
}
