#include "../inc/ThreadPool.h"
#include "../inc/Bussiness.h"
#include "../inc/user.h"

// void cleanup(void* arg) {
//     ThreadPool* pool = (ThreadPool*)arg;
//     pthread_rwlock_unlock(&pool->rwlock);
// }

extern User_t user_info[1024];

void* event_loop(void* arg) {
    ThreadPool* pool = (ThreadPool*) arg;
    pthread_t tid = pthread_self();
    // 线程清理函数
    //  pthread_cleanup_push(cleanup, pool);
    for(;;) {
        task_t task;
        task = blockq_pop(pool->task_queue); // 阻塞点
        if (task.connfd == -1) {
            printf("%lu exits\n", tid);
            pthread_exit(0);
        }
        
        printf("%lx: handle request from %d\n", tid, task.connfd);
        // 子进程处理业务逻辑
        if(task.type == SEND) {
            send_file(task.connfd, &user_info[task.connfd], task.file_name);
        }

        if(task.type == RECV) {
            recv_file(task.connfd, &user_info[task.connfd], task.file_name);
        }

        printf("%lx: requst from %d done\n", tid, task.connfd);
        user_info[task.connfd].online = false;
        close(task.connfd);
        user_info[task.connfd].recv_status = RECV_CMD;

    }          
    // pthread_cleanup_pop(0);
}

ThreadPool* create_thread_pool(int n)
{
    ThreadPool* pool = (ThreadPool*)malloc(sizeof(ThreadPool));
    pool->threads = (pthread_t*) calloc(n, sizeof(pthread_t));
    pool->num_threads = n;
    pool->task_queue = blockq_create();

    // 创建线程
    for(int i = 0; i < n; i++) {
        pthread_create(&pool->threads[i], NULL, event_loop, pool);
        printf("INFO: create a thread %lx\n", pool->threads[i]);
    }

    return pool;
}

void destroy_thread_pool(ThreadPool* pool) 
{
    free(pool->threads);
    blockq_destroy(pool->task_queue);
    free(pool);
}
