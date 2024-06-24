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
#include "../include/task.h"

extern char cur_path[256];

void do_login(int route_sfd)
{
    // 输入用户名
    char username[32] = {0};
    printf("[INFO] : >username:");
    fflush(stdout);
    if (read(STDIN_FILENO, username, sizeof(username)))
    {
        size_t len = strlen(username);
        if (len > 0 && username[len - 1] == '\n')
        {
            username[len - 1] = '\0'; // 去掉换行符
        }
    }
    else
    {
        printf("Error reading input.\n");
        return;
    }

    // 输入密码
    char passwd[128] = {0};
    printf("[INFO] : >passwd:");
    fflush(stdout);
    if (read(STDIN_FILENO, passwd, sizeof(passwd)))
    {
        size_t len = strlen(passwd);
        if (len > 0 && passwd[len - 1] == '\n')
        {
            passwd[len - 1] = '\0'; // 去掉换行符
        }
    }
    else
    {
        printf("Error reading input.\n");
        return;
    }

    // 发送用户名
    train_t t;
    t.data_len = strlen(username);
    t.state = CMD_LOGIN;
    strncpy(t.data_buf, username, t.data_len);
    if (sendn(route_sfd, &t.data_len, sizeof(t.data_len)) != sizeof(t.data_len)) {
        perror("[ERROR] : sendn data_len failed");
        return;
    }
    if (sendn(route_sfd, &t.state, sizeof(t.state)) != sizeof(t.state)) {
        perror("[ERROR] : sendn state failed");
        return;
    }
    if (sendn(route_sfd, t.data_buf, t.data_len) != t.data_len) {
        perror("[ERROR] : sendn data_buf failed");
        return;
    }

    int query_signal = 0;
    if (recvn(route_sfd, &query_signal, sizeof(query_signal)) != sizeof(query_signal)) {
        perror("[ERROR] : recvn query_signal failed");
        return;
    }

    if (query_signal == 1)
    {
        // 接收盐值
        char salt[32] = {0};
        if (recvn(route_sfd, salt, sizeof(salt)) != sizeof(salt)) {
            perror("[ERROR] : recvn salt failed");
            return;
        }

        // crypt生成加密密文
        char *encrypted_passwd = crypt(passwd, salt);
        char *p = strtok(encrypted_passwd, "$");
        p = strtok(NULL, "$");
        p = strtok(NULL, "$");

        // 发送加密密文
        char ec_passwd[128] = {0};
        strncpy(ec_passwd, p, strlen(p));
        if (sendn(route_sfd, ec_passwd, sizeof(ec_passwd)) != sizeof(ec_passwd)) {
            perror("[ERROR] : sendn ec_passwd failed");
            return;
        }

        // 接收用户存在标志
        int user_exist = 1;
        if (recvn(route_sfd, &user_exist, sizeof(user_exist)) != sizeof(user_exist)) {
            perror("[ERROR] : recvn user_exist failed");
            return;
        }
        printf("user_exist: %d\n", user_exist);

        // 接收登录标志
        int login_ok = 0;
        if (recvn(route_sfd, &login_ok, sizeof(login_ok)) != sizeof(login_ok)) {
            perror("[ERROR] : recvn login_ok failed");
            return;
        }

        if (user_exist == 0) {
            printf("[INFO] : user %s not exist\n", username);
            exit(1);
        }
        if (login_ok == 0) {
            printf("[INFO] : %s login failed\n", username);
            exit(1);
        } else {
            printf("[INFO] : %s login success\n", username);
        }
    }
    else
    {
        // 接收用户存在标志
        int user_exist = 1;
        if (recvn(route_sfd, &user_exist, sizeof(user_exist)) != sizeof(user_exist)) {
            perror("[ERROR] : recvn user_exist failed");
            return;
        }

        // 接收登录标志
        int login_ok = 0;
        if (recvn(route_sfd, &login_ok, sizeof(login_ok)) != sizeof(login_ok)) {
            perror("[ERROR] : recvn login_ok failed");
            return;
        }

        if (user_exist == 0) {
            printf("[INFO] : user %s not exist\n", username);
            exit(1);
        }
        if (login_ok == 0) {
            printf("[INFO] : %s login failed\n", username);
            exit(1);
        } else {
            printf("[INFO] : %s login success\n", username);
        }
    }
}