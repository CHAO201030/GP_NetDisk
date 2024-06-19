#include "thread_pool.h"

//完成从服务器将文件传输到客户端任务
//二期实现
void getsCommand(task_t * task)
{
    int length = 0;
    int filefd = open(task->data, O_RDWR);                 
    
    if(filefd <= 0){
        sendn(task->peerfd, &length, sizeof(length));
        perror("getsCommand: open file state is failed");
        return;
    }

    struct stat state;
    
    int err = fstat(filefd, &state);
    if(err == -1){
        close(filefd);
        perror("getsCommand: get file state is failed");
        return ;
    }
    
    length = state.st_size;

    err = sendn(task->peerfd, (char*)&length, sizeof(length));    // 发送文件大小
    if(err==-1){
        close(filefd);
        perror("getsCommand: send file state is failed");
        return ;
    }

    int offset = 0;
    err = recvn(task->peerfd, (char*)&offset, sizeof(offset));
    if(err==-1){
        close(filefd);
        perror("getsCommand: recv file offset is failed");
        return ;
    }

    int donesize = 0;   //传输的文件大小

    if(length>100*1024*1024){
        void*p = mmap(NULL, length, 
                      PROT_READ|PROT_WRITE, MAP_SHARED, filefd, 0);   // 文件大于100M时，使用mmap。
        if(p==NULL){
            close(filefd);
            fprintf(stderr, "mmap is failed\n");
            return;
        }
        int ret = sendn(task->peerfd, (char*)p, length);
        if(ret == -1){
            close(filefd);
            perror("sendFile: send file data is failed");
            return ;
        }
        donesize += length;
        munmap(p, length);
        close(filefd);
    }else{
        //偏移位置，进行断点续传
        err = lseek(filefd, offset, SEEK_SET);
        if(err==-1){
            close(filefd);
            perror("getsCommand: lseek file offset is failed");
            return ;
        }
        donesize = offset;
        char buff[1000] = {0};
        while(donesize<length){
            int ret = read(filefd, buff, sizeof(buff));
            if(ret == -1){
                close(filefd);
                perror("sendFile: read file data is failed");
                return ;
            }
            err = sendn(task->peerfd, (char*)&ret, sizeof(ret));
            if(err == -1){
                close(filefd);
                perror("sendFile: send file data is failed");
                return ;
            }

            err = sendn(task->peerfd, buff, ret);
            if(err == -1){
                close(filefd);
                perror("sendFile: send file data is failed");
                return ;
            }
            donesize += ret;
            printf("The file is send: %5.2f%%\r", 100.00*donesize/length);
            fflush(stdout);
        }
        printf("\n");
        close(filefd);
    }
    if(donesize == length){
        fprintf(stderr, "The file %s send successful\n", task->data);
    }
    else{
        fprintf(stderr, "The file %s send failed\n", task->data);
    }
}