#include "../inc/mysql_disk.h"

void finish_with_error(MYSQL *conn) {
    mysql_close(conn);
    error(1, 0, "ERROR: %s\n", mysql_error(conn));
}

MYSQL* conn = NULL;

void mysql_disk_init(void){
    conn = mysql_init(NULL);
    MYSQL* p = mysql_real_connect(conn, "localhost", "yukino", "232507", "disk", 0, NULL, 0);
    if (p == NULL) {
        finish_with_error(conn);
    }
}

MYSQL_RES* mysql_disk_query(const char* query) {
    int err = mysql_query(conn, query);
    if (err) {
        error(1, 0, "ERROR: %s\n", mysql_error(conn));
    }

    MYSQL_RES* result = mysql_store_result(conn);

    return result;
}

void mysql_disk_insert(const char* insert) {
    int err = mysql_query(conn, insert);
    if (err) {
        error(1, 0, "ERROR: %s\n", mysql_error(conn));
    }
}

void mysql_disk_update(const char* update) {
    int err = mysql_query(conn, update);
    if (err) {
        error(1, 0, "ERROR: %s\n", mysql_error(conn));
    }
}


int mysql_pwd_to_id(User_t* user) {
    char pwd[256];
    char query[256];
    strcpy(pwd, user->pwd);
    char* dir = strtok(pwd, "/");
    int parent_id = 0;
    MYSQL_RES* result;
    MYSQL_ROW row;
    while(dir != NULL) {
        sprintf(query, "SELECT id FROM files \
                        WHERE parent_id = '%d' AND type = 'd' AND filename = '%s' AND owner_id = '%d'", 
                        parent_id, dir, user->id);
        result = mysql_disk_query(query);
        row = mysql_fetch_row(result);
        if(row == NULL) {
            mysql_free_result(result);
            return -1;
        }
        parent_id = atoi(row[0]);
        dir = strtok(NULL, "/");
        mysql_free_result(result);
    }

    return parent_id;
}

