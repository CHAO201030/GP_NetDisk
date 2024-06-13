#ifndef __SQL_H__
#define __SQL_H__

#include "head.h"

int sql_connect_to_database(void);

int sql_disconnect_to_database(void);

int sql_do_register(const char *user_name, const char *salt, const char *encrypted_passwd);

int sql_check_dup_user_name(const char *user_name);

int sql_do_login(const char *user_name, char *salt, char *sql_passwd, int *uid);

int sql_do_ls(const client_t* client, char *server_msg);

int sql_do_cd(int uid, int *code, int *pre_code, const char *target_dir);

int sql_do_rm(const client_t *client, const char *target_file);

int sql_do_gets();

int sql_do_puts();

int sql_do_mkdir(const client_t *client, const char *target_dir);

int sql_do_rmdir(const client_t *client, const char *target_dir);


#endif