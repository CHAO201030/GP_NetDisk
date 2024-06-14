#include "../inc/Bussiness.h"
#include "../inc/md5.h"

#define MAXLINE 1024
#define BIG_FILE_SIZE (100 * 1024 * 1024)
#define MMAP_SIZE (1024 * 1024)

int recvn(int fd, void* buf, int length) {
    int bytes = 0; 
    while (bytes < length) {
        int n = recv(fd, (char*)buf + bytes, length - bytes, 0);
        if (n < 0) {
            close(fd);
            error(0, errno, "recv"); 
            return -1;
        } else if(n == 0) {
            printf("INFO: server shutdown\n");
            close(fd);
            return 0;
        }
        bytes += n;
    } // bytes == length
    return 1;
}

int sendn(int fd, void* buf, int length) {
    int bytes = 0;
    while (bytes < length) {
        int n = send(fd, (char*)buf + bytes, length - bytes, MSG_NOSIGNAL);
        if (n < 0) {
            return -1;
        }
        bytes += n;
    } // bytes == length 
    return 0;
}

void recv_string(int fd, char* str) {
    int len;
    recvn(fd, &len, sizeof(len));
    recvn(fd, str, len);
}


void send_string(int fd, char* str) {
    int len = strlen(str) + 1;
    sendn(fd, &len, sizeof(len));
    sendn(fd, str, len);
}

void send_file(int sockfd, char* file_name)
{
    int fd = open(file_name, O_RDONLY);
    if (fd == -1) {
        error(0, errno, "open %s failed", file_name);
    }
    //发送文件的md5
    char md5[33];
    md5sum(fd, md5);
    sendn(sockfd, md5, sizeof(md5));
    char response[MAXLINE];
    recv_string(sockfd, response);
    if(strcmp(response, "FAST_UPLOAD") == 0) {
        close(fd);
        return;
    }
    
    // 发送文件的大小
    struct stat statbuf;
    fstat(fd, &statbuf);
    off_t fsize = statbuf.st_size;
    sendn(sockfd, &fsize, sizeof(fsize));
    lseek(fd, 0, SEEK_SET);
    // 再发送文件的内容
    off_t bytes = 0; // 已发送的字节数目
    if (fsize >= BIG_FILE_SIZE) {
        // 大文件
        while (bytes < fsize) {
            off_t length;
            if (fsize - bytes >= MMAP_SIZE) {
                length = MMAP_SIZE;
            } else {
                length = fsize - bytes;
            }
            // 内存映射
            void* addr = mmap(NULL, length, PROT_READ, MAP_SHARED, fd, bytes);
            sendn(sockfd, addr, length);
            munmap(addr, length);
            bytes += length;
        }
    } else {
        // 小文件
        char buf[MAXLINE];
        while (bytes < fsize) {
            int length;
            if (fsize - bytes >= MAXLINE) {
                length = MAXLINE;
            } else {
                length = fsize - bytes;
            }
            read(fd, buf, length);
            sendn(sockfd, buf, length);
            bytes += length;
        }
    }
    close(fd);
}

void recv_file(int sockfd, char* file_name) {
    // 打开文件
    int fd = open(file_name, O_RDWR | O_CREAT, 0666);
    if (fd == -1) {
        error(0, errno, "open %s failed", file_name);
    }

    //发送已有文件的大小
    struct stat statbuf;
    fstat(fd, &statbuf);
    off_t fsize = statbuf.st_size;
    sendn(sockfd, &fsize, sizeof(fsize));

    off_t offset = 0;
    //如果文件大小不为0，发送已有文件的md5，并检查校验结果
    if(fsize != 0) {
        char md5[33];
        md5sum(fd, md5);
        sendn(sockfd, md5, sizeof(md5));

        //接收md5检查回应
        char response[MAXLINE];
        recv_string(sockfd, response);
        if(strcmp(response, "MD5OK") == 0) {
            offset = fsize;
        }
    }
    
    // 接收文件的大小
    recvn(sockfd, &fsize, sizeof(fsize));
    off_t bytes = offset;
    // 再接收文件的内容
    if (fsize >= BIG_FILE_SIZE) {
        ftruncate(fd, fsize); 
        // 大文件
        while (bytes < fsize) {
            off_t length;
            if (fsize - bytes >= MMAP_SIZE) {
                length = MMAP_SIZE;
            } else {
                length = fsize - bytes;
            }
            // 内存映射
            void* addr = mmap(NULL, length, PROT_READ | PROT_WRITE, 
                              MAP_SHARED, fd, bytes);
            recvn(sockfd, addr, length);
            munmap(addr, length);
            bytes += length;
            // 打印进度条
            printf("%5.2lf%%\r", 100.0 * bytes / fsize);
            fflush(stdout);
        }
    } else {
        // 小文件
        char buf[MAXLINE];
        lseek(fd, offset, SEEK_SET);
        while (bytes < fsize) {
            int length;
            if (fsize - bytes >= MAXLINE) {
                length = MAXLINE;
            } else {
                length = fsize - bytes;
            }
            recvn(sockfd, buf, length);
            write(fd, buf, length);
            bytes += length;
            // 打印进度条
            printf("%5.2lf%%\r", 100.0 * bytes / fsize);
            fflush(stdout);
        }
    }
    close(fd);
}

