/**
 * @file do_login.c
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-06-19
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "../include/route_task.h"
#include "../include/client_manager.h"

extern MYSQL *sql_conn;

int generate_token(char *token, const client_t* client)
{

}

void do_login(client_t *client, char *cmd)
{

}

int sql_do_login(const char *user_name, char *salt, char *sql_passwd, int *uid)
{

}
