#include <func.h>
#include <openssl/evp.h>

#define MAXLINE 1024

void md5sum(char* hash_str, int fd, off_t size);