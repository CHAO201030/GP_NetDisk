/**
 * @file do_login.c
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-06-22
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "../include/route_task.h"
#include "../include/client_manager.h"
extern MYSQL *sql_conn;
int user_exist = 1;

void do_login(client_t *client, char *cmd)
{
    // 接收用户名
    char *username = cmd;
    printf("[INFO] : user -> %s want Login\n", username);

    char salt[32] = {0};
    char sql_passwd[128] = {0};
    int uid = 0;
    int login_ok = 0;
    int ret = sql_do_login(username, salt, sql_passwd, &uid);
    int query_signal = 0;
    if (ret == 0) { query_signal = 1; }

    // 发送查询信号并进行错误处理
    if (sendn(client->fd, &query_signal, sizeof(query_signal)) != sizeof(query_signal)) {
        perror("[ERROR] : sendn query_signal failed");
        return;
    }

    if (ret == 0)
    {
        // 发送盐值并进行错误处理
        if (sendn(client->fd, salt, sizeof(salt)) != sizeof(salt)) {
            perror("[ERROR] : sendn salt failed");
            return;
        }

        // 接收加密密文并进行错误处理
        char encryted_passwd[128] = {0};
        if (recvn(client->fd, encryted_passwd, sizeof(encryted_passwd)) != sizeof(encryted_passwd)) {
            perror("[ERROR] : recvn encryted_passwd failed");
            return;
        }

        /* printf("encryted_passwd_len: %ld\n", strlen(encryted_passwd));
        printf("encryted_passwd: %s\n", encryted_passwd);
        printf("sql_passwd_len: %ld\n", strlen(sql_passwd));
        printf("sql_passwd: %s\n", sql_passwd); */

        if (strcmp(encryted_passwd, sql_passwd) == 0)
        {
            login_ok = 1;
            // 初始化结构体
            client->uid = uid;
            client->code = 0;
            client->pre_code = -1;
            strncpy(client->name, username, strlen(username));
            strcpy(client->path, "~");
            printf("[INFO] : password -> verify pass\n");
        }
        else
        {
            printf("[INFO] : password -> verify fail\n");
        }

        // 发送用户存在标志并进行错误处理
        if (sendn(client->fd, &user_exist, sizeof(user_exist)) != sizeof(user_exist)) {
            perror("[ERROR] : sendn user_exist failed");
            return;
        }

        // 发送登录标志并进行错误处理
        if (sendn(client->fd, &login_ok, sizeof(login_ok)) != sizeof(login_ok)) {
            perror("[ERROR] : sendn login_ok failed");
            return;
        }
    }
    else
    {
        // 发送用户存在标志并进行错误处理
        if (sendn(client->fd, &user_exist, sizeof(user_exist)) != sizeof(user_exist)) {
            perror("[ERROR] : sendn user_exist failed");
            return;
        }

        // 发送登录标志并进行错误处理
        if (sendn(client->fd, &login_ok, sizeof(login_ok)) != sizeof(login_ok)) {
            perror("[ERROR] : sendn login_ok failed");
            return;
        }
    }
}

int sql_do_login(const char *user_name, char *salt, char *sql_passwd, int *uid)
{
    MYSQL_RES *sql_result = NULL;
    MYSQL_ROW sql_row;

    char query[1024] = {0};
    sprintf(query, "SELECT salt,crypt_passwd,user_id FROM user_table WHERE user_name='%s'", user_name);
    //printf("query%s\n",query);
    if (mysql_query(sql_conn, query))
    {
       
        printf("query error\n");
        //if (sql_result) mysql_free_result(sql_result); // 确保释放资源
        return -1;
    }

    sql_result = mysql_store_result(sql_conn);
    if (sql_result == NULL)
    {
        printf("store result error\n");
        mysql_free_result(sql_result);
        return -1;
    }
    
    // 检查结果集是否为空
    if (mysql_num_rows(sql_result) == 0)
    {
        // 用户名不存在
        user_exist = 0;
        printf("[INFO] : user %s does not exist\n", user_name);
        mysql_free_result(sql_result);
        return -1;
    }
    
    while ((sql_row = mysql_fetch_row(sql_result)))
    {
        unsigned long *lengths;
        lengths = mysql_fetch_lengths(sql_result);

        strncpy(salt, sql_row[0], lengths[0]);
        salt[lengths[0]] = '\0'; // 确保字符串以空字符结尾

        strncpy(sql_passwd, sql_row[1], lengths[1]);
        sql_passwd[lengths[1]] = '\0'; // 确保字符串以空字符结尾

        *uid = atoi(sql_row[2]);
    }

    mysql_free_result(sql_result);
    return 0;
}