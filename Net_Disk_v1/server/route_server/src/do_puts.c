#include "../include/route_task.h"
#include "../include/client_manager.h"

extern MYSQL *sql_conn;

extern int log_fd;

void load_balance(char* ip1, char *port1, char *ip2, char* port2)
{
    srand(time(NULL));
    int fuck_net_disk = rand() % 3;
    switch (fuck_net_disk)
    {
    case 0:
    {
        sprintf(ip1,   "%s", CLUSTER_1_IP);
        sprintf(port1, "%s", CLUSTER_1_PORT);
        sprintf(ip2,   "%s", CLUSTER_2_IP);
        sprintf(port2, "%s", CLUSTER_2_PORT);
        break;
    }
    case 1:
    {
        sprintf(ip1,   "%s", CLUSTER_3_IP);
        sprintf(port1, "%s", CLUSTER_3_PORT);
        sprintf(ip2,   "%s", CLUSTER_1_IP);
        sprintf(port2, "%s", CLUSTER_1_PORT);
        break;
    }
    case 2:
    {
        sprintf(ip1,   "%s", CLUSTER_2_IP);
        sprintf(port1, "%s", CLUSTER_2_PORT);
        sprintf(ip2,   "%s", CLUSTER_3_IP);
        sprintf(port2, "%s", CLUSTER_3_PORT);
        break;
    }
    default:
        break;
    }
    return;
}

void do_puts(client_t *client, char *cmd)
{
    char *file_name = strtok(cmd, " ");
    file_name = strtok(NULL, " ");
    
    char md5sum[33] = {0};
    recvn(client->fd, md5sum, sizeof(md5sum));

    bool quick_trans = false;
    int ret = -1;
    if((ret = sql_is_file_exist(md5sum)) == 0)
    {
        quick_trans = true;
        sendn(client->fd, &quick_trans, sizeof(quick_trans));
        sql_do_quick_puts(client, md5sum, file_name);
        
        // 打印日志
        LOG_INFO("user %s quick upload %s in %s\n", client->name, file_name, client->path);
    }
    else if(ret == 1)
    {
        sendn(client->fd, &quick_trans, sizeof(quick_trans));

        char ip1[16]  = {0};
        char ip2[16]  = {0};
        char port1[5] = {0};
        char port2[5] = {0};

        load_balance(ip1, port1, ip2, port2);
        // 发送文件服务器信息
        sendn(client->fd, ip1, sizeof(ip1));
        sendn(client->fd, ip2, sizeof(ip2));
        sendn(client->fd, port1, sizeof(port1));
        sendn(client->fd, port2, sizeof(port2));

        // 接收文件的大小 以及分片的大小
        off_t file_size  = 0;
        int part1_size = 0;
        int part2_size = 0;

        recvn(client->fd, &file_size,  sizeof(file_size));
        recvn(client->fd, &part1_size, sizeof(part1_size));
        recvn(client->fd, &part2_size, sizeof(part2_size));


        sql_do_puts(client, file_name, md5sum, file_size, \
                    ip1, port1, part1_size, \
                    ip2, port2, part2_size);
        
        // 打印日志
        LOG_INFO("user %s upload %s in %s\n", client->name, file_name, client->path);
    }
    return;
}

int sql_is_file_exist(const char *md5sum)
{
    MYSQL_RES *sql_res = NULL;
    MYSQL_ROW sql_row;

    char query[512] = {0};

    sprintf(query, \
            "SELECT file_name "
            " FROM VFS WHERE file_md5 = '%s'", md5sum);
    
    if(mysql_query(sql_conn, query))
    {
        printf("Error making query: %s\n", mysql_error(sql_conn));
        return 1;
    }
    else
    {
        sql_res = mysql_use_result(sql_conn);
        if(sql_res)
        {        
            if((sql_row = mysql_fetch_row(sql_res)) != NULL)
            {
                mysql_free_result(sql_res);

                return 0;
            }
            else
            {
                mysql_free_result(sql_res);

                return 1;
            }
        }
    }
    return 0;
}

int sql_do_quick_puts(const client_t *client, const char *md5sum, const char *file_name)
{
    MYSQL_RES *sql_res = NULL;
    MYSQL_ROW sql_row;

    int pre_code = client->code == 0 ? -1 : client->code;

    char query[512] = {0};

    sprintf(query, \
            "SELECT file_size, file_size_1, file_size_2,"
            " cluster_ip_1, cluster_port_1,"
            " cluster_ip_2, cluster_port_2"
            " FROM VFS WHERE file_md5 = '%s'", md5sum);
    
    if(mysql_query(sql_conn, query))
    {
        printf("Error making query: %s\n", mysql_error(sql_conn));
        return 1;
    }
    else
    {
        sql_res = mysql_use_result(sql_conn);
        if(sql_res)
        {        
            if((sql_row = mysql_fetch_row(sql_res)) != NULL)
            {
                
                int file_size = atoi(sql_row[0]);
                int file_size_1 = atoi(sql_row[1]);
                int file_size_2 = atoi(sql_row[2]);
                char ip1[16] = {0};
                char ip2[16] = {0};
                char port1[5] = {0};
                char port2[5] = {0};

                strcpy(ip1, sql_row[3]);
                strcpy(port1, sql_row[4]);
                strcpy(ip2, sql_row[5]);
                strcpy(port2, sql_row[6]);
                
                mysql_free_result(sql_res);

                bzero(query, sizeof(query));
                sprintf(query,
                        "INSERT INTO VFS("
                        " pre_code, file_name, file_type,"
                        " file_size, file_md5, owner_uid,"
                        " cluster_ip_1, cluster_port_1,"
                        " cluster_ip_2, cluster_port_2,"
                        " file_size_1, file_size_2)"
                        " VALUES(%d, '%s', 'f',"
                        " %d, '%s', %d,"
                        " '%s', '%s', '%s', '%s', %d, %d)", \
                        pre_code, file_name, \
                        file_size, md5sum, client->uid, \
                        ip1, port1, \
                        ip2,port2, \
                        file_size_1, file_size_2);


                if(mysql_query(sql_conn, query))
                {
                    printf("Error making query: %s\n", mysql_error(sql_conn));
                    return 1;
                }
                else
                {
                    printf("[INFO] : quick puts %s success\n", file_name);
                    
                    return 0;
                }

                return 0;
            }
            else
            {
                mysql_free_result(sql_res);
                return 1;
            }
        }
    }
    return 0;
}

int sql_do_puts(const client_t *client, const char *file_name, const char *md5sum, int file_size,\
                const char *ip1, const char *port1, int part1_size,\
                const char *ip2, const char *port2, int part2_size)
{
    int pre_code = client->code == 0 ? -1 : client->code;
    char query[512] = {0};
    sprintf(query,
            "INSERT INTO VFS("
            " pre_code, file_name, file_type,"
            " file_size, file_md5, owner_uid,"
            " cluster_ip_1, cluster_port_1,"
            " cluster_ip_2, cluster_port_2,"
            " file_size_1, file_size_2)"
            " VALUES(%d, '%s', 'f',"
            " %d, '%s', %d,"
            " '%s', '%s', '%s', '%s', %d, %d)", \
            pre_code, file_name, \
            file_size, md5sum, client->uid, \
            ip1, port1, \
            ip2,port2, \
            part1_size, part2_size);
    
    if(mysql_query(sql_conn, query))
    {
        printf("Error making query: %s\n", mysql_error(sql_conn));
        
        return -1;
    }
    else
    {
        printf("[INFO] : normal puts %s success\n", file_name);
                
        return 0;
    }
}
