#include "../inc/md5.h"

void md5sum(int fd, char* hash_str) {
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mdctx, EVP_md5(), NULL);

    char buf[MAXLINE];
    int n;
    while ((n = read(fd, buf, MAXLINE)) > 0) {
        EVP_DigestUpdate(mdctx, buf, n);
    }

    unsigned char md5sum[16];
    EVP_DigestFinal_ex(mdctx, md5sum, NULL);

    for (int i = 0; i < 16; i++) {
        sprintf(hash_str + (i * 2), "%02x", md5sum[i]);
    }
    hash_str[32] = '\0';

    EVP_MD_CTX_free(mdctx);
}