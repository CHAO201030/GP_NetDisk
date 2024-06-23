/**
 * @file do_gets.c
 * @author your name (you@domain.com)
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

/*route中的do_gets只负责发送给客户端主线程要下载的文件的大小,md5,和该到哪个服务器去下载，不负责具体发送文件。
具体业务逻辑为：先根据传入的client_t获得到用户uid，当前目录层级，上一级目录；根据传入的cmd获得到要下载的文件名字。
通过uid, code, pre_code和filename去vsf表中查询是否有文件符合，并且要注意is_valid是否为1（为0则用户删除了这个文件，不能下载）
得到文件的md5以及file_size后再去md5_vfs表中查到文件可以通过哪文件两个服务器下载，并将文件服务器的端口和地址都发送给客户端主线程。

*/
void do_gets(client_t *client, char *cmd)
{  
    printf("[INFO] : do_gets begin!\n");
    char *target_file = strtok(cmd, " ");
    target_file = strtok(NULL, " ");        //这一次获取到的才是真正的文件名

    bool is_valid = false;  //服务器对文件命令是否有效进行检验，若文件命令无效则不会传输数据。默认文件命令无效

    unsigned char md5sum[33] = {0};

    char ip1[16] = {0};
    char port1[5] = {0};
    
    char ip2[16] = {0};
    char port2[5] = {0};

    int file_size=0;

    if(sql_do_gets(client, target_file, md5sum, ip1, port1, ip2, port2, &file_size) == 0)   //获取到了md5
    {
        is_valid = true;

        int part1_size = file_size >> 13 << 12;         //将part1_size 取得以4K为单位的总文件的一半大小
        int part2_size = file_size - part1_size;

        //发送多点下载的文件信息,客户端接收时也应当按序接收
        sendn(client->fd, &is_valid, sizeof(is_valid)); //下载是否有效

        sendn(client->fd, md5sum, sizeof(md5sum));      //md5
        sendn(client->fd, ip1, sizeof(ip1));            //第一个服务器的地址
        sendn(client->fd, port1, sizeof(port1));        //第一个服务器的端口号
        sendn(client->fd, &part1_size, sizeof(part1_size)); //发送第一个分片大小   
    
        sendn(client->fd, ip2, sizeof(ip2));            //第二个服务器的地址
        sendn(client->fd, port2, sizeof(port2));        //第二个服务器的端口
        sendn(client->fd, &part2_size, sizeof(part2_size));     //发送第二个分片大小
    }
    else {
        sendn(client->fd, &is_valid, sizeof(is_valid)); //下载是否有效
    }
}

/*
该函数应该先在VFS根据传入的client_t和target_file在VFS表中查到文件是否存在和有效以及大小，若有效则根据查到的md5再到
md5_ip表中查到文件可以通过哪两个服务器中下载,错误返回-1，成功返回0
*/
int sql_do_gets(const client_t *client, const char *target_file, unsigned char *md5sum, char *ip1, char *port1, char *ip2, char *port2, int *file_size)
{
    MYSQL_RES *sql_res = NULL;
    MYSQL_ROW sql_row;

    int pre_code = client->code == 0 ? -1 : client->code;    //客户端当前是否为在根目录

    char query[512] = {0};
    
    //检查当前用户在当前目录能否查到该文件,并且用户还没有删掉该文件
    //用户删除了也算没找到则找到根据md5去md5_ip表找到真正的文件
    sprintf(query, "SELECT file_md5, file_size"
                   " FROM VFS WHERE owner_uid = %d AND file_type = 'f' AND file_name = '%s' AND pre_code = %d AND is_valid = 1", 
                   client->uid, target_file, pre_code); 
    
    if(mysql_query(sql_conn, query)) {      //有错
        printf("ERROR! VFS Making query: %s\n", mysql_errno(sql_conn));
    }
    else { 
        sql_res = mysql_use_result(sql_conn);   //有结果,但也有可能为NULL
        if(sql_res) {   //找到,sql_res不为空
            sql_row = mysql_fetch_row(sql_res);
            strncpy((char*)md5sum, sql_row[0], 32);     //获取第一行的md5sum
            *file_size = atoi(sql_row[2]);              //获取文件大小
        }
        else {      //没找到
            mysql_free_result(sql_res);

            return -1;
        }
    }

    //此时正常流程md5sum和file_size已经有值了,去md5_ip表查,肯定能查到
    sprintf(query,  "SELECT file_server1_ip, file_server1_port,"
                    " file_server2_ip, file_server2_port"
                    " FROM MD5_IP WHERE file_md5 = '%s'", md5sum);

    if(mysql_query(sql_conn, query)) {      //查询有错
        printf("ERROR! MD5_IP Making query: %s\n", mysql_errno(sql_conn));
    }
    else {  //在md5_ip表中查必定能获得唯一的结果
        sql_res = mysql_use_result(sql_conn);
        strncpy(ip1, sql_row[0], 16);
        strncpy(port1, sql_row[1], 5);

        strncpy(ip2, sql_row[2], 16);
        strncpy(port2, sql_row[3], 5);
    }

    return 0;

}
