#ifndef __THREAD_FUNC_H__
#define __THREAD_FUNC_H__

#include "head.h"
#include "net.h"

/**
 * @brief 启动线程进行长命令操作
 * 
 * @param args 命令类型CMD_TYPE, 命令内容 cur_cmd
 * @return void* NULL
 */
void *pth_func(void *args);

/**
 * @brief 线程进行多点下载任务
 * 
 * @param args 文件的 fd, 文件服务器的 sfd, 文件偏移 offset, 文件片大小 part_size
 * @return void* NULL
 */
void *pth_download(void *args);


#endif