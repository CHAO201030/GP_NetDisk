#include "../inc/login.h"
#include "../inc/mysql_disk.h"
#include "../inc/Bussiness.h"
#include "../inc/token.h"

void generate_salt(char *salt, size_t size) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    size_t charset_size = sizeof(charset) - 1;  // Exclude the null terminator

    int fd = open("/dev/urandom", O_RDONLY);
    if (fd == -1) {
        error(1, errno, "open");
    }

    unsigned char random_bytes[size];
    if (read(fd, random_bytes, size) != size) {
        close(fd);
        error(1, errno, "read");
    }

    close(fd);

    for (size_t i = 0; i < size; i++) {
        salt[i] = charset[random_bytes[i] % charset_size];
    }
    salt[0] = '$';
    salt[1] = '6';
    salt[2] = '$';
    salt[size - 1] = '$';
    salt[size] = '\0';  // Null-terminate the string
}

int login_handler(User_t* user) {
    if(user->online == false) {
        //接收token长度，如果为0，则为密码登录
        int token_len;
        recvn(user->connfd, &token_len, sizeof(token_len));

        if(token_len == 0) {
            int user_len;
            recvn(user->connfd, &user_len, sizeof(user_len));
            char passwd[MAXLINE];                    
            recvn(user->connfd, passwd, user_len);
            char* user_name = strtok(passwd, " ");
            char* password = strtok(NULL, "\n");

            char query[STR_MAXLEN];
            sprintf(query, "SELECT salt, cryptpasswd FROM users WHERE username = '%s'", user_name);
            MYSQL_RES* result = mysql_disk_query(query);
            MYSQL_ROW row = mysql_fetch_row(result);

            if(row == NULL) {
                char insert[STR_MAXLEN];
                char salt[SALT_SIZE + 1];
                generate_salt(salt, SALT_SIZE);
                char* cryptpasswd = crypt(password, salt);
                sprintf(insert, "INSERT INTO users (username,salt,cryptpasswd,pwd) \
                                             VALUES('%s','%s','%s','/')", 
                                                    user_name, salt, cryptpasswd);
                mysql_disk_insert(insert);
                send_string(user->connfd, "Register success!");
            } else {
                char* salt = row[0];
                char* cryptpasswd = row[1];
                if(strcmp(cryptpasswd, crypt(password, salt)) == 0) {
                    send_string(user->connfd, "Login success!");
                } else {
                    send_string(user->connfd, "Password error!");
                    mysql_free_result(result);
                    return -1;
                }
            }

            mysql_free_result(result);
            sprintf(query, "SELECT id, pwd FROM users WHERE username = '%s'", user_name);
            result = mysql_disk_query(query);
            row = mysql_fetch_row(result);
            user->id = atoi(row[0]);
            strcpy(user->pwd, row[1]);
            mysql_free_result(result);

            //登录或注册成功发送token
            char* jwt;
            encode_token(&jwt, user->id);
            send_string(user->connfd, jwt);
            l8w8jwt_free(jwt);

        } else {
            char token[MAXLINE];
            recvn(user->connfd, token, token_len);

            int id;
            if(decode_token(token, &id) == 0) {
                char query[STR_MAXLEN];
                sprintf(query, "SELECT pwd FROM users WHERE id = '%d'", id);
                MYSQL_RES* result = mysql_disk_query(query);
                MYSQL_ROW row = mysql_fetch_row(result);

                if(row == NULL) {
                    return -1;
                }
                user->id = id;
                strcpy(user->pwd, row[0]);
                mysql_free_result(result);
            }
        }
        
    } 
        
    user->online = true;
    return 0;
}


