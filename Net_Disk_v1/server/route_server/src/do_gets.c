#include "../include/route_task.h"
#include "../include/client_manager.h"

extern MYSQL *sql_conn;

extern int log_fd;

void do_gets(client_t *client, char *cmd)
{
    char *target_file = strtok(cmd, " ");
    target_file = strtok(NULL, " ");

    bool is_valid = false;

    unsigned char md5sum[33] = {0};

    char ip1[16] = {0};
    char port1[5] = {0};
    int part1_size = 0;

    char ip2[16] = {0};
    char port2[5] = {0};
    int part2_size = 0;

    if(sql_do_gets(client, target_file, md5sum,
                   ip1, port1, &part1_size,
                   ip2, port2, &part2_size) == 0)
    {       
        is_valid = true;
        sendn(client->fd, &is_valid, sizeof(is_valid));

        // 发送文件的md5
        sendn(client->fd, md5sum, sizeof(md5sum));
        // 发送文件服务器1的 {IP, PORT, Part1_size}
        sendn(client->fd, ip1, sizeof(ip1));
        sendn(client->fd, port1, sizeof(port1));
        sendn(client->fd, &part1_size, sizeof(part1_size));
        // 发送文件服务器2的 {IP, PORT, Part1_size}
        sendn(client->fd, ip2, sizeof(ip2));
        sendn(client->fd, port2, sizeof(port2));
        sendn(client->fd, &part2_size, sizeof(part2_size));
        
        // 打印日志
        LOG_INFO("user %s download %s in %s\n", client->name, target_file, client->path);
    }
    else
    {
        sendn(client->fd, &is_valid, sizeof(is_valid));
    }
}

int sql_do_gets(const client_t *client, const char *target_file, unsigned char *md5sum, \
                char *ip1, char *port1, int *part1_size, \
                char *ip2, char *port2, int *part2_size)
{
    MYSQL_RES *sql_res = NULL;
    MYSQL_ROW sql_row;
    
    int pre_code = client->code == 0 ? -1 : client->code;

    char query[512] = {0};
    sprintf(query,
            "SELECT file_md5,"
            " cluster_ip_1, cluster_port_1, file_size_1,"
            " cluster_ip_2, cluster_port_2, file_size_2"
            " FROM VFS WHERE owner_uid = %d AND file_type = 'f' AND file_name = '%s' AND pre_code = %d",
            client->uid, target_file, pre_code);
    
    if(mysql_query(sql_conn, query))
    {
        printf("Error making query: %s\n", mysql_error(sql_conn));
    }
    else
    {
        sql_res = mysql_use_result(sql_conn);
        if(sql_res)
        {        
            while((sql_row = mysql_fetch_row(sql_res)) != NULL)
            {
                strncpy((char *)md5sum, sql_row[0], 32);
                
                strncpy(ip1, sql_row[1], 16);
                strncpy(port1, sql_row[2], 4);
                *part1_size = atoi(sql_row[3]);

                strncpy(ip2, sql_row[4], 16);
                strncpy(port2, sql_row[5], 4);
                *part2_size = atoi(sql_row[6]);
            }
        }
        else
        {
            mysql_free_result(sql_res);

            return -1;
        }
    }
    
    mysql_free_result(sql_res);

    return 0; 
}
