#include "../include/sql.h"
#include "../include/config.h"

extern config_table_t *cfg_table;

MYSQL *sql_conn = NULL;

int sql_connect_to_database()
{
    sql_conn = mysql_init(NULL);

    if(!mysql_real_connect(sql_conn, 
                           config_table_find(cfg_table, "sql_server"),
                           config_table_find(cfg_table, "sql_user"),
                           config_table_find(cfg_table, "sql_passwd"),
                           config_table_find(cfg_table, "sql_database"),
                           0, NULL, 0))
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