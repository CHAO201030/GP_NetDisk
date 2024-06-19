#ifndef __TRANS_FILE_H__
#define __TRANS_FILE_H__

#include "head.h"

/**
 * @brief 获取文件大小
 * 
 * @param fd 文件描述符
 * @return off_t 返回文件的大小
 */
off_t get_file_size(int fd);

void send_file(int sfd);

/**
 * @brief 接收大文件
 * 
 * @param sfd 文件服务器套接字
 * @param fd 该文件的fd
 * @param offset 相对文件首部的偏移
 * @param part_size 分片大小
 * @return int 
 */
int recv_big_file(int sfd, int fd, int offset, int part_size);

void recv_cluster_info(int sfd, char *ip, char *port);

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

#endif