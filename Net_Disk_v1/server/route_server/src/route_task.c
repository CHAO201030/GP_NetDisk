#include "../include/route_task.h"

int recv_cmd(int client_fd, train_t *cmd_train)
{
    // 接收命令长度
    int ret = recvn(client_fd, &cmd_train->data_len, sizeof(cmd_train->data_len));

    if(ret == 0)return 0;   // 对端断开处理
    
    // 接收命令类型
    recvn(client_fd, &cmd_train->state, sizeof(cmd_train->state));

    // 接收命令内容
    recvn(client_fd, cmd_train->data_buf, cmd_train->data_len);

    return 1;
}

int cmd_analyse(client_t *cur_client, train_t cmd_train)
{

    switch (cmd_train.state)
    {
    case CMD_LS         :do_ls      (cur_client, cmd_train.data_buf);   break;
    case CMD_CD         :do_cd      (cur_client, cmd_train.data_buf);   break;
    case CMD_RM         :do_rm      (cur_client, cmd_train.data_buf);   break;
    case CMD_PWD        :do_pwd     (cur_client, cmd_train.data_buf);   break;
    case CMD_GETS       :do_gets    (cur_client, cmd_train.data_buf);   break;
    case CMD_PUTS       :do_puts    (cur_client, cmd_train.data_buf);   break;
    case CMD_MKDIR      :do_mkdir   (cur_client, cmd_train.data_buf);   break;
    case CMD_RMDIR      :do_rmdir   (cur_client, cmd_train.data_buf);   break;
    case CMD_TOKEN      :do_token   (cur_client, cmd_train.data_buf);   break;
    case CMD_LOGIN      :do_login   (cur_client, cmd_train.data_buf);   break;
    case CMD_REGISTER   :do_register(cur_client, cmd_train.data_buf);   break;
    case CMD_EXIT       :return 1;
    default:
        break;
    }

    return 0;
}

void do_ls(client_t *client, char *cmd)
{
    /*
        1. 根据client的当前目录层级code 查找数据库得到所有pre_code为code的文件
    */

    // 下面的printf在实际项目中要输出到系统日志里
    printf("[INFO] : %s\n", cmd);

    // 调用 sql_xxxxx 函数去从数据库找结果 然后处理结果

    // 发送消息 客户端跳转到recv_server_msg函数执行
    train_t server_msg = {0};
    server_msg.state = CMD_LS;
    sql_do_ls(client, server_msg.data_buf);
    server_msg.data_len = strlen(server_msg.data_buf);

    // 发送结果给客户端
    sendn(client->fd, &server_msg.data_len, sizeof(server_msg.data_len));
    sendn(client->fd, &server_msg.state, sizeof(server_msg.state));
    sendn(client->fd, server_msg.data_buf, server_msg.data_len);
}

void do_cd(client_t *client, char *cmd)
{
    /*
        1. 支持cd到当前目录的子目录 或者到上一级目录
        2. cd后要把路径拼接到当前client的 path[128] 路径中
    */

    printf("[INFO] : %s\n", cmd);

    char *target_dir = strtok(cmd, " ");
    target_dir = strtok(NULL, " ");

    train_t server_msg = {0};
    server_msg.state = CMD_CD;

    if(sql_do_cd(client->uid, &client->code, &client->pre_code, target_dir) == 0)
    {
        if(strcmp(target_dir, "..") == 0)
        {
            int path_len = strlen(client->path);

            // 截断目录
            int i = 0;
            for(i = path_len - 1; i > 0; i--)
            {
                if(client->path[i] == '/')break;
            }
            for(int j = i; j < path_len; j++)
            {
                client->path[j] = '\0';
            }
        }
        else
        {
            // cd 到子目录
            
            // 拼接目录
            strcat(client->path, "/");
            strcat(client->path, target_dir);
        }

        server_msg.data_len = strlen(client->path);
        strncpy(server_msg.data_buf, client->path, server_msg.data_len);
    }
    else
    {
        // 不合法的路径
        sprintf(server_msg.data_buf, "ERROR");
        server_msg.data_len = strlen(server_msg.data_buf);
        printf("%s\n", server_msg.data_buf);
    }
    
    // 发送当前路径给客户端
    sendn(client->fd, &server_msg.data_len, sizeof(server_msg.data_len));
    sendn(client->fd, &server_msg.state, sizeof(server_msg.state));
    sendn(client->fd, server_msg.data_buf, server_msg.data_len);
}

