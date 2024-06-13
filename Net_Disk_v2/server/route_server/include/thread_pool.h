#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__

#include "head.h"

thread_pool_t* thread_pool_init(int thread_num);

void thread_pool_exit(thread_pool_t *p_manager);

void distribute_task(thread_pool_t *p_manager, client_t new_client);

#endif