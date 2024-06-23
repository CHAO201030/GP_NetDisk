#ifndef __ROUTE_TASK_H__
#define __ROUTE_TASK_H__

#include "head.h"
#include "trans_file.h"
#include "sql.h"

/**
 * @brief 命令分析函数
 * @param cur_client 指向当前触发 epoll 的客户端
 * @param cmd_train  当前客户端发送的命令消息
 * @return NORMAL_RETURN 正常返回
 *         CLIENT_EXIT 客户端发送 exit 命令
 *         TOKEN_FAILED 客户端 TOKEN 验证失败
*/
int cmd_analyse(client_t *cur_client, train_t cmd_train);

/**
 * @brief 接收命令火车
 * @param client_fd 当前发送消息的客户端的fd
 * @param cmd_train 命令火车
 * @return 成功返回1 对端断开连接返回0
*/
int recv_cmd(int client_fd, train_t *cmd_train);

/**
 * @brief 进行 LS 操作
 * @param client 指向当前发送命令的客户端
 * @param cmd 客户端发来的命令
*/
void do_ls(client_t *client, char *cmd);

/**
 * @brief 进行 CD 操作
 * @param client 指向当前发送命令的客户端
 * @param cmd 客户端发来的命令
*/
void do_cd(client_t *client, char *cmd);

/**
 * @brief 进行 RM 操作
 * @param client 指向当前发送命令的客户端
 * @param cmd 客户端发来的命令
*/
void do_rm(client_t *client, char *cmd);

/**
 * @brief 进行 PWD 操作
 * @param client 指向当前发送命令的客户端
 * @param cmd 客户端发来的命令
*/
void do_pwd(client_t *client, char *cmd);

/**
 * @brief 进行 GETS 操作
 * @param client 指向当前发送命令的客户端
 * @param cmd 客户端发来的命令
*/
void do_gets(client_t *client, char *cmd);

/**
 * @brief 进行 PUTS 操作
 * @param client 指向当前发送命令的客户端
 * @param cmd 客户端发来的命令
*/
void do_puts(client_t *client, char *cmd);

/**
 * @brief 进行 MKDIR 操作
 * @param client 指向当前发送命令的客户端
 * @param cmd 客户端发来的命令
*/
void do_mkdir(client_t *client, char *cmd);

/**
 * @brief 进行 RMDIR 操作
 * @param client 指向当前发送命令的客户端
 * @param cmd 客户端发来的命令
*/
void do_rmdir(client_t *client, char *cmd);

/**
 * @brief 进行 TOKEN 验证
 * @param client 指向当前发送命令的客户端
 * @param token 客户端发送的 TOKEN
 * @return 成功返回0 失败返回-1
*/
int do_token(client_t *client, char *token);

/**
 * @brief 进行 LOGIN 操作
 * @param client 指向当前发送命令的客户端
 * @param cmd 客户端发来的命令
*/
void do_login(client_t *client, char *cmd);

/**
 * @brief 进行 REGISTER 操作
 * @param client 指向当前发送命令的客户端
 * @param cmd 客户端发来的命令
*/
void do_register(client_t *client, char *cmd);

#endif