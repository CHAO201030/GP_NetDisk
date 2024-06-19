#include "thread_pool.h"

void rmdirCommand(task_t * task)
{
    printf("execute mkdir command.\n");
    train_t t;
    memset(&t, 0, sizeof(t));
    int err = rmdir(task->data);

    if(err) {
        strcpy(t.buff, "rmdir failed.\n");
        sendn(task->peerfd, &t, strlen(task->data));
        return;
    }
    strcpy(t.buff, "rmdir command succeed.\n");
    sendn(task->peerfd, &t, strlen(task->data));
}
