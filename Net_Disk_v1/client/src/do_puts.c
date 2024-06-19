#include "../include/task.h"
#include "../include/md5.h"
#include "../include/thread_func.h"
#include "../include/trans_file.h"

// TODO : 丑陋的实现 没封装 下次一定封装

void do_puts(thread_args *pth_args)
{
    /**
     * 1. 先检查本地是否有这个文件
     * 2. 有文件先获取大小
     * 3. 大文件就计算分片信息 为以后的多点下载服务  小文件直接用
     * 4. 
    */
    train_t puts_train = {0};
    strncpy(puts_train.data_buf, pth_args->cur_cmd, strlen(pth_args->cur_cmd));

    char *file_name = strtok(pth_args->cur_cmd, " ");
    file_name = strtok(NULL, " ");

    int cur_fd = open(file_name, O_RDWR);
    if(cur_fd == -1)
    {
        printf("No such file in current directory");
        return ;
    }

    int route_new_fd = tcp_connect(ROUTE_IP, ROUTE_PORT);

    bool token_is_valid = verify_token(route_new_fd);

    if(token_is_valid)
    {
        // TOKEN 验证成功发送 PUTS 命令火车
        puts_train.data_len = strlen(puts_train.data_buf);
        puts_train.state = CMD_PUTS;

        send_cmd(&puts_train, CMD_PUTS, route_new_fd);
        
        // TODO : 获得该文件的MD5发送给路由服务器
        char md5sum[33] = {0};
        get_file_md5(cur_fd, md5sum);
        sendn(route_new_fd, md5sum, sizeof(md5sum));

        bool is_quick_trans = false;
        recvn(route_new_fd, &is_quick_trans, sizeof(is_quick_trans));

        if(is_quick_trans)
        {
            printf("\nQuick transfile success\n");
            close(cur_fd);
            close(route_new_fd);
            return ;
        }
        else
        {
            char ip1[16]  = {0};
            char port1[5] = {0};
            char ip2[16]  = {0};
            char port2[5] = {0};

            bool is_big_file = false;
            
            // 接收文件服务器信息
            recvn(route_new_fd, ip1, sizeof(ip1));
            recvn(route_new_fd, ip2, sizeof(ip2));
            recvn(route_new_fd, port1, sizeof(port1));
            recvn(route_new_fd, port2, sizeof(port2));  

            off_t file_size = get_file_size(cur_fd);
            off_t send_size = 0;
            off_t cur_size  = 0;

            int part1_size = 0;
            int part2_size = 0;
            
            char buf[BUFFER_SIZE] = {0};

            if(file_size < BIG_FILE_SIZE)
            {
                // 小文件
                part1_size = file_size;
                // printf("part1 = %d, part2 = %d\n", part1_size, part2_size);
                // 告诉路由服务器文件的大小 以及多点下载时的分片大小
                sendn(route_new_fd, &file_size, sizeof(file_size));
                sendn(route_new_fd, &part1_size, sizeof(part1_size));
                sendn(route_new_fd, &part2_size, sizeof(part2_size));

                // 连接文件服务器
                int file_server_fd = tcp_connect(ip1, port1);

                // 发送puts火车 触发文件服务器的线程
                train_t t = {0};
                t.data_len = strlen(md5sum);
                t.state  = CMD_PUTS;
                strncpy(t.data_buf, md5sum, t.data_len);
                send_cmd(&t, CMD_PUTS, file_server_fd);

                // 发送文件大小信息
                sendn(file_server_fd, &is_big_file, sizeof(is_big_file));
                sendn(file_server_fd, &file_size, sizeof(file_size));

                while(send_size < file_size)
                {
                    if(file_size - send_size < BUFFER_SIZE)
                    {
                        cur_size = file_size - send_size;
                    }
                    else
                    {
                        cur_size = BUFFER_SIZE;
                    }
                    
                    read(cur_fd, buf, cur_size);
                    sendn(file_server_fd, buf, cur_size);
                    send_size += cur_size;
                }
            }
            else
            {
                // 大文件
                is_big_file = true;
                part1_size = file_size >> 13 << 12;
                part2_size = file_size - part1_size;

                // 告诉路由服务器文件的大小 以及多点下载时的分片大小
                sendn(route_new_fd, &file_size, sizeof(file_size));
                sendn(route_new_fd, &part1_size, sizeof(part1_size));
                sendn(route_new_fd, &part2_size, sizeof(part2_size));

                int file_server1_fd = tcp_connect(ip1, port1);
                int file_server2_fd = tcp_connect(ip2, port2);

                // 发送puts火车 触发文件服务器1/2的线程
                train_t t = {0};
                t.data_len = strlen(md5sum);
                t.state  = CMD_PUTS;
                strncpy(t.data_buf, md5sum, t.data_len);
                send_cmd(&t, CMD_PUTS, file_server1_fd);
                send_cmd(&t, CMD_PUTS, file_server2_fd);

                sendn(file_server1_fd, &is_big_file, sizeof(is_big_file));
                sendn(file_server2_fd, &is_big_file, sizeof(is_big_file));
                
                sendn(file_server1_fd, &file_size, sizeof(file_size));
                sendn(file_server2_fd, &file_size, sizeof(file_size));


                int send_size = 0;
                int cur_size = 0;

                while(send_size < file_size)
                {
                    if(file_size - send_size < MMAP_SIZE)
                    {
                        cur_size = file_size - send_size;
                    }
                    else
                    {
                        cur_size = MMAP_SIZE;
                    }

                    void *mm_addr = mmap(NULL, cur_size, PROT_READ|PROT_WRITE, MAP_SHARED, cur_fd, send_size);

                    sendn(file_server1_fd, mm_addr, cur_size);
                    sendn(file_server2_fd, mm_addr, cur_size);

                    munmap(mm_addr, cur_size);

                    send_size += cur_size;
                }
            }
        }
    }
}
