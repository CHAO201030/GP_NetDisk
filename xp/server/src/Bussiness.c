#include "../inc/Bussiness.h"
#include "../inc/mysql_disk.h"
#include "../inc/md5.h"

#define MAXLINE 1024
#define BIG_FILE_SIZE (100 * 1024 * 1024)
#define MMAP_SIZE (1024 * 1024)

extern User_t user_info[1024];

int recvn(int fd, void* buf, int length) {
    int bytes = 0; 
    while (bytes < length) {
        int n = recv(fd, (char*)buf + bytes, length - bytes, 0);
        if (n < 0) {
            close(fd);
            error(0, errno, "recv"); 
            return -1;
        } else if(n == 0) {
            printf("INFO: client shutdown\n");
            user_info[fd].online = false;
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

void send_string(int fd, char* str) {
    int len = strlen(str) + 1;
    sendn(fd, &len, sizeof(len));
    sendn(fd, str, len);
}

void recv_string(int fd, char* str) {
    int len;
    recvn(fd, &len, sizeof(len));
    recvn(fd, str, len);
}

void send_file(int sockfd, User_t* user, char* file_name)
{
    int parent_id = mysql_pwd_to_id(user);
    char query[256];
    sprintf(query, "SELECT md5, filesize FROM files \
                    WHERE parent_id = '%d' AND filename = '%s'", 
                    parent_id, file_name);
    MYSQL_RES* result = mysql_disk_query(query);
    MYSQL_ROW row = mysql_fetch_row(result);
    char file_path[256];
    sprintf(file_path, "/home/yukino/storage/%s", row[0]);
    int fd = open(file_path, O_RDONLY);
    if (fd == -1) {
        error(0, errno, "open %s failed", file_name);
    }
    off_t fsize = atoi(row[1]);
    off_t offset = 0;
    //接收文件的大小
    off_t file_size;
    recvn(sockfd, &file_size, sizeof(file_size));
    if(file_size != 0) {
        //接收文件的md5值
        char md5[33];
        recvn(sockfd, md5, sizeof(md5));
        char md5_check[33];
        md5sum(md5_check, fd, file_size);
        if(strcmp(md5, md5_check) == 0) {
            send_string(sockfd, "MD5OK");
            offset = file_size;
        } else {
            send_string(sockfd, "MD5ERROR");
        }
    }
    // 发送文件的大小
    sendn(sockfd, &fsize, sizeof(fsize));

    // 再发送文件的内容
    off_t bytes = offset; // 已发送的字节数目
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
        lseek(fd, offset, SEEK_SET);
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

void recv_file(int sockfd, User_t* user, char* file_name) {
    //接收文件的md5值
    char md5[33];
    recvn(sockfd, md5, sizeof(md5));

    char query[256];
    sprintf(query, "SELECT filesize FROM files WHERE md5 = '%s'", md5);
    MYSQL_RES* result = mysql_disk_query(query);
    MYSQL_ROW row = mysql_fetch_row(result);
    int parent_id = mysql_pwd_to_id(user);
    off_t fsize;
    if(row != NULL) {
        send_string(sockfd, "FAST_UPLOAD");
        fsize = atoi(row[0]);
    }
    else {
        send_string(sockfd, "NO_FAST_UPLOAD");
        // 打开文件
        char file_path[256];
        sprintf(file_path, "/home/yukino/storage/%s", md5);
        int fd = open(file_path, O_RDWR | O_TRUNC | O_CREAT, 0666);
        if (fd == -1) {
            error(0, errno, "open %s failed", file_name);
        }
        // 接收文件的大小
        recvn(sockfd, &fsize, sizeof(fsize));
        off_t bytes = 0;
        // 再接收文件的内容
        if (fsize >= BIG_FILE_SIZE) {
            ftruncate(fd, fsize);
            // 大文件
            while (bytes < fsize) {
                off_t length;
                if (fsize - bytes >= MMAP_SIZE) {
                    length = MMAP_SIZE;
                }
                else {
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
        }
        else {
            // 小文件
            char buf[MAXLINE];
            while (bytes < fsize) {
                int length;
                if (fsize - bytes >= MAXLINE) {
                    length = MAXLINE;
                }
                else {
                    length = fsize - bytes;
                }
                recvn(sockfd, buf, length);
                write(fd, buf, length);
                bytes += length;
            }
        }
        close(fd);
    }

    mysql_free_result(result);
    char insert[256];
    sprintf(insert, "INSERT INTO files (parent_id,filename,owner_id,md5,filesize,type) \
                                         VALUES('%d','%s','%d','%s','%ld','f')", 
                                        parent_id, file_name, user->id, md5, fsize);
    mysql_disk_insert(insert);
}

