#include "../include/route_task.h"
#include "../include/client_manager.h"

extern MYSQL *sql_conn;

extern int log_fd;

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
    char *user_name = cmd;

    if(sql_check_dup_user_name(user_name) == 0)
    {
        // 已存在用户名
        int is_exist = 1;
        sendn(client->fd, &is_exist, sizeof(is_exist));
        return;
    }
    else
    {
        // 用户名不存在 生成salt发给client
        int is_exist = 0;
        sendn(client->fd, &is_exist, sizeof(is_exist));

        char salt[16] = {0};
        char encrypted_passwd[128] = {0};

        generate_salt(salt);
        sendn(client->fd, salt, sizeof(salt));

        // 保存密文并告知客户端注册成功
        recvn(client->fd, encrypted_passwd, sizeof(encrypted_passwd));

        sql_do_register(user_name, salt, encrypted_passwd);

        int register_success = 1;
        sendn(client->fd, &register_success, sizeof(register_success));
        
        // 打印日志
        LOG_INFO("user %s register\n", user_name);
    }
}

int sql_check_dup_user_name(const char *user_name)
{
    MYSQL_RES *sql_res = NULL;
    MYSQL_ROW sql_row;

    char query[512] = "SELECT user_name FROM user_table WHERE user_name = '";
    strcat(query, user_name);
    strcat(query,"'");

    if(mysql_query(sql_conn, query))
    {
        printf("[INFO] : Error making query: %s\n", mysql_error(sql_conn));
    }
    else
    {
        sql_res = mysql_use_result(sql_conn);
        if(sql_res)
        {
            if((sql_row = mysql_fetch_row(sql_res)) != NULL)
            {
                mysql_free_result(sql_res);
                return 0;
            }
            else
            {
                mysql_free_result(sql_res);
                return -1;
            }
        }
    }
    return -1;
}

int sql_do_register(const char *user_name, const char *salt, const char *encrypted_passwd)
{
    char query[512] = {0};
    sprintf(query, 
            "INSERT INTO user_table(user_name, salt, crypt_passwd) VALUES('%s','%s','%s')",\
            user_name, salt, encrypted_passwd);
    
    if(mysql_query(sql_conn, query))
    {
        printf("[INFO] : Error making query: %s\n", mysql_error(sql_conn));
    }
    else
    {
        printf("[INFO] : %s Register success...\n", user_name);
    }

    return 0;
}
