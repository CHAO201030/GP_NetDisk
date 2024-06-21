#include "../include/route_task.h"
#include "../include/client_manager.h"

extern MYSQL *sql_conn;

extern int log_fd;

int generate_token(char *token, const client_t* client)
{
    char* jwt;
    size_t jwt_length;
    
    char fd_str[32] = {0};
    sprintf(fd_str, "%d", client->fd);

    char key[] = "client_fd";
    size_t key_len = strlen(key);

    char *value = fd_str;
    size_t value_len = strlen(value);

    struct l8w8jwt_claim payload = {key, key_len, value, value_len, 0};

    struct l8w8jwt_encoding_params params;
    l8w8jwt_encoding_params_init(&params);

    params.alg = L8W8JWT_ALG_HS512; // 加密算法

    params.sub = "Net Disk Token";  // 主题
    params.iss = "Administrator" ;  // 签发人
    params.aud = "Net Disk User" ;  // 受众

    params.iat = l8w8jwt_time(NULL);        // 签发时间
    params.exp = l8w8jwt_time(NULL) + 600;  // 过期时间 600s 10min

    params.additional_payload_claims = &payload; // 指向 payload K-V pair的位置
    params.additional_payload_claims_count = 1;  // payload 数组的大小

    params.secret_key = (unsigned char*)ROUTE_TOKEN_KEY;
    params.secret_key_length = strlen((const char *)params.secret_key);

    params.out = &jwt;
    params.out_length = &jwt_length;

    int r = l8w8jwt_encode(&params);
    if(r == L8W8JWT_SUCCESS)
    {
        printf("[INFO] : user <%s> TOKEN encode success\n", client->name);
    }

    strncpy(token, jwt, jwt_length);

    printf("user <%s> TOKEN : %s\n", client->name, token);

    l8w8jwt_free(jwt);

    return 0;
}

void do_login(client_t *client, char *cmd)
{
    char *user_name = cmd;

    int uid = 0;
    char salt[16] = {0};
    char sql_passwd[128] = {0};
    char recv_passwd[128] = {0};
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
            char token[512] = {0};
            if(generate_token(token, client) == 0)
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

                // 打印日志
                LOG_INFO("user %s login\n", client->name);

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

int sql_do_login(const char *user_name, char *salt, char *sql_passwd, int *uid)
{
    MYSQL_RES *sql_res = NULL;
    MYSQL_ROW sql_row;
    
    char query[512] = "SELECT salt, crypt_passwd, user_id FROM user_table WHERE user_name = '";
    strcat(query, user_name);
    strcat(query, "'");
    
    unsigned int t = mysql_query(sql_conn, query);

    if(t)
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
                strncpy(salt, sql_row[0], 12);
                strncpy(sql_passwd, sql_row[1], 86);
                *uid = atoi(sql_row[2]);
                mysql_free_result(sql_res);
                return 0;
            }
            else
            {
                mysql_free_result(sql_res);
                return -1;
            }
        }
        else
        {
            printf("[INFO] : Don't find user %s\n", user_name);
            mysql_free_result(sql_res);
            return -1;
        }
    }

    return 0;
}
