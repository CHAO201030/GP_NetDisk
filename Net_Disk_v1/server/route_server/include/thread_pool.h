#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__

#include "head.h"

/**
 * @brief 线程池初始化
 * 
 * @param thread_num 线程数量
 * @return thread_pool_t* 返回一个堆空间分配的线程池结构体
 */
thread_pool_t* thread_pool_init(int thread_num);

/**
 * @brief 线程池退出
 * 
 * @param p_manager 线程池管理结构
 */
void thread_pool_exit(thread_pool_t *p_manager);

/**
 * @brief 将任务分发给子线程
 * 
 * @param p_manager 线程池结构体
 * @param new_task 新任务
 */
void distribute_task(thread_pool_t *p_manager, task_t new_task);

#endif