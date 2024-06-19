#include "../include/task.h"

extern char cur_path[256];

void do_cd(train_t server_msg)
{
    printf("[INFO] : client do cd\n");
    printf("recv path = %s\n", server_msg.data_buf);
    
    if(strcmp(server_msg.data_buf, "ERROR") == 0)
    {
        printf("cd: No such directory\n");
        printf("%s", cur_path);
        fflush(stdout);
        return;
    }

    strtok(cur_path, ":");
    int path_len = strlen(cur_path);

    cur_path[path_len + 1] = '\0';
    cur_path[path_len] = ':';

    strcat(cur_path, "\033[34m");
    strcat(cur_path, server_msg.data_buf);
    strcat(cur_path, "$\033[0m ");

    printf("%s", cur_path);
    fflush(stdout);
}
