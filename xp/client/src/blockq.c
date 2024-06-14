#include "../inc/blockq.h"

// 创建空的阻塞队列
blockq_t* blockq_create(void) {
    blockq_t* q = (blockq_t*) calloc(1, sizeof(blockq_t));
    // 条件变量的初始化
    pthread_cond_init(&q->not_full, NULL);
    pthread_cond_init(&q->not_empty, NULL);
    // 初始化互斥锁
    pthread_mutex_init(&q->mutex, NULL);
    return q;
}

// 销毁阻塞队列
void blockq_destroy(blockq_t* q) {
    // 销毁条件变量
    pthread_cond_destroy(&q->not_full);
    pthread_cond_destroy(&q->not_empty);
    // 销毁互斥锁
    pthread_mutex_destroy(&q->mutex);
    // 释放空间
    free(q);
}

// 判空
bool blockq_isempty(blockq_t* q) {
    pthread_mutex_lock(&q->mutex);
    bool result = (q->size == 0);
    pthread_mutex_unlock(&q->mutex);
    return result;
}

// 判满
bool blockq_isfull(blockq_t* q) {
    pthread_mutex_lock(&q->mutex);
    bool result = (q->size == N);
    pthread_mutex_unlock(&q->mutex);
    return result;
}

// 查看队头元素
task_t blockq_peek(blockq_t* q) {
    // 上锁
    pthread_mutex_lock(&q->mutex);
    // 等待 not_empty 条件成立
    while (q->size == 0) {
        // 等待
        // 可能会存在虚假唤醒现象
        pthread_cond_wait(&q->not_empty, &q->mutex);
    } // q->size != 0

    // 保证 not_empty 条件成立
    task_t task = q->elements[q->front];
    // 释放锁
    pthread_mutex_unlock(&q->mutex);
    return task;
}

// 入队列
void blockq_push(blockq_t* q, task_t task) {
    pthread_mutex_lock(&q->mutex);
    // 等待 not_full 条件成立
    while (q->size == N) {
        pthread_cond_wait(&q->not_full, &q->mutex);
    }
    // not_full 条件成立
    // 入队列
    q->elements[q->rear] = task;
    q->rear = (q->rear + 1) % N;
    q->size++;
    // 队列一定不空，唤醒等待 not_empty 条件成立的线程
    // 至少会唤醒一个线程，可能会唤醒多个。
    // signal不会释放锁
    pthread_cond_signal(&q->not_empty);
    pthread_mutex_unlock(&q->mutex);
}

// 出队列
task_t blockq_pop(blockq_t* q) {
    pthread_mutex_lock(&q->mutex);
    // 等待 not_empty 条件成立
    while (q->size == 0) {
        pthread_cond_wait(&q->not_empty, &q->mutex);
    }
    // not_empty 条件成立
    // 出队列
    task_t task = q->elements[q->front];
    q->front = (q->front + 1) % N;
    q->size--;
    // 唤醒等待 not_full 条件成立的线程
    pthread_cond_signal(&q->not_full);
    pthread_mutex_unlock(&q->mutex);
    return task;
}
