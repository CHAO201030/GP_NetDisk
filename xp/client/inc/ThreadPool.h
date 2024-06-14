#ifndef __WD_THREAD_POOL_H
#define __WD_THREAD_POOL_H 

#include <func.h>
#include "blockq.h"

typedef struct {
    pthread_t* threads; // 保存线程的tid
    int num_threads;    // 线程的数目
    blockq_t* task_queue;
} ThreadPool;

ThreadPool* create_thread_pool(int n);
void destroy_thread_pool(ThreadPool* pool);

#endif
