#ifndef __WD_TOKEN_H
#define __WD_TOKEN_H
#include "l8w8jwt/decode.h"
#include "l8w8jwt/encode.h"
#include <func.h>

void encode_token(char** jwt, int id);
int decode_token(const char* jwt, int* id);

#endif