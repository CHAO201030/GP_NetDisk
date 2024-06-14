#ifndef __WD_MYSQL_H
#define __WD_MYSQL_H

#include <func.h>
#include <mysql/mysql.h>
#include "user.h"

void mysql_disk_init(void);

MYSQL_RES* mysql_disk_query(const char* query);
void mysql_disk_insert(const char* insert);
void mysql_disk_update(const char* update);

int mysql_pwd_to_id(User_t* user);

#endif