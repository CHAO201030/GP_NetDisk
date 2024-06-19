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