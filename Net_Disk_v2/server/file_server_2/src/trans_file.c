#include "../include/trans_file.h"

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

int send_big_file(int client_fd, int fd, int offset, int part_size)
{

    off_t send_size = 0;
    off_t cur_size = 0;

    while(send_size < part_size)
    {
        if(part_size - send_size < MMAP_SIZE)
        {
            cur_size = part_size - send_size;
        }
        else
        {
            cur_size = MMAP_SIZE;
        }
        // 每次映射 cur_size 个字节 从 fd 的 send_size + start 位置开始
        void *mm_addr = mmap(NULL, cur_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, offset + send_size);

        sendn(client_fd, mm_addr, cur_size);

        munmap(mm_addr, cur_size);

        send_size += cur_size;
    }

    return part_size;
}
