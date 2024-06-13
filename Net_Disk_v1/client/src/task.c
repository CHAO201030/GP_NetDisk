#include "../include/task.h"

extern char cur_path[256];
extern char *token_key;

void start_menu(void)
{
    system("clear");
    printf("======================================================================\n"
           "|                                                                    |\n"
           "|                         Net Disk v5.0                              |\n"
           "|                                                                    |\n"
           "|       1.LOGIN                                                      |\n"
           "|       2.REGISTER                                                   |\n"
           "|       3.EXIT                                                       |\n"
           "|                                                                    |\n"
           "|                  Hongshan District Children's Programming Team one |\n"
           "======================================================================\n");
}

void send_cmd(train_t* cmd_train, CMD_TYPE type, int route_sfd)
{
    cmd_train->state = type;

    // 发送命令长度
    sendn(route_sfd, &cmd_train->data_len, sizeof(cmd_train->data_len));

    // 发送命令类型
    sendn(route_sfd, &cmd_train->state, sizeof(cmd_train->state));

    // 发送命令内容
    sendn(route_sfd, cmd_train->data_buf, cmd_train->data_len);
}

void command_analyse(train_t* cmd_train, int route_sfd)
{
    if(strncmp(cmd_train->data_buf, "ls", 2) == 0)
    {
        send_cmd(cmd_train, CMD_LS, route_sfd);
    }
    else if(strncmp(cmd_train->data_buf, "cd ", 3) == 0)
    {
        send_cmd(cmd_train, CMD_CD, route_sfd);
    }
    else if(strncmp(cmd_train->data_buf, "rm ", 3) == 0)
    {
        send_cmd(cmd_train, CMD_RM, route_sfd);
    }
    else if(strncmp(cmd_train->data_buf, "pwd", 3) == 0)
    {
        send_cmd(cmd_train, CMD_PWD, route_sfd);
    }
    else if(strncmp(cmd_train->data_buf, "mkdir ", 6) == 0)
    {
        send_cmd(cmd_train, CMD_MKDIR, route_sfd);
    }
    else if(strncmp(cmd_train->data_buf, "rmdir ", 6) == 0)
    {
        send_cmd(cmd_train, CMD_RMDIR, route_sfd);
    }
    else if(strncmp(cmd_train->data_buf, "gets ", 5) == 0)
    {
        do_gets(cmd_train);
    }
    else if(strncmp(cmd_train->data_buf, "puts ", 5) == 0)
    {
        do_puts();
    }
    else if(strncmp(cmd_train->data_buf, "exit", 4) == 0)
    {
        send_cmd(cmd_train, CMD_EXIT, route_sfd);
        exit(0);
    }
    else
    {
        printf("INFO : ERROR command reinput...\n");
        printf("%s", cur_path);
        fflush(stdout);
    }
}

int recv_server_msg(int route_sfd)
{
    train_t server_msg = {0};
    
    // 接收消息长度
    int ret = recvn(route_sfd, &server_msg.data_len, sizeof(server_msg.data_len));

    if(ret == 0)
    {
        printf("[INFO] : Route Server shut down client will cose after GETS/PUTS finish\n");
        return 0;
    }

    recvn(route_sfd, &server_msg.state, sizeof(server_msg.state));
    recvn(route_sfd, server_msg.data_buf, server_msg.data_len);

    switch (server_msg.state)
    {
    case CMD_LS         :do_ls(server_msg);         break;
    case CMD_CD         :do_cd(server_msg);         break;
    case CMD_RM         :do_rm(server_msg);         break;
    case CMD_PWD        :do_pwd(server_msg);        break;
    // case CMD_GETS       :do_gets(server_msg);       break;
    // case CMD_PUTS       :do_puts(server_msg);       break;
    case CMD_MKDIR      :do_mkdir(server_msg);      break;
    case CMD_RMDIR      :do_rmdir(server_msg);      break;
    case CMD_TOKEN      :do_token(server_msg);      break;
    default:
        break;
    }

    return 1;
}

void do_ls(train_t server_msg)
{
    // printf("[INFO] : client recv ls result\n");
    printf("%s\n", server_msg.data_buf);

    printf("%s", cur_path);
    fflush(stdout);
}

void do_cd(train_t server_msg)
{
    printf("[INFO] : client do cd\n");
    printf("recv path = %s\n", server_msg.data_buf);
    
    if(strcmp(server_msg.data_buf, "ERROR") == 0)
    {
        printf("cd: No such directory\n");
        printf("%s", cur_path);
        fflush(stdout);
        return;
    }

    strtok(cur_path, ":");
    int path_len = strlen(cur_path);

    cur_path[path_len + 1] = '\0';
    cur_path[path_len] = ':';

    strcat(cur_path, "\033[34m");
    strcat(cur_path, server_msg.data_buf);
    strcat(cur_path, "$\033[0m ");

    printf("%s", cur_path);
    fflush(stdout);
}

void do_rm(train_t server_msg)
{
    printf("[INFO] : client do rm\n");
    printf("%s\n", server_msg.data_buf);
    printf("%s", cur_path);
    fflush(stdout);
}

void do_pwd(train_t server_msg)
{
    // printf("[INFO] : client do pwd\n");
    printf("%s\n", server_msg.data_buf);

    printf("%s", cur_path);
    fflush(stdout);
}

void *pth_func(void *args)
{


}

void do_gets(train_t* cmd_train)
{
    // 启动子线程 发送KEY给客户端

}

void do_puts()
{

}

void do_mkdir(train_t server_msg)
{
    printf("[INFO] : client do mkdir\n");

    if(strcmp(server_msg.data_buf, "ERROR") == 0)
    {
        printf("[INFO] : mkdir failed\n");
    }
    else
    {
        // do nothing
    }
    printf("%s", cur_path);
    fflush(stdout);
}

void do_rmdir(train_t server_msg)
{
    printf("[INFO] : client do rmdir\n");
    printf("%s\n", server_msg.data_buf);
    printf("%s", cur_path);
    fflush(stdout);
}

void do_token(train_t server_msg)
{
    printf("[INFO] : client recv token\n");

}

void do_login(int route_sfd)
{
    printf("[INFO] : client do login\n");
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
    login_info.state = CMD_LOGIN;
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
        char salt[12] = {0};
        recvn(route_sfd, salt, sizeof(salt));

        // 根据盐值获取加密密文 发给服务器
        char *encrypted_passwd = crypt(user_passwd, salt);
        
        char *p = strtok(encrypted_passwd, "$");
        p = strtok(NULL, "$");
        p = strtok(NULL, "$");

        sendn(route_sfd, p, strlen(p));

        bool login_ok = false;
        recvn(route_sfd, &login_ok, sizeof(login_ok));

        if(login_ok)
        {
            // 密码正确
            printf("[INFO] : Login success...\n");

            // 接收服务器发送的TOKEN KEY
            int key_len = 0;
            recvn(route_sfd, &key_len, sizeof(key_len));
            token_key = calloc(key_len + 1, sizeof(char));
            recvn(route_sfd, token_key, key_len);

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
        char salt[12] = {0};
        recvn(route_sfd, salt, sizeof(salt));

        // 根据salt生成密文密码发送给服务器
        char *encrypted_passwd = crypt(user_passwd, salt);
        
        char *p = strtok(encrypted_passwd, "$");
        p = strtok(NULL, "$");
        p = strtok(NULL, "$");

        sendn(route_sfd, p, strlen(p));
        
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