#include "../include/sql.h"

/*          MYSQL 初始化          */
MYSQL *sql_conn = NULL;
const char *sql_server = "192.168.7.121";
const char *sql_user = "root";
const char *sql_password = "1234";
const char *sql_database = "Net_Disk";

int sql_connect_to_database()
{
    sql_conn = mysql_init(NULL);
    if(!mysql_real_connect(sql_conn, sql_server, sql_user, sql_password, sql_database, 0, NULL, 0))
    {
        printf("[INFO] : Error connecting to database: %s\n", mysql_error(sql_conn));
        exit(-1);
    }
    printf("[INFO] : Connected to databases success...\n");
    return 0;
}

int sql_disconnect_to_database()
{
    mysql_close(sql_conn);
    return 0;
}

int sql_check_dup_user_name(const char *user_name)
{
    MYSQL_RES *sql_res = NULL;
    MYSQL_ROW sql_row;

    char query[512] = "SELECT user_name FROM user_table WHERE user_name = '";
    strcat(query, user_name);
    strcat(query,"'");

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
    
    unsigned int t = mysql_query(sql_conn, query);

    if(t)
    {
        printf("[INFO] : Error making query: %s\n", mysql_error(sql_conn));
    }
    else
    {
        printf("[INFO] : %s Register success...\n", user_name);
    }

    return 0;
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

int sql_do_ls(const client_t* client, char *server_msg)
{
    MYSQL_RES *sql_res = NULL;
    MYSQL_ROW sql_row;

    int pre_code = client->code == 0 ? -1 : client->code;

    char query[512] = {0};
    
    sprintf(query, 
    "SELECT time, file_name, file_type, file_size FROM VFS WHERE owner_uid = %d AND pre_code = %d AND file_count = 1",
        client->uid, pre_code);
    
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
            while((sql_row = mysql_fetch_row(sql_res)) != NULL)
            {
                // time name type size
                strcat(server_msg, sql_row[0]);
                strcat(server_msg, "  ");

                // 目录高亮显示
                if(strcmp(sql_row[2], "d") == 0)
                {
                    strcat(server_msg, "\033[32m");
                    strcat(server_msg, sql_row[1]);
                    strcat(server_msg, "\033[0m");
                }
                else
                {
                    strcat(server_msg, sql_row[1]);
                }

                strcat(server_msg, "\t");
                strcat(server_msg, sql_row[2]);
                strcat(server_msg, "  ");
                strcat(server_msg, sql_row[3]);
                strcat(server_msg, "\n");
            }
            server_msg[strlen(server_msg) - 1] = '\0';
        }
    }

    mysql_free_result(sql_res);

    return 0;    
}

int sql_get_code(int uid, const char *target_dir)
{
    MYSQL_RES *sql_res = NULL;
    MYSQL_ROW sql_row;

    char query[512] = {0};
    sprintf(query,
            "SELECT code FROM VFS WHERE "
            "owner_uid = %d AND file_name = '%s' AND pre_code < %d",
            uid, target_dir, 0);

    int code = 0;
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
            if((sql_row  = mysql_fetch_row(sql_res)) != NULL)
            {
                code = atoi(sql_row[0]);
            }
        }
    }

    mysql_free_result(sql_res);

    return code;
}

int sql_do_cd(int uid, int *code, int *pre_code, const char *target_dir)
{
    MYSQL_RES *sql_res = NULL;
    MYSQL_ROW sql_row;

    char query[512] = {0};

    int classify = -1;

    if(strcmp(target_dir, "..") == 0)
    {
        // 去上一级目录
        if(*code == 0)
        {
            // printf("分类1\n");
            // 当前为根 不能再向上了 path : ~
            return -1;
        }
        else
        {
            // 当前不是根 并且上一级是根 path : ~/dir1 --> ~/
            if(*pre_code == 0)
            {
                // printf("分类2\n");
                *pre_code = -1;
                *code = 0;
                return 0;
            }
            else
            {
                // 当前不是根 上一级也不是根需要查数据库获得code信息 path : ~/dir1/dir2 --> ~/dir1
                // printf("分类3\n");
                sprintf(query, 
                        "SELECT code, pre_code FROM VFS WHERE owner_uid = %d AND code = %d",
                        uid, *pre_code);
                classify = 3;
            }

        }

    }
    else
    {
        // 去下一级目录
        if(*code == 0)
        {
            // 当前为根 path : ~/ --> ~/dir1
            // printf("分类4\n");
            *pre_code = 0;
            sprintf(query,
                    "SELECT code FROM VFS WHERE "
                    "owner_uid = %d AND file_name = '%s' AND pre_code < 0",
                    uid, target_dir);
            classify = 4;
        }
        else
        {
            // 当前不是根
            // printf("分类5\n");
            sprintf(query,
                    "SELECT code, pre_code FROM VFS WHERE "
                    "owner_uid = %d AND pre_code = %d AND file_name = '%s'",
                    uid, *code, target_dir);
            classify = 5;
        }
    }

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
                switch(classify)
                {
                case 3:
                {
                    // 当前不是根 上一级也不是根需要查数据库获得code信息
                    // path : ~/dir1/dir2 --> ~/dir1
                    *code = atoi(sql_row[0]);
                    *pre_code = atoi(sql_row[1]);
                    // printf("code = %d, pre_code = %d\n",*code, *pre_code);

                    // 这个目录是根目录的直接孩子
                    if(*pre_code == -1)*pre_code = 0;
                    
                    break;
                }
                case 4:
                {
                    // 当前为根
                    // path : ~/ --> ~/dir1
                    *code = atoi(sql_row[0]);
                    break;
                }
                case 5:
                {
                    // 当前不是根
                    *code = atoi(sql_row[0]);
                    *pre_code = atoi(sql_row[1]);
                    break;
                }
                }
            }
            else
            {
                printf("[INFO] : cd %s no such directory\n", target_dir);
                mysql_free_result(sql_res);
                return -1;               
            }
        }
    }

    mysql_free_result(sql_res);

    return 0;
}

