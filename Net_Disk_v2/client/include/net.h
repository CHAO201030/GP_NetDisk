#ifndef __NET_H__
#define __NET_H__

#include "head.h"

/**
 * @brief 对 IP PORT 发起TCP连接
 * 
 * @param ip 服务器IP
 * @param port 服务器PORT
 * @return int 成功返回 TCP连接套接字描述符
 */
int tcp_connect(const char *ip, const char *port);

/**
 * @brief epoll 监听 fd
 * 
 * @param epfd epoll 实例
 * @param fd 要监听的 fd
 */
void epoll_add(int epfd, int fd);

/**
 * @brief epoll 取消监听 fd
 * 
 * @param epfd epoll 实例
 * @param fd 要取消监听的 fd
 */
void epoll_del(int epfd, int fd);

#endif