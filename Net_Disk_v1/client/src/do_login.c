#include "../include/task.h"

extern char cur_path[256];

extern char *TOKEN;

void do_login(int route_sfd)
{
    // printf("[INFO] : client do login\n");
    char user_name[32] = {0};
    char user_passwd[64] = {0};

    // 输入用户名
    printf("[INFO] : Input user name: ");
    fflush(stdout);
    int user_name_len = read(STDIN_FILENO, user_name, sizeof(user_name));
    user_name[--user_name_len] = '\0';

    // 输入密码
    printf("[INFO] : Input password : ");
    fflush(stdout);
    int user_passwd_len = read(STDIN_FILENO, user_passwd, sizeof(user_name));
    user_passwd[--user_passwd_len] = '\0';

    // 将用户名发送给服务器
    train_t login_info = {0};

    login_info.data_len = user_name_len;
    strncpy(login_info.data_buf, user_name, login_info.data_len);
    
    send_cmd(&login_info, CMD_LOGIN, route_sfd);

    int classify = -1;
    recvn(route_sfd, &classify, sizeof(classify));

    switch (classify)
    {
    case 0:
    {
        // 用户名存在
        
        // 获取盐值
        char salt[16] = {0};
        recvn(route_sfd, salt, sizeof(salt));

        // 根据盐值获取加密密文 发给服务器
        char *encrypted_passwd = crypt(user_passwd, salt);
        
        char *p = strtok(encrypted_passwd, "$");
        p = strtok(NULL, "$");
        p = strtok(NULL, "$");

        char ec_passwd[128] = {0};
        strncpy(ec_passwd, p, strlen(p));
        
        sendn(route_sfd, ec_passwd, sizeof(ec_passwd));

        bool login_ok = false;
        recvn(route_sfd, &login_ok, sizeof(login_ok));

        if(login_ok)
        {
            // 密码正确
            printf("[INFO] : Login success...\n");

            // 接收服务器发送的TOKEN KEY
            int token_len = 0;
            recvn(route_sfd, &token_len, sizeof(token_len));
            TOKEN = calloc(token_len + 1, sizeof(char));
            recvn(route_sfd, TOKEN, token_len);
            printf("TOKEN : %s\n", TOKEN);
            sprintf(cur_path, "\033[31m%s\033[36m@NetDisk:\033[34m~$\033[0m ", user_name);
            printf("%s", cur_path);
            fflush(stdout);
            return;
        }
        else
        {
            // 密码不正确
            printf("[INFO] : password error...\n");
            exit(-1);
        }
    }
    case 1:
    {
        // 用户名不存在
        printf("[INFO] : user not exist...\n");
        exit(-1);
    }
    default:
    {
        printf("[INFO] : unknow error...\n");
        exit(-1);
    }
    }
}
