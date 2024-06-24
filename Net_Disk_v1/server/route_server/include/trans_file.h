#ifndef __TRANS_FILE_H__
#define __TRANS_FILE_H__

#include "head.h"

off_t get_file_size(int fd);

/**
 * @brief 从 buf 向 sfd 发送 buf_len 个字节数据
 * 
 * @param sfd 网络套接字
 * @param buf 数据缓冲区
 * @param buf_len 要发送的大小
 * @return int 成功返回本次发送的大小 对端断开返回0 失败返回-1
 */
int sendn(int sfd, void *buf, int buf_len);

/**
 * @brief 从 sfd 接收 buf_len 个字节数据到 buf 中
 * 
 * @param sfd 网络套接字
 * @param buf 数据缓冲区
 * @param buf_len 要接收的大小
 * @return int 成功返回本次接收的大小 对端断开返回0 失败返回-1
 */
int recvn(int sfd, void *buf, int buf_len);

/**
 * @brief 发送文件服务器集群的信息
 * 
 * @param sfd 对应客户端的fd
 * @param ip 文件服务器IP
 * @param ip_len IP的长度
 * @param port 文件服务器PORT
 * @param port_len PORT的长度
 */
void send_file_server_info(int sfd, const char *ip, int ip_len, const char *port, int port_len);

#endif