#include "../inc/md5.h"

void md5sum(char* hash_str, int fd, off_t size) {
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mdctx, EVP_md5(), NULL);

    char buf[4096];
    int pages = size >> 12;
    int others = size % 4096;
    for(int i = 0; i < pages; i++) {
        read(fd, buf, 4096);
        EVP_DigestUpdate(mdctx, buf, 4096);
    }
    read(fd, buf, others);
    EVP_DigestUpdate(mdctx, buf, others);

    unsigned char md5sum[16];
    EVP_DigestFinal_ex(mdctx, md5sum, NULL);

    for (int i = 0; i < 16; i++) {
        sprintf(hash_str + (i * 2), "%02x", md5sum[i]);
    }
    hash_str[32] = '\0';

    EVP_MD_CTX_free(mdctx);
}