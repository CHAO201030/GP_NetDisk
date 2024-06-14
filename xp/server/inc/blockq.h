#ifndef __WD_BLOCKQ_H
#define __WD_BLOCKQ_H

#include <func.h>
#define N 1024

typedef enum {
    SEND,
    RECV
} task_type;

typedef struct {
    int connfd;
    task_type type;
    char* pwd;
    char file_name[256];
} task_t;

typedef struct {
    task_t elements[N];
    int front;
    int rear;
    int size;
    // 不满
    pthread_cond_t not_full;
    // 不空
    pthread_cond_t not_empty;
    // 互斥锁
    pthread_mutex_t mutex;
} blockq_t;

// API
blockq_t* blockq_create(void);
void blockq_destroy(blockq_t* q);
bool blockq_isempty(blockq_t* q);
bool blockq_isfull(blockq_t* q);
task_t blockq_peek(blockq_t* q);
task_t blockq_pop(blockq_t* q);
void blockq_push(blockq_t* q, task_t task);

#endif
