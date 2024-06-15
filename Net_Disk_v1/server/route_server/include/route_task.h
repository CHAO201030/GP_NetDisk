#ifndef __ROUTE_TASK_H__
#define __ROUTE_TASK_H__

#include "head.h"
#include "trans_file.h"
#include "sql.h"

int cmd_analyse(client_t *cur_client, train_t cmd_train);

int recv_cmd(int client_fd, train_t *cmd_train);

void do_ls(client_t *client, char *cmd);

void do_cd(client_t *client, char *cmd);

void do_rm(client_t *client, char *cmd);

void do_pwd(client_t *client, char *cmd);

void do_gets(client_t *client, char *cmd);

void do_puts(client_t *client, char *cmd);

void do_mkdir(client_t *client, char *cmd);

void do_rmdir(client_t *client, char *cmd);

void do_token(client_t *client, char *cmd);

void do_login(client_t *client, char *cmd);

void do_register(client_t *client, char *cmd);

#endif