#ifndef __TRANS_FILE_H__
#define __TRANS_FILE_H__

#include "head.h"

off_t get_file_size(int fd);

void send_file(int sfd);

void recv_file(int sfd);

void recv_cluster_info(int sfd, char *ip, char *port);

int sendn(int sfd, void *buf, int buf_len);

int recvn(int sfd, void *buf, int buf_len);

#endif