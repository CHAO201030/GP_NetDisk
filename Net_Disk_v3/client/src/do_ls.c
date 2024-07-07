#include "../include/task.h"

extern char cur_path[256];

void do_ls(train_t server_msg)
{
    // printf("[INFO] : client recv ls result\n");
    printf("%s\n", server_msg.data_buf);

    printf("%s", cur_path);
    fflush(stdout);
}
