#include "thread_pool.h"

void mkdirCommand(task_t * task)
{
    printf("execute mkdir command.\n");
    train_t t;
    memset(&t, 0, sizeof(t));
    int err = mkdir(task->data, 0777);
    
    if(err) {
        strcpy(t.buff, "mkdir successful.\n");
        sendn(task->peerfd, &t, strlen(task->data));
    } 
    strcpy(t.buff, "mkdir command succeed.\n");
    sendn(task->peerfd, &t, strlen(task->data));
}
