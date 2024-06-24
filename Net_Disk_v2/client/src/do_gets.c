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

#include "../include/task.h"
#include "../include/thread_func.h"
#include "../include/trans_file.h"
#define BIG_FILE 100*1024*1024

/*客户端do_gets逻辑
先连接到路由服务器
验证token
验证成功后第一次发送gets命令,包含要下载的文件名字和当前用户的client_t，
客户端会期待接收到路由服务器发来的验证消息，若客户端想要下载一个已经删除的文件或者不存在的文件，会收到一个无效消息，收不到md5
若有效，则会收到路由服务器发来的文件的md5，和两个文件服务器的地址以及端口，文件的大小。
第二次发送gets命令，会发送client_t和md5
*/

void do_gets(thread_args *pth_args)
{
    int route_fd = tcp_connect(ROUTE_IP, ROUTE_PORT);

    train_t gets_train = {0};

    if(verify_token(route_fd)) {    //Token验证成功时
        gets_train.data_len = strlen(pth_args->cur_cmd);
        gets_train.state = CMD_GETS;
        strncpy(gets_train.data_buf, pth_args->cur_cmd, gets_train.data_len);

        send_cmd(&gets_train, CMD_GETS, route_fd);

        //验证文件命令（客户端文件传输命令有误时将不会进行数据的接收
        bool file_valid = false;    //默认文件传输命令无效
        recvn(route_fd, &file_valid, sizeof(file_valid));      //服务器在进行文件命令验证后只会发送True或者False
        
        if(file_valid) {    //文件命令有效
            char *file_name = strtok(pth_args->cur_cmd, " ");       //此时第一个被空格分割的字符串为gets或puts
            file_name = strtok(NULL, " ");          //获得真正的文件名

            char new_file_path[300]  = {0};
            sprintf(new_file_path, "./download/%s", file_name);

            unsigned char md5sum[33] = {0};
            char ip1[16] = {0};
            char port1[5] = {0};
            int part1_size = 0;

            char ip2[16] = {0};
            char port2[5] = {0};
            int part2_size = 0;

            //服务器的发送顺序为：md5,ip1,port1,par1_size, ip2, port2, part2_size
            recvn(route_fd, md5sum, sizeof(md5sum));        //接收文件的md5
            recvn(route_fd, ip1, sizeof(ip1));  
            recvn(route_fd, port1, sizeof(port1));
            recvn(route_fd, &part1_size, sizeof(part1_size));

            recvn(route_fd, ip2, sizeof(ip2));  
            recvn(route_fd, port2, sizeof(port2));
            recvn(route_fd, &part2_size, sizeof(part2_size));


            //验证是否为大文件
            if(part1_size + part2_size > BIG_FILE) {  //大文件
                int file_server1_fd = tcp_connect(ip1, port1);
                int file_server2_fd = tcp_connect(ip2, port2);

                bzero(&gets_train, sizeof(gets_train));
                gets_train.data_len = strlen((const char*)md5sum);
                gets_train.state = CMD_GETS;
                strncpy(gets_train.data_buf, (const char*)md5sum, gets_train.data_len);

                send_cmd(&gets_train, CMD_GETS, file_server1_fd);
                send_cmd(&gets_train, CMD_GETS, file_server2_fd);

                int cur_fd = open(new_file_path, O_RDWR|O_CREAT, 0666);
                if(cur_fd == -1) {
                    printf("[INFO]: File create failed!\n");
                    return;
                }
                //设置文件的大小为file_size
                ftruncate(cur_fd, part1_size + part2_size);

                //给服务器1发送偏移信息
                int offset1 = 0;
                sendn(file_server1_fd, &offset1, sizeof(offset1));
                sendn(file_server1_fd, &part1_size, sizeof(part1_size));

                //给服务器2发送偏移信息
                int offset2 = part1_size;
                sendn(file_server2_fd, &offset2, sizeof(offset2));
                sendn(file_server2_fd, &part2_size, sizeof(part2_size));
                
                close(cur_fd);
                close(route_fd);
                close(file_server1_fd);
                close(file_server2_fd);

            }
            else {    //传递小文件
                // 传送小文件
                int file_server1_fd = tcp_connect(ip1, port1);

                bzero(&gets_train, sizeof(gets_train));
                gets_train.data_len = strlen((const char *)md5sum);
                gets_train.state = CMD_GETS;
                strncpy(gets_train.data_buf, (const char *)md5sum, gets_train.data_len);

                // 给文件服务器发送命令火车
                send_cmd(&gets_train, CMD_GETS, file_server1_fd);

                // 给文件服务器1发送文件偏移信息
                int offset1 = 0;
                sendn(file_server1_fd, &offset1, sizeof(offset1));
                sendn(file_server1_fd, &part1_size, sizeof(part1_size));


                off_t file_size = part1_size;

                int cur_fd = open(new_file_path, O_RDWR|O_CREAT, 0666);
                if(cur_fd == -1)
                {
                    printf("[INFO] : create file failed\n");
                    return;
                }

                recv_small_file(file_server1_fd, cur_fd, file_size);
                
                close(route_fd); 
                close(file_server1_fd);
            }
        }
        else {
            printf("[INFO]: Get file failed, no such file, please retry!\n");
        }
    }
    else {      //Token验证失败
        printf("[INFO]: Token verify failed, Please retry!\n");
    }

    close(route_fd);
    return;
}
