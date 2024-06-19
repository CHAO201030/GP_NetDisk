#include "../include/net_disk.h"

extern HashMap *client_manage_map;
extern time_out_queue *time_queue;

void* pth_func(void *args)
{
    thread_pool_t *p_manager = (thread_pool_t *)args; 
    pthread_t tid = pthread_self();

    printf("[INFO] : thread %ld create\n", tid);
    
    usleep(500);

    while(1)
    {
        printf("[INFO] : thread %ld waiting for job\n", tid);

        task_t new_task = block_queue_pop(p_manager->block_queue);
        
        if(new_task.fd == -1)
        {
            //printf("[INFO] : thread %ld will exit...\n", tid);
            pthread_exit(0);
        }

        printf("[INFO] : thread %ld begin his job, fd = %d\n", tid, new_task.fd);

        if(new_task.state == CMD_GETS)
        {
            client_t *client_child_thread = get_cur_client(client_manage_map, new_task.fd);

            printf("server threads task : %s\n", new_task.task_info);

            do_gets(client_child_thread, new_task.task_info);

            del_client(client_manage_map, time_queue, client_child_thread);

            close(new_task.fd);

            printf("server thread do_gets finished\n");
        }
        else if(new_task.state == CMD_PUTS)
        {
            client_t *client_child_thread = get_cur_client(client_manage_map, new_task.fd);

            printf("server threads task : %s\n", new_task.task_info);

            do_puts(client_child_thread, new_task.task_info);

            del_client(client_manage_map, time_queue, client_child_thread);

            close(new_task.fd);

            printf("server thread do_puts finished\n");
        }
        else
        {
            printf("[INFO] : Error Thread task...\n");
        }

        printf("[INFO] : thread %ld finish his job, fd = %d\n", tid, new_task.fd);
    }
}

thread_pool_t* thread_pool_init(int thread_num)
{
    thread_pool_t *p_manager = (thread_pool_t *)calloc(1, sizeof(thread_pool_t));
    if(p_manager == NULL)
    {
        error(-1, errno, "thread_pool calloc failed\nReason");
    }

    p_manager->pthid = (pthread_t *)calloc(thread_num, sizeof(pthread_t));
    if(p_manager->pthid == NULL)
    {
        error(-1, errno, "thread_pool pthid calloc failed\nReason");
    }

    p_manager->thread_num = thread_num;

    p_manager->block_queue = block_queue_create();

    for(int i = 0; i < thread_num; i++)
    {
        pthread_create(p_manager->pthid + i, NULL, pth_func, (void *)p_manager);
    }

    return p_manager;
}

void thread_pool_exit(thread_pool_t *p_manager)
{
    printf("[INFO] : thread_pool exit begin...\n");

    for(int i = 0; i < p_manager->thread_num; i++)
    {
        task_t exit_flag = {0};
        exit_flag.fd = -1;
        block_queue_push(p_manager->block_queue, exit_flag);
    }

    for(int i = 0; i < p_manager->thread_num; i++)
    {
        pthread_join(p_manager->pthid[i], NULL);
    }
    
    printf("[INFO] : thread_pool exit success...\n");
}

void distribute_task(thread_pool_t *p_manager, task_t new_task)
{
    block_queue_push(p_manager->block_queue, new_task);
}