void do_rm(client_t *client, char *cmd)
{
    /*
        1. 将数据库中该文件的引用计数变为0
    */
    printf("[INFO] : %s\n", cmd);
    char *target_file = strtok(cmd, " ");
    target_file = strtok(NULL, " ");

    train_t server_msg = {0};
    server_msg.state = CMD_RM;

    if(sql_do_rm(client, target_file) == 0)
    {
        sprintf(server_msg.data_buf, "rm %s success", target_file);
        server_msg.data_len = strlen(server_msg.data_buf);
    }
    else
    {
        sprintf(server_msg.data_buf, "rm %s failed no such file", target_file);
        server_msg.data_len = strlen(server_msg.data_buf);
    }

    sendn(client->fd, &server_msg.data_len, sizeof(server_msg.data_len));
    sendn(client->fd, &server_msg.state, sizeof(server_msg.state));
    sendn(client->fd, server_msg.data_buf, server_msg.data_len);
}

void do_pwd(client_t *client, char *cmd)
{
    /*
        1. 把path[128]内容传给client
    */
    printf("[INFO] : %s\n", cmd);

    train_t server_msg = {0};
    server_msg.state = CMD_PWD;
    server_msg.data_len = strlen(client->path);
    strncpy(server_msg.data_buf, client->path, server_msg.data_len);

    sendn(client->fd, &server_msg.data_len, sizeof(server_msg.data_len));
    sendn(client->fd, &server_msg.state, sizeof(server_msg.state));
    sendn(client->fd, server_msg.data_buf, server_msg.data_len);
}

void do_gets(client_t *client, char *cmd)
{
    /*
        1. 查数据库获得存储该文件的集群服务器地址
        2. 将集群地址发给客户端子线程
    */
    printf("[INFO] : %s\n", cmd);
}

void do_puts(client_t *client, char *cmd)
{
    /*
        1. 查数据库获得要接收该文件的集群服务器地址
        2. 将集群地址发给客户端子线程 
    */
    printf("[INFO] : %s\n", cmd);

}

void do_mkdir(client_t *client, char *cmd)
{
    /*
        1. 向数据库中插入一个目录类型的文件 所有者为client->uid
    */
    printf("[INFO] : %s\n", cmd);

    char *target_dir = strtok(cmd, " ");
    target_dir = strtok(NULL, " ");
    
    train_t server_msg = {0};

    if(sql_do_mkdir(client, target_dir) == 0)
    {
        server_msg.state = CMD_MKDIR;
        sprintf(server_msg.data_buf, "SUCCESS");
        server_msg.data_len = strlen(server_msg.data_buf);
    }
    else
    {
        server_msg.state = CMD_MKDIR;
        sprintf(server_msg.data_buf, "ERROR");
        server_msg.data_len = strlen(server_msg.data_buf);
    }

    sendn(client->fd, &server_msg.data_len, sizeof(server_msg.data_len));
    sendn(client->fd, &server_msg.state, sizeof(server_msg.state));
    sendn(client->fd, server_msg.data_buf, server_msg.data_len);
}

void do_rmdir(client_t *client, char *cmd)
{
    /*
        1. 将所有pre_code为该目录code的文件一并删除 
    */
    printf("[INFO] : %s\n", cmd);
    char *target_dir = strtok(cmd, " ");
    target_dir = strtok(NULL, " ");

    train_t server_msg = {0};
    server_msg.state = CMD_RMDIR;

    switch (sql_do_rmdir(client, target_dir))
    {
    case 0:
    {
        // 成功删除
        sprintf(server_msg.data_buf,"rmdir %s success", target_dir);
        server_msg.data_len = strlen(server_msg.data_buf);
        break;
    }
    case 1:
    {
        // 目录不为空
        sprintf(server_msg.data_buf,"rmdir %s failed : directory not empty", target_dir);
        server_msg.data_len = strlen(server_msg.data_buf); 
        break;
    }
    case 2:
    {
        // 没有该目录
        sprintf(server_msg.data_buf,"rmdir %s failed : no such directory", target_dir);
        server_msg.data_len = strlen(server_msg.data_buf);
        break;
    }
    default:
    {
        sprintf(server_msg.data_buf, "Unknow error");
        server_msg.data_len = strlen(server_msg.data_buf);
        break;
    }
    }

    // 发送结果给客户端
    sendn(client->fd, &server_msg.data_len, sizeof(server_msg.data_len));
    sendn(client->fd, &server_msg.state, sizeof(server_msg.state));
    sendn(client->fd, server_msg.data_buf, server_msg.data_len);
}

