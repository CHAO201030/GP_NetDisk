#ifndef __WD_NETWORK_H
#define __WD_NETWORK_H

#include <func.h>

int tcp_listen(void);
int tcp_connect(const char* host, const char* service);
void* get_ip_addr(struct sockaddr* sa);

void epoll_add(int epfd, int fd);
void epoll_del(int epfd, int fd);

#endif
