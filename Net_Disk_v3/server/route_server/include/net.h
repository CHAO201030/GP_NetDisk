#ifndef __NET_H__
#define __NET_H__

#include "head.h"

/**
 * @brief 初始化TCP被动连接套接字
 * 
 * @param ip 服务器的 IP
 * @param port 服务器的 PORT
 * @return int 返回该 TCP连接 的文件描述符
 */
int tcp_init(const char* ip, const char* port);

/**
 * @brief 接收客户端的连接
 * 
 * @param sfd tcp_init获得的被动连接套接字
 * @return int 返回新客户端 TCP连接 的文件描述符 
 */
int tcp_accept(int sfd);

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
 * @param fd 要监听的 fd
 */
void epoll_del(int epfd, int fd);

#endif