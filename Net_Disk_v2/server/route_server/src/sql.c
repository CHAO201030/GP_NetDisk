#include "../include/sql.h"

/*          MYSQL 初始化          */
// 下面的 IP 用户 密码 改为自己的
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

int sql_do_register()
{

}

int sql_do_login()
{

}

int sql_do_ls()
{

}

int sql_get_code()
{

}

int sql_do_cd()
{

}

int sql_do_rm()
{

}

int sql_do_gets()
{

}

int sql_do_puts()
{

}

int sql_do_mkdir()
{
 
}

int sql_do_rmdir()
{

}