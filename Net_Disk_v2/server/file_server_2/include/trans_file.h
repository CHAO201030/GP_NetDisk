#ifndef __TRANS_FILE_H__
#define __TRANS_FILE_H__

#include "head.h"

int send_big_file(int sfd, int fd, int offset, int part_size);

off_t recv_file(int sfd);

int sendn(int sfd, void *buf, int buf_len);

int recvn(int sfd, void *buf, int buf_len);

#endif