#ifndef __TASK_H__
#define __TASK_H__

#include "file_server.h"

/**
 * @brief 任务分析
 * 
 * @param new_client_fd 客户端fd
 */
void task_analyse(int new_client_fd);

/**
 * @brief 进行 GETS 操作
 * 
 * @param client_fd 客户端fd
 * @param file_name 文件在文件服务器仓库中的名字(MD5值)
 */
void do_gets_task(int client_fd, char *file_name);

/**
 * @brief 进行 PUTS 操作
 * 
 * @param client_fd 客户端fd
 * @param file_name 文件在文件服务器仓库中的名字(MD5值)
 */
void do_puts_task(int client_fd, char *file_name);

#endif