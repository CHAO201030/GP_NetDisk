#ifndef __TASK_H__
#define __TASK_H__

#include "file_server.h"

void task_analyse(int new_client_fd);
void do_gets_task(int client_fd, char *cmd);
void do_puts_task();




#endif