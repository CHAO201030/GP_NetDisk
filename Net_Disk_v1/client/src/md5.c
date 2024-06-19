#include "../include/md5.h"

int get_file_md5(int fd, char *md5sum)
{
    MD5_CTX context;
    MD5_Init(&context);

    struct stat state = {0};
    fstat(fd, &state);
    off_t file_size = state.st_size;
    
    char buf[BUFFER_SIZE] = {0};
    ssize_t read_size = 0;
    ssize_t cur_size = 0;

    while (read_size < file_size)
    {
        cur_size = read(fd, buf, BUFFER_SIZE);
        MD5_Update(&context, buf, cur_size);
        read_size += cur_size;
    }

    lseek(fd, 0, SEEK_SET);

    unsigned char temp[16] = {0};
    MD5_Final(temp, &context);
    
    for(int i = 0; i < 16; i++)
    {
        sprintf(md5sum + 2 * i, "%02x", temp[i]);
    }
    return 0;
}
