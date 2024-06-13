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

off_t get_file_size(int fd)
{
    struct stat file_stat;
    bzero(&file_stat, sizeof(file_stat));
    
    fstat(fd, &file_stat);

    return file_stat.st_size;
}

void send_file(int sfd)
{
    int fd = open(FILE_NAME, O_RDWR);
    if(fd == -1)
    {
        error(0, errno, "open %s failed", FILE_NAME);
    }

    // 发送文件名
    train_t t = {0};
    t.data_len = strlen(FILE_NAME);
    t.state = CMD_PUTS; // 客户端上传文件PUTS
    strncpy(t.data_buf, FILE_NAME, t.data_len);

    sendn(sfd, &t.data_len, sizeof(t.data_len));
    sendn(sfd, &t.state, sizeof(t.state));
    sendn(sfd, t.data_buf, t.data_len);
    
    off_t file_size = get_file_size(fd);

    // 发送文件大小
    bzero(&t, sizeof(t));
    
    t.data_len = file_size;
    t.state = CMD_PUTS;
    
    sendn(sfd, &t.data_len, sizeof(t.data_len));
    sendn(sfd, &t.state, sizeof(t.state));
    
    // 发送文件内容
    off_t send_size = 0;
    off_t cur_size = 0;
        
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

        void *mm_addr = mmap(NULL, cur_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, send_size);

        sendn(sfd, mm_addr, cur_size);

        munmap(mm_addr, cur_size);

        send_size += cur_size;
    }

    close(fd);
}

void recv_file(int sfd)
{
    // 接收文件名
    train_t t = {0};
    recvn(sfd, &t.data_len, sizeof(t.data_len));
    recvn(sfd, &t.state, sizeof(t.state));
    recvn(sfd, t.data_buf, t.data_len);

    // 创建文件
    int fd = open(t.data_buf, O_RDWR|O_CREAT, 0666);
    if(fd == -1)
    {
        error(-1, errno, "create %s failed", t.data_buf);
    }

    // 接收文件大小
    bzero(&t, sizeof(t));
    recvn(sfd, &t.data_len, sizeof(t.data_len));
    recvn(sfd, &t.state, sizeof(t.state));

    off_t file_size = t.data_len;

    // 改变文件大小
    ftruncate(fd, file_size);

    // 接收文件内容
    off_t recv_size = 0;
    off_t cur_size = 0;
        
    while(recv_size < file_size)
    {
        if(file_size - recv_size < MMAP_SIZE)
        {
            cur_size = file_size - recv_size;
        }
        else
        {
            cur_size = MMAP_SIZE;
        }

        void *mm_addr = mmap(NULL, cur_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, recv_size);

        recvn(sfd, mm_addr, cur_size);

        munmap(mm_addr, cur_size);

        recv_size += cur_size;
    }

    close(fd);
}