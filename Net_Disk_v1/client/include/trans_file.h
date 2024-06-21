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
 * @brief 接收文件服务器的信息
 * 
 * @param sfd 路由服务器的sfd
 * @param ip 存储文件服务器IP
 * @param ip_len IP的长度
 * @param port 存储文件服务器PORT
 * @param port_len PORT的长度
 */
void recv_file_server_info(int sfd, const char *ip, int ip_len, const char *port, int port_len);

/**
 * @brief 获取文件大小
 * 
 * @param fd 文件描述符
 * @return off_t 返回文件的大小
 */
off_t get_file_size(int fd);

/**
 * @brief 发送大文件
 * 
 * @param file_server_fd 文件服务器fd
 * @param cur_fd 文件fd
 * @param file_size 文件大小
 * @return int 成功返回已经发送的大小
 */
int send_big_file(int file_server_fd, int cur_fd, off_t file_size);

/**
 * @brief 发送小文件
 * 
 * @param file_server_fd 文件服务器fd
 * @param cur_fd 文件fd
 * @param file_size 文件大小
 * @return int 成功返回已经发送的大小
 */
int send_small_file(int file_server_fd, int cur_fd, off_t file_size);

/**
 * @brief 多点下载
 * 
 * @param cur_fd 当前要下载的文件的fd
 * @param file_server1_fd 文件服务器1的fd 
 * @param offset1 文件服务器1发送的分片起始位置
 * @param part1_size 文件服务器1发送的分片结束位置
 * @param file_server2_fd 文件服务器2的fd
 * @param offset2 文件服务器2发送的分片起始位置
 * @param part2_size 文件服务器2发送的分片结束位置
 */
void multi_point_download(int cur_fd, int file_server1_fd, int offset1, int part1_size, int file_server2_fd, int offset2, int part2_size);

/**
 * @brief 接收大文件
 * 
 * @param sfd 文件服务器套接字
 * @param fd 该文件的fd
 * @param offset 相对文件首部的偏移
 * @param part_size 分片大小
 * @return int 成功返回已经接收的大小
 */
int recv_big_file(int sfd, int fd, int offset, int part_size);

/**
 * @brief 接收小文件
 * 
 * @param sfd 文件服务器套接字
 * @param fd 该文件的fd
 * @param offset 相对文件首部的偏移
 * @param part_size 分片大小
 * @return int 成功返回已经接收的大小
 */
int recv_small_file(int sfd, int fd, off_t file_size);

#endif