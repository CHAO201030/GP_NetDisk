#ifndef __NET_H__
#define __NET_H__

#include "head.h"

int tcp_init(const char* ip, const char* port);

int tcp_accept(int sfd);

void epoll_add(int epfd, int fd);

void epoll_del(int epfd, int fd);

#endif