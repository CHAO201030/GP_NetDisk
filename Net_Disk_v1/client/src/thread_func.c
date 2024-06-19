#include "../include/task.h"
#include "../include/thread_func.h"
#include "../include/trans_file.h"

void *pth_func(void *args)
{
    thread_args *pth_args = (thread_args *)args;

    switch (pth_args->state)
    {
    case CMD_GETS:  do_gets(pth_args);  break;
    case CMD_PUTS:  do_puts(pth_args);  break;
    default:
        break;
    }

    pthread_exit(0);
}

void *pth_download(void *args)
{
    pth_gets_args *file_info = (pth_gets_args *)args;

    recv_big_file(file_info->sfd, file_info->fd, file_info->offset, file_info->part_size);

    return NULL;
}
