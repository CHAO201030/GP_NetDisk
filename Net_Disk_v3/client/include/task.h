#ifndef __TASK_H__
#define __TASK_H__

#include "head.h"
#include "trans_file.h"

/**
 * @brief 打印起始界面
 * 
 */
void start_menu(void);

/**
 * @brief 发送命令火车
 * 
 * @param cmd_train 命令火车
 * @param type 命令类型
 * @param route_sfd 路由服务器sfd
 */
void send_cmd(train_t* cmd_train, CMD_TYPE type, int route_sfd);

/**
 * @brief 命令分析函数 分析命令后向服务器发送命令火车
 * 
 * @param cmd_train 命令火车
 * @param route_sfd 路由服务器套接字
 */
void command_analyse(train_t* cmd_train, int route_sfd);

/**
 * @brief 接收服务器的消息火车并进入对应的命令处理
 * 
 * @param route_sfd 服务器套接字
 * @return int 成功返回1 对端断开返回0
 */
int recv_server_msg(int route_sfd);

/**
 * @brief 进行 LS 操作
 * 
 * @param server_msg 服务器传来的消息
 */
void do_ls(train_t server_msg);

/**
 * @brief 进行 CD 操作
 * 
 * @param server_msg 服务器传来的消息
 */
void do_cd(train_t server_msg);

/**
 * @brief 进行 RM 操作
 * 
 * @param server_msg 服务器传来的消息
 */
void do_rm(train_t server_msg);

/**
 * @brief 进行 PWD 操作
 * 
 * @param server_msg 服务器传来的消息
 */
void do_pwd(train_t server_msg);

/**
 * @brief 子线程进行 GETS 操作
 * 
 * @param pth_args 线程所需参数 命令类型以及命令
 */
void do_gets(thread_args *pth_args);

/**
 * @brief 子线程进行 GETS 操作
 * 
 * @param pth_args 线程所需参数 命令类型以及命令
 */
void do_puts(thread_args *pth_args);

/**
 * @brief 进行 MKDIR 操作
 * 
 * @param server_msg 服务器传来的消息
 */
void do_mkdir(train_t server_msg);

/**
 * @brief 进行 RMDIR 操作
 * 
 * @param server_msg 服务器传来的消息
 */
void do_rmdir(train_t server_msg);

/**
 * @brief 发送 TOKEN 验证消息
 * 
 * @param server_msg 服务器传来的消息
 */
void do_token(train_t server_msg);

/**
 * @brief 进行登录操作
 * 
 * @param route_sfd 服务器套接字
 */
void do_login(int route_sfd);

/**
 * @brief 进行注册操作
 * 
 * @param route_sfd 服务器套接字
 */
void do_register(int route_sfd);

/**
 * @brief 验证 TOKEN 是否有效
 * 
 * @param route_new_fd 路由服务器sfd
 * @return true 
 * @return false 
 */
bool verify_token(int route_new_fd);


#endif