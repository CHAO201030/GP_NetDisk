#include "../include/task.h"

extern char cur_path[256];

void do_rmdir(train_t server_msg)
{
    // printf("[INFO] : client do rmdir\n");
    printf("%s\n", server_msg.data_buf);
    printf("%s", cur_path);
    fflush(stdout);
}