int sql_do_rm(const client_t *client, const char *target_file)
{
    MYSQL_RES *sql_res = NULL;
    MYSQL_ROW sql_row;

    char query[512] = {0};
    int pre_code = client->code == 0 ? -1 : client->code;

    // 先对文件的存在性进行判断
    sprintf(query,
            "SELECT file_name FROM VFS"
            " WHERE pre_code = %d AND file_name = '%s' AND owner_uid = %d AND file_type = 'f' AND file_count = 1",
            pre_code, target_file, client->uid);

    unsigned int t = mysql_query(sql_conn, query);

    if(t)
    {
        printf("[INFO] : Error making query: %s\n", mysql_error(sql_conn));
        return -1;
    }
    else
    {
        sql_res = mysql_use_result(sql_conn);
        if(sql_res)
        {
            if((sql_row = mysql_fetch_row(sql_res)) != NULL)
            {
                // 同一个连接 第二次QUERY之前一定要释放掉上一次的结果
                // 不然会出现 Error making query: Commands out of sync; you can't run this command now
                mysql_free_result(sql_res);
                // 用户目录下存在该文件 将引用计数变为0标记为已删除
                bzero(query, sizeof(query));
                sprintf(query,
                        "UPDATE VFS SET file_count = 0"
                        " WHERE pre_code = %d AND file_name = '%s' AND owner_uid = %d AND file_type = 'f'",
                        pre_code, target_file, client->uid);

                t = mysql_query(sql_conn, query);
                if(t)
                {
                    printf("[INFO] : Error making query: %s\n", mysql_error(sql_conn));
                    return -1;
                }
                else
                {
                    printf("[INFO] : <%s> rm %s success\n", client->name, target_file);
                    return 0;
                }
            }
            else
            {
                printf("[INFO] : <%s> rm failed no such file\n", client->name);
                mysql_free_result(sql_res);
                return -1;
            }
            
        }
    }

    mysql_free_result(sql_res);
    return -1;
}

int sql_do_gets()
{

}

int sql_do_puts()
{

}

int sql_do_mkdir(const client_t *client, const char *target_dir)
{
    char query[512] = {0};
    int pre_code = client->code == 0 ? -1 : client->code;

    sprintf(query,
            "INSERT INTO VFS(pre_code, file_name, file_type, file_size, file_size_1,file_size_2,file_md5, owner_uid)"
            " VALUES (%d, '%s', 'd', 0, 0, 0,'0', %d)",
            pre_code, target_dir, client->uid);

    unsigned int t = mysql_query(sql_conn, query);

    if(t)
    {
        printf("[INFO] : Error making query: %s\n", mysql_error(sql_conn));
        return -1;
    }
    else
    {
        printf("[INFO] : <%s> mkdir %s success\n", client->name, target_dir);
    }

    return 0;    
}

int sql_do_rmdir(const client_t *client, const char *target_dir)
{
    // 1. 
    MYSQL_RES *sql_res = NULL;
    MYSQL_ROW sql_row;

    char query[512] = {0};
    int pre_code = client->code == 0 ? -1 : client->code;

    // 先对目录的存在性进行判断
    sprintf(query,
            "SELECT code FROM VFS"
            " WHERE pre_code = %d AND file_name = '%s' AND owner_uid = %d AND file_type = 'd' AND file_count = 1",
            pre_code, target_dir, client->uid);

    if(mysql_query(sql_conn, query))
    {
        printf("[INFO] : Error making query: %s\n", mysql_error(sql_conn));
        return -1;
    }
    else
    {
        sql_res = mysql_use_result(sql_conn);
        if((sql_row = mysql_fetch_row(sql_res)) != NULL)
        {
            int target_dir_code = atoi(sql_row[0]);
            mysql_free_result(sql_res);
            
            // 有该目录 判断是否为空
            bzero(query, sizeof(query));
            sprintf(query,
                    "SELECT code FROM VFS"
                    " WHERE pre_code = %d AND owner_uid = %d",
                    target_dir_code, client->uid);

            if(mysql_query(sql_conn, query))
            {
                printf("[INFO] : Error making query: %s\n", mysql_error(sql_conn));
                return -1;
            }
            else
            {
                sql_res = mysql_use_result(sql_conn);
                if((sql_row = mysql_fetch_row(sql_res)) != NULL)
                {
                    // 目录不为空
                    printf("[INFO] : <%s> rmdir %s failed directory not empty\n", client->name, target_dir);
                    mysql_free_result(sql_res);
                    return 1;
                }
                else
                {
                    // 目录为空直接删除
                    mysql_free_result(sql_res);
                    bzero(query, sizeof(query));
                    sprintf(query,
                        "DELETE FROM VFS"
                        " WHERE pre_code = %d AND owner_uid = %d AND file_name = '%s' AND file_type = 'd'",
                        pre_code, client->uid, target_dir);
                    if(mysql_query(sql_conn, query) == 0)
                    {
                        printf("[INFO] : <%s> rmdir %s success\n", client->name, target_dir);
                        return 0;
                    }
                    else
                    {
                        printf("[INFO] : Error making query: %s\n", mysql_error(sql_conn));
                        return -1;
                    }
                }
            }
        }
        else
        {
            // 没有该目录
            printf("[INFO] : <%s> rmdir %s failed no such directory\n", client->name, target_dir);
            mysql_free_result(sql_res);
            return 2;
        }
    }
}
