#include "client.h"


void getsHandler(int sockfd, train_t* t){
    sendn(sockfd, t, 4 + 4 + t->len);
    int length = 0;
    int err = recvn(sockfd, (char*)&length, sizeof(length));
    if(err == -1){
        perror("recvFile: recv filename length is error");
        return ;
    }

    if(length==0){
        printf("file %s is not exist.\n", t->buff);
        return;
    }

    int filefd = open(t->buff, O_RDWR|O_CREAT, 0644);
    if(filefd < 0){
        perror("open file");
        return ;
    }
    struct stat state;
    err = fstat(filefd, &state);
    if(err == -1){
        close(filefd);
        perror("fstate is failed");
        return ;
    }

    int filesize = state.st_size;
    err = sendn(sockfd, (char*)&filesize, sizeof(filesize));
    if(err==-1){
        perror("send file size is error");
        return ;
    }
#define BIG_FILE_SIZE (((off_t)1<<20)*100)
off_t part_size = BIG_FILE_SIZE >> 13 << 12;
    if(length>100*1024*1024){
        ftruncate(filefd, length);
        void*p = mmap(NULL, length, 
                      PROT_READ|PROT_WRITE, MAP_SHARED, filefd, 0);
        if(p==NULL){
            close(filefd);
            fprintf(stderr, "mmap is failed\n");
            return;
        }
        int ret = recvn(sockfd, (char*)p, length);
        if(ret == -1){
            close(filefd);
            perror("recvFile: recv file data is failed");
            return ;
        }
        munmap(p, length);
        close(filefd);
        printf("The file is recv successful\n" );
    }else{
        //把插入位置移动到文件末尾,方便断点续传
        lseek(filefd, 0, SEEK_END);
        int donesize = filesize;
        char buff[1000];
        while(donesize<length){
            int len = 0;
            int ret = recvn(sockfd, (char*)&len, sizeof(len));
            if(ret<=0){
                perror("recv file len is failed");
                close(filefd);
                return ;
            }
            ret = recvn(sockfd, buff, len);
            if(ret<=0){
                perror("recv file data is failed");
                close(filefd);
                return ;
            }
            err = write(filefd, buff, ret);
            if(err == -1){
                perror("recvFile: write file data is failed");
                close(filefd);
                return ;
            }
            donesize += ret;
            printf("The file is recv: %5.2f%%\r", 100.00*donesize/length);

            fflush(stdout);
        }
        printf("\n");

        close(filefd);
        if(donesize == length)
            printf("The file is recv successful\n" );
        else
            printf("The file is recv failed\n");
    }
}

