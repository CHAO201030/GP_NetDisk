/**
 * @file do_puts.c
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

void load_balance(char* ip1, char *port1, char *ip2, char* port2)
{

}

void do_puts(client_t *client, char *cmd)
{

}

int sql_is_file_exist(const char *md5sum)
{

}

int sql_do_quick_puts(const client_t *client, const char *md5sum, const char *file_name)
{

}

int sql_do_puts(const client_t *client, const char *file_name, const char *md5sum, int file_size,\
                const char *ip1, const char *port1, int part1_size,\
                const char *ip2, const char *port2, int part2_size)
{

}
