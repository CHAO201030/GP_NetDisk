#include "thread_pool.h"
#define INITIAL_BUFFER_SIZE 4096

void lsCommand(task_t * task)
{
    printf("execute ls command.\n");

    DIR *pdir;
    char dir_path[1024]={0};
    struct dirent *entry;
    size_t buffer_size = INITIAL_BUFFER_SIZE;
    size_t offset = 0;
    char *buffer = malloc(buffer_size);

    if (buffer == NULL) {
        perror("malloc");
        return;
    }

    // 打开目录
    if(strlen(task->data)==0){
        getcwd(dir_path,sizeof(dir_path));
    }else{
        strcpy(dir_path,task->data);
    }
    pdir = opendir(dir_path);
    if (pdir == NULL) {
        perror("opendir");
        free(buffer);
        return;
    }

    // 读取目录内容并存储到缓冲区
    while ((entry = readdir(pdir)) != NULL) {
        int len = snprintf(NULL, 0, "%s ", entry->d_name);
        if (offset + len + 1 > buffer_size) {
            // 增加缓冲区大小
            buffer_size *= 2;
            buffer = realloc(buffer, buffer_size);
            if (buffer == NULL) {
                perror("realloc");
                closedir(pdir);
                return;
            }
        }
        snprintf(buffer + offset, buffer_size - offset, "%s ", entry->d_name);
        offset += len;
    }

    // 发送缓冲区内容
    if (send(task->peerfd, buffer, offset, 0) < 0) {
        perror("send");
    }
    printf("%s\n",buffer);

    closedir(pdir);
    free(buffer);
}
