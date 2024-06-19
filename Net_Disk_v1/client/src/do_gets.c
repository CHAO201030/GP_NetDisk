#include "../include/task.h"
#include "../include/thread_func.h"
#include "../include/trans_file.h"

void do_gets(thread_args *pth_args)
{
    int route_new_fd = tcp_connect(ROUTE_IP, ROUTE_PORT);

    bool token_is_valid = verify_token(route_new_fd);

    train_t gets_train = {0};

    if(token_is_valid)
    {
        // TOKEN 验证成功 发送 GETS 命令火车
        gets_train.data_len = strlen(pth_args->cur_cmd);
        gets_train.state = CMD_GETS;
        strncpy(gets_train.data_buf, pth_args->cur_cmd, gets_train.data_len);
        
        send_cmd(&gets_train, CMD_GETS, route_new_fd);

        // 验证文件命令的有效性
        bool file_valid = false;
        recvn(route_new_fd, &file_valid, sizeof(file_valid));

        if(file_valid)
        {
            bool is_big_file = false;

            char *new_file_name = strtok(pth_args->cur_cmd, " ");
            new_file_name = strtok(NULL, " ");
            
            char new_file_path[300]  = {0};
            sprintf(new_file_path, "./download/%s", new_file_name);

            unsigned char md5sum[33] = {0};
            
            char ip1[16] = {0};
            char port1[5] = {0};
            int part1_size = 0;

            char ip2[16] = {0};
            char port2[5] = {0};
            int part2_size = 0;

            // 接收文件的md5
            recvn(route_new_fd, md5sum, sizeof(md5sum));
            // 接收文件服务器1的 {IP, PORT, Part1_size}
            recvn(route_new_fd, ip1, sizeof(ip1));
            recvn(route_new_fd, port1, sizeof(port1));
            recvn(route_new_fd, &part1_size, sizeof(part1_size));
            // 接收文件服务器2的 {IP, PORT, Part2_size}
            recvn(route_new_fd, ip2, sizeof(ip2));
            recvn(route_new_fd, port2, sizeof(port2));
            recvn(route_new_fd, &part2_size, sizeof(part2_size));

            is_big_file = part2_size == 0 ? false : true;

            if(is_big_file)
            {
                int file_server1_fd = tcp_connect(ip1, port1);
                int file_server2_fd = tcp_connect(ip2, port2);

                // 同一个文件 可以用同一个火车
                bzero(&gets_train, sizeof(gets_train));
                gets_train.data_len = strlen((const char *)md5sum);
                gets_train.state = CMD_GETS;
                strncpy(gets_train.data_buf, (const char *)md5sum, gets_train.data_len);
                // 给文件服务器发送命令火车
                send_cmd(&gets_train, CMD_GETS, file_server1_fd);
                send_cmd(&gets_train, CMD_GETS, file_server2_fd);
                int cur_fd = open(new_file_path, O_RDWR|O_CREAT, 0666);
                if(cur_fd == -1)
                {
                    printf("[INFO] : create file failed\n");
                    return;
                }
                ftruncate(cur_fd, part1_size + part2_size);
                // 给文件服务器1发送文件偏移信息
                int offset1 = 0;
                sendn(file_server1_fd, &offset1, sizeof(offset1));
                sendn(file_server1_fd, &part1_size, sizeof(part1_size));
                // 给文件服务器2发送文件偏移信息
                int offset2 = part1_size;
                sendn(file_server2_fd, &offset2, sizeof(offset2));
                sendn(file_server2_fd, &part2_size, sizeof(part2_size));

                pthread_t pth_gets1;
                pthread_t pth_gets2;

                pth_gets_args file_info_1 = {0};
                pth_gets_args file_info_2 = {0};

                file_info_1.fd = cur_fd;
                file_info_1.sfd = file_server1_fd;
                file_info_1.offset = 0;
                file_info_1.part_size = part1_size;
                
                file_info_2.fd = cur_fd;
                file_info_2.sfd = file_server2_fd;
                file_info_2.offset = part1_size;
                file_info_2.part_size = part2_size;

                pthread_create(&pth_gets1, NULL, pth_download, (void *)&file_info_1);
                pthread_create(&pth_gets2, NULL, pth_download, (void *)&file_info_2);

                pthread_join(pth_gets1, NULL);
                pthread_join(pth_gets2, NULL);
            
                close(cur_fd);
                close(file_server1_fd);
                close(file_server2_fd);
            }
            else
            {
                // printf("send small file\n");
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


                int file_size = part1_size, recv_size = 0, cur_size = 0;

                char file_buf[BUFFER_SIZE] = {0};

                int cur_fd = open(new_file_path, O_RDWR|O_CREAT, 0666);
                if(cur_fd == -1)
                {
                    printf("[INFO] : create file failed\n");
                    return;
                }

                while(recv_size < file_size)
                {
                    if(file_size - recv_size < BUFFER_SIZE)
                    {
                        cur_size = file_size - recv_size;
                    }
                    else
                    {
                        cur_size = BUFFER_SIZE;
                    }

                    bzero(file_buf, BUFFER_SIZE);
                    recvn(file_server1_fd, file_buf, cur_size);
                    write(cur_fd, file_buf, cur_size);

                    recv_size += cur_size;
                }
            }
        }
        else
        {
            printf("[INFO] : Gets failed no such file...\n");
        }
    }
    else
    {
        printf("[INFO] : Token verify failed\n");
    }
    
    close(route_new_fd);

    return ;
}
