#ifndef __TRANS_FILE_H__
#define __TRANS_FILE_H__

#include "head.h"

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
 * @brief 发送小文件
 * 
 * @param client_fd 客户端fd
 * @param fd 文件fd
 * @param offset 文件起始位置
 * @param part_size 文件大小
 * @return int 成功返回本次发送大小 对端断开返回0 失败返回-1
 */
int send_small_file(int client_fd, int fd, int offset, int part_size);

/**
 * @brief 发送大文件
 * 
 * @param client_fd 客户端fd
 * @param fd 文件fd
 * @param offset 文件起始偏移
 * @param part_size 文件分片大小
 * @return int 成功返回本次发送大小 对端断开返回0 失败返回-1
 */
int send_big_file(int client_fd, int fd, int offset, int part_size);

/**
 * @brief 接收小文件
 * 
 * @param client_fd 客户端fd
 * @param fd 文件fd
 * @param offset 文件起始位置
 * @param part_size 文件大小
 * @return int 成功返回本次接收大小 对端断开返回0 失败返回-1
 */
int recv_small_file(int client_fd, int fd, off_t file_size);

/**
 * @brief 接收大文件
 * 
 * @param client_fd 客户端fd
 * @param fd 文件fd
 * @param offset 文件起始偏移
 * @param part_size 文件分片大小
 * @return int 成功返回本次接收大小 对端断开返回0 失败返回-1
 */
int recv_big_file(int client_fd, int fd, off_t file_size);

#endif