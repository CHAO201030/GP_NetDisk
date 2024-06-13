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

void send_cluster_info(int sfd, const char *ip, const char *port)
{
    train_t t = {0};

    // 发送集群服务器的IP
    t.data_len = strlen(ip);
    t.state = CMD_MSG;
    strncpy(t.data_buf, ip, t.data_len);
    
    sendn(sfd, &t.data_len, sizeof(t.data_len));
    sendn(sfd, &t.state, sizeof(t.state));
    sendn(sfd, t.data_buf, t.data_len);

    // 发送集群服务器的PORT
    bzero(&t, sizeof(t));

    t.data_len = strlen(port);
    t.state = CMD_MSG;
    strncpy(t.data_buf, port, t.data_len);

    sendn(sfd, &t.data_len, sizeof(t.data_len));
    sendn(sfd, &t.state, sizeof(t.state));
    sendn(sfd, t.data_buf, t.data_len);
}