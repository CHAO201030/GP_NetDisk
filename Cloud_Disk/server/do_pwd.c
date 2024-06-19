#include "thread_pool.h"

void pwdCommand(task_t * task)
{
    printf("execute pwd command.\n");

    char buff[1000]={0};
    if(getcwd(buff,sizeof(buff))==NULL){
        perror("getcwd");
        return;
    }
    send(task->peerfd,buff,strlen(buff),0);
    
    printf("current working directory is %s\n", buff);
}
