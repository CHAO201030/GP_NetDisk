#ifndef __MD5_H__
#define __MD5_H__

#define OPENSSL_API_COMPAT 0x00908000L

#include "head.h"
#include <openssl/md5.h>

/**
 * @brief 获得文件的MD5
 * 
 * @param fd 文件描述符
 * @param md5sum 存放MD5值的数组
 * @return int 返回0
 */
int get_file_md5(int fd, char *md5sum);

#endif