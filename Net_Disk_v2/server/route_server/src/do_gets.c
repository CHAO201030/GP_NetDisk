/**
 * @file do_gets.c
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

void do_gets(client_t *client, char *cmd)
{  

}

int sql_do_gets(const client_t *client, const char *target_file, unsigned char *md5sum, \
                char *ip1, char *port1, int *part1_size, \
                char *ip2, char *port2, int *part2_size)
{

}
