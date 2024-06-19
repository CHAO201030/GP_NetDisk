#include "../include/task.h"

extern char cur_path[256];

void do_register(int route_sfd)
{
    printf("[INFO] : client do register\n");
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
    login_info.state = CMD_REGISTER;
    strncpy(login_info.data_buf, user_name, login_info.data_len);
    
    send_cmd(&login_info, CMD_REGISTER, route_sfd);
    
    // 接收是否存在
    int is_exist = -1;
    recvn(route_sfd, &is_exist, sizeof(is_exist));
    if(is_exist == 0)
    {
        // 注册流程 接收salt
        char salt[16] = {0};
        recvn(route_sfd, salt, sizeof(salt));

        // 根据salt生成密文密码发送给服务器
        char *encrypted_passwd = crypt(user_passwd, salt);
        
        char *p = strtok(encrypted_passwd, "$");
        p = strtok(NULL, "$");
        p = strtok(NULL, "$");
        char ec_passwd[128] = {0};
        strncpy(ec_passwd, p, strlen(ec_passwd));
        
        sendn(route_sfd, ec_passwd, sizeof(ec_passwd));
        
        int register_success = 0;
        recvn(route_sfd, &register_success, sizeof(register_success));
        printf("[INFO] : Register %s success...\n", user_name);
        return;
    }
    else
    {
        // 用户名已存在
        printf("[INFO] : Register false please restart client...\n");
        exit(-1);
    }
}
