#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__

#include "head.h"

thread_pool_t* thread_pool_init(int thread_num);

void distribute_task(thread_pool_t *p_manager, int fd);

void thread_pool_exit(thread_pool_t *p_manager);

#endif