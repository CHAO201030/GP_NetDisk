#ifndef __SQL_H__
#define __SQL_H__

#include "head.h"

int sql_connect_to_database(void);

int sql_disconnect_to_database(void);

int sql_do_register();

int sql_do_login();

int sql_do_ls();

int sql_do_cd();

int sql_do_rm();

int sql_do_gets();

int sql_do_puts();

int sql_do_mkdir();

int sql_do_rmdir();


#endif