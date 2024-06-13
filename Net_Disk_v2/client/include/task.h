#ifndef __TASK_H__
#define __TASK_H__

#include "head.h"
#include "trans_file.h"

void start_menu(void);

void command_analyse(train_t* cmd_train, int route_sfd);

int recv_server_msg(int route_sfd);

void do_ls(train_t server_msg);

void do_cd(train_t server_msg);

void do_rm(train_t server_msg);

void do_pwd(train_t server_msg);

void do_gets(train_t* cmd_train);

void do_puts();

void do_mkdir(train_t server_msg);

void do_rmdir(train_t server_msg);

void do_token(train_t server_msg);

void do_login(int route_sfd);

void do_register(int route_sfd);

#endif