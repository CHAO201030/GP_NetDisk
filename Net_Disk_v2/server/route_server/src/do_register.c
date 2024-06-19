/**
 * @file do_register.c
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

void generate_salt(char *salt)
{
    char str[9] = {0};
    int i, flag;
    srand(time(NULL));
    for(i = 0; i < 8; i++)
    {
        flag = rand()%3;
        switch(flag)
        {
        case 0:str[i] = rand() % 26 + 'a'; break;
        case 1:str[i] = rand() % 26 + 'A'; break;
        case 2:str[i] = rand() % 10 + '0'; break;

        }
    }
    strcat(salt, "$6$");
    strcat(salt, str);
    strcat(salt, "$");
}

void do_register(client_t *client, char *cmd)
{

}

int sql_check_dup_user_name(const char *user_name)
{

}

int sql_do_register(const char *user_name, const char *salt, const char *encrypted_passwd)
{

}
