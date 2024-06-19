#ifndef __BLOCK_QUEUE_H__
#define __BLOCK_QUEUE_H__

#include "head.h"

/**
 * @brief 创建阻塞队列
 * @return 返回一个在堆空间分配的阻塞队列指针
*/
block_queue_t* block_queue_create(void);

/**
 * @brief 出队操作
 * @param q 阻塞队列
 * @return 返回队首的任务结构体
*/
task_t block_queue_pop(block_queue_t* q);

/**
 * @brief 查看队头的元素
 * @param q 阻塞队列
 * @return 返回队首的任务结构体
*/
task_t block_queue_peek(block_queue_t* q);

/**
 * @brief 入队操作
 * @param q 阻塞队列
 * @param task 任务结构体
*/
void block_queue_push(block_queue_t* q, task_t task);

/**
 * @brief 销毁阻塞队列
 * @param q 阻塞队列
*/
void block_queue_destroy(block_queue_t* q);

/**
 * @brief 判断阻塞队列是否为空
 * @param q 阻塞队列
 * @return 队列为空返回 true 非空返回 false
*/
bool block_queue_is_empty(block_queue_t* q);

/**
 * @brief 判断阻塞队列是否为满
 * @param q 阻塞队列
 * @return 队列为满返回 true 非满返回 false
*/
bool block_queue_is_full(block_queue_t* q);


#endif