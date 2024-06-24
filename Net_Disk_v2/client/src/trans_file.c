/**
 * @file trans_file.c
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-06-19
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "../include/trans_file.h"
#include "../include/thread_func.h"

int sendn(int sfd, void *buf, int buf_len)
{
    char *p = buf;
    int finish_send = 0;
    int cur_send;

    while(finish_send < buf_len)
    {
        cur_send = send(sfd, p + finish_send, buf_len - finish_send, 0);
        if(cur_send <= 0)return cur_send;

        finish_send += cur_send;
    }

    return finish_send;
}

int recvn(int sfd, void *buf, int buf_len)
{
    char *p = buf;
    int finish_recv = 0;
    int cur_recv;

    while(finish_recv < buf_len)
    {
        cur_recv = recv(sfd, p + finish_recv, buf_len - finish_recv, 0);
        if(cur_recv <= 0)return cur_recv;

        finish_recv += cur_recv;
    }

    return finish_recv;
}

off_t get_file_size(int fd)
{
    struct stat file_stat;
    bzero(&file_stat, sizeof(file_stat));
    
    fstat(fd, &file_stat);

    return file_stat.st_size;
}

int recv_big_file(int sfd, int fd, int offset, int part_size)
{
    off_t recv_size = 0;
    off_t cur_size = 0;

    while(recv_size < part_size) {
        if(part_size - recv_size < MMAP_SIZE)
        {
            cur_size = part_size - recv_size;
        }
        else
        {
            cur_size = MMAP_SIZE;
        }

        // 每次映射 cur_size 个字节 从 fd 的 recv_size + offset 位置开始
        void *mm_addr = mmap(NULL, cur_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, offset + recv_size);
        if(mm_addr == MAP_FAILED)
        {
            perror("mmap");
            exit(EXIT_FAILURE);
        }

        int ret = recvn(sfd, mm_addr, cur_size);
        if(ret <= 0)return ret;

        munmap(mm_addr, cur_size);

        recv_size += cur_size;
    }

    return recv_size;
}

void multi_point_download(int cur_fd, int file_server1_fd, int offset1, int part1_size, int file_server2_fd, int offset2, int part2_size)
{
    pthread_t pth_gets1;
    pthread_t pth_gets2;

    pth_gets_args file_info_1 = {cur_fd, file_server1_fd, 0, part1_size};
    pth_gets_args file_info_2 = {cur_fd, file_server2_fd, part1_size, part2_size};

    pthread_create(&pth_gets1, NULL, pth_download, (void *)&file_info_1);
    pthread_create(&pth_gets2, NULL, pth_download, (void *)&file_info_2);

    pthread_join(pth_gets1, NULL);
    pthread_join(pth_gets2, NULL);

    return;
}
