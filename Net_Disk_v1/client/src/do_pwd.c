#include "../include/task.h"

extern char cur_path[256];

void do_pwd(train_t server_msg)
{
    printf("%s\n", server_msg.data_buf);

    printf("%s", cur_path);
    fflush(stdout);
}
