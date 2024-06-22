#include "../include/route_task.h"
#include "../include/client_manager.h"

extern MYSQL *sql_conn;

extern int log_fd;

void do_mkdir(client_t *client, char *cmd)
{
    /*
        MKDIR 操作
            在当前工作目录下查找是否有同名的文件或目录
                1. 有同名(TODO BUG IN HERE)
                    告诉客户端创建失败
                2. 没有同名
                    在 VFS 表中创建一个表示目录的表项 并将其 pre_code 设置为 client->code 
    */

    char *target_dir = strtok(cmd, " ");
    target_dir = strtok(NULL, " ");
    
    train_t server_msg = {0};

    if(sql_do_mkdir(client, target_dir) == 0)
    {
        server_msg.state = CMD_MKDIR;
        sprintf(server_msg.data_buf, "SUCCESS");
        server_msg.data_len = strlen(server_msg.data_buf);
        
        // 打印日志
        LOG_INFO("user %s mkdir %s in %s\n", client->name, target_dir, client->path);
    }
    else
    {
        server_msg.state = CMD_MKDIR;
        sprintf(server_msg.data_buf, "ERROR");
        server_msg.data_len = strlen(server_msg.data_buf);
    }

    sendn(client->fd, &server_msg.data_len, sizeof(server_msg.data_len));
    sendn(client->fd, &server_msg.state, sizeof(server_msg.state));
    sendn(client->fd, server_msg.data_buf, server_msg.data_len);
}

int sql_do_mkdir(const client_t *client, const char *target_dir)
{
    char query[512] = {0};
    int pre_code = client->code == 0 ? -1 : client->code;

    sprintf(query,
            "INSERT INTO VFS(pre_code, file_name, file_type, file_size, file_size_1,file_size_2,file_md5, owner_uid)"
            " VALUES (%d, '%s', 'd', 0, 0, 0,'0', %d)",
            pre_code, target_dir, client->uid);

    if(mysql_query(sql_conn, query))
    {
        printf("[INFO] : Error making query: %s\n", mysql_error(sql_conn));
        return -1;
    }
    else
    {
        printf("[INFO] : <%s> mkdir %s success\n", client->name, target_dir);
    }

    return 0;    
}
