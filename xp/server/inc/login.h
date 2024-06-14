#ifndef __WD_LOGIN_H
#define __WD_LOGIN_H

#include <func.h>
#include "user.h"

#define SALT_SIZE 12

void generate_salt(char *salt, size_t size);
int login_handler(User_t* user);

#endif