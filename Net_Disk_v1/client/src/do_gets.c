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
            
            // 留到合并文件时候用
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


                // 创建两个临时文件
                char part1_name[512] = {0};
                sprintf(part1_name, "%s.temp1", new_file_path);

                int part1_fd = open(part1_name, O_RDWR|O_CREAT, 0666);
                if(part1_fd == -1)
                {
                    printf("[INFO] : create file failed\n");
                    return;                    
                }

                char part2_name[512] = {0};
                sprintf(part2_name, "%s.temp2", new_file_path);

                int part2_fd = open(part2_name, O_RDWR|O_CREAT, 0666);
                if(part2_fd == -1)
                {
                    printf("[INFO] : create file failed\n");
                    return;                    
                }

                // 获取已下载部分1的磁盘占用页数 并发送偏移信息
                struct stat part_stat = {0};

                fstat(part1_fd, &part_stat);
                
                long part1_pages = part_stat.st_blocks / 8;
            
                // 获取已下载部分2的磁盘占用页数 并发送偏移信息
                fstat(part2_fd, &part_stat);
                
                long part2_pages = part_stat.st_blocks / 8;

                int offset1 = 0;
                int offset2 = 0;
                
                if(part1_pages == 0)
                {
                    offset1 = part1_pages;
                    ftruncate(part1_fd, part1_size);
                }
                else
                {
                    offset1 = (part1_pages - 1) << 12;
                }

                if(part2_pages == 0)
                {
                    offset2 = part2_pages;
                    ftruncate(part2_fd, part2_size);
                }
                else
                {
                    offset2 = (part2_pages - 1) << 12;
                }

                sendn(file_server1_fd, &offset1, sizeof(offset1));
                sendn(file_server1_fd, &part1_size, sizeof(part1_size));

                offset2 += part1_size;
                sendn(file_server2_fd, &offset2, sizeof(offset2));
                sendn(file_server2_fd, &part2_size, sizeof(part2_size));


                multi_point_download(part1_fd, part2_fd,
                                     file_server1_fd, offset1, part1_size, 
                                     file_server2_fd, offset2, part2_size);
                
                // merge_file(fd1, fd2, size1, size2);
                ftruncate(part1_fd, part1_size + part2_size);

                int cur_size   = 0;
                int merge_size = 0;

                while(merge_size < part2_size)
                {
                    cur_size = (part2_size - merge_size) < MMAP_SIZE ? part2_size - merge_size : MMAP_SIZE;

                    void *mm_addr1 = mmap(NULL, cur_size, PROT_READ|PROT_WRITE, MAP_SHARED, part1_fd, part1_size + merge_size);
                    void *mm_addr2 = mmap(NULL, cur_size, PROT_READ|PROT_WRITE, MAP_SHARED, part2_fd, merge_size);
                    
                    memcpy(mm_addr1, mm_addr2, cur_size);

                    munmap(mm_addr1, cur_size);
                    munmap(mm_addr2, cur_size);

                    merge_size += cur_size;
                }

                close(part1_fd);
                close(part2_fd);
                close(route_new_fd); 
                close(file_server1_fd);
                close(file_server2_fd);

                rename(part1_name, new_file_path);
                unlink(part2_name);
            }
            else
            {
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
                
                close(route_new_fd); 
                close(file_server1_fd);
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
        
        close(route_new_fd);
    }
    
    return ;
}
