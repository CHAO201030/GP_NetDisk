#include <func.h>
#include <openssl/evp.h>

#define MAXLINE 1024

void md5sum(int fd, char* hash_str);