void do_token(client_t *client, char *cmd)
{
    printf("[INFO] : %s\n", cmd);

    
}

int generate_token(char *token, const char *key, const client_t* client)
{
    /*
    char* jwt;
    size_t jwt_length;

    struct l8w8jwt_encoding_params params;
    l8w8jwt_encoding_params_init(&params);

    params.alg = L8W8JWT_ALG_HS512; // 加密算法

    params.sub = "Net Disk Token";   // 主题
    params.iss = "Administrator" ;   // 签发人
    params.aud = "Net Disk User" ;   // 受众

    params.iat = l8w8jwt_time(NULL);        // 签发时间
    params.exp = l8w8jwt_time(NULL) + 600;  // 过期时间 600s 10min

    params.secret_key = (unsigned char*)key;
    params.secret_key_length = strlen(params.secret_key);
    params.out = &jwt;
    params.out_length = &jwt_length;

    int r = l8w8jwt_encode(&params);
    if(r == L8W8JWT_SUCCESS)
    {
        printf("[INFO] : <%s> generate token success...\n", client->name);
        strncpy(token, jwt, 256);
        l8w8jwt_free(jwt);
        
        return 0;
    }
    else
    {
        printf("[INFO] : <%s> generate token failed...\n", client->name);
        l8w8jwt_free(jwt);

        return -1;
    }

    return 0;
    */
   sprintf(token, "test token 12345");
   return 0;
}

void do_login(client_t *client, char *cmd)
{
    /*
        1. 查数据库检查用户是否注册
        2. 检查用户名和密码
    */
    printf("[INFO] : %s\n", cmd);
    char *user_name = cmd;

    printf("[INFO] : user -> %s want Login\n", user_name);

    int uid = 0;
    char salt[12] = {0};
    char sql_passwd[86] = {0};
    char recv_passwd[86] = {0};
    int classify = -1;

    if(sql_do_login(user_name, salt, sql_passwd, &uid) == 0)
    {
        classify = 0;
        sendn(client->fd, &classify, sizeof(classify));

        // 发送盐值
        sendn(client->fd, salt, sizeof(salt));

        // 收取密文密码
        recvn(client->fd, recv_passwd, sizeof(recv_passwd));

        // 对比密码
        if(strcmp(recv_passwd, sql_passwd) == 0)
        {
            /**
             * 1. 密码正确 服务端生成一个TOKEN
             * 2. 发给客户端一个TOKEN {client->uid, client->code, client->pre_code, client->name}
            */

            char token[257] = {0};
            if(generate_token(token, ROUTE_TOKEN_KEY, client) == 0)
            {
                // 发送登录成功标志
                bool login_ok = true;
                sendn(client->fd, &login_ok, sizeof(login_ok));
        
                // 发送TOKEN
                int token_len = strlen(token);
                sendn(client->fd, &token_len, sizeof(token_len));
                sendn(client->fd, token, token_len);
                
                // 初始化client结构体
                client->uid = uid;
                client->code = 0;
                client->pre_code = -1;
                strncpy(client->name, user_name, strlen(user_name));
                strcpy(client->path, "~");

                return ;
            }
            else
            {
                bool login_ok = false;
                sendn(client->fd, &login_ok, sizeof(login_ok));

                return;  
            }
        }
        else
        {
            // TOKEN 生成失败
            bool login_ok = false;
            sendn(client->fd, &login_ok, sizeof(login_ok));

            return;
        }
    }
    else
    {
        // 用户未注册
        classify = 1;
        sendn(client->fd, &classify, sizeof(classify));

        return;
    }
}

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
    /*
        1. 检查name是否已经注册
        2. 注册新用户
    */
    printf("[INFO] : %s\n", cmd);
    
    char *user_name = cmd;
    printf("[INFO] : user -> %s Register\n", user_name);

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

        char salt[13] = {0};
        char encrypted_passwd[87] = {0};

        generate_salt(salt);
        sendn(client->fd, salt, sizeof(salt) - 1);

        // 保存密文并告知客户端注册成功
        recvn(client->fd, encrypted_passwd, sizeof(encrypted_passwd) - 1);

        sql_do_register(user_name, salt, encrypted_passwd);

        int register_success = 1;
        sendn(client->fd, &register_success, sizeof(register_success));
    }
}