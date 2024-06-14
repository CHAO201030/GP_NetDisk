#ifndef __WD_BUSSINESS_H
#define __WD_BUSSINESS_H

#include <func.h>

int recvn(int sockfd, void* buf, int length);
int sendn(int sockfd, void* buf, int length);

void send_string(int fd, char* str);
void recv_string(int fd, char* str);

void send_file(int sockfd, char* file_name);
void recv_file(int sockfd, char* file_name);

#endif
