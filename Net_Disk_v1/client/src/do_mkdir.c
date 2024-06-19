#include "../include/task.h"

extern char cur_path[256];

void do_mkdir(train_t server_msg)
{
    printf("[INFO] : client do mkdir\n");

    if(strcmp(server_msg.data_buf, "ERROR") == 0)
    {
        printf("[INFO] : mkdir failed\n");
    }
    else
    {
        printf("[INFO] : mkdir success\n");
    }
    
    printf("%s", cur_path);
    fflush(stdout);
}