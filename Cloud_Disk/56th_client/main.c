#include "client.h"

int main()
{
    int clientfd = tcpConnect("192.168.93.131", 8080);

    
    //登录验证
    login(clientfd);
   
    char buf[128] = {0};
    train_t t;

    while(1) {
        //读取标准输入中的数据
        memset(buf, 0, sizeof(buf));
        int ret = read(STDIN_FILENO, buf, sizeof(buf));
        if(0 == ret) {
            printf("byebye.\n");
            break;
        }
        memset(&t, 0, sizeof(t));
        //解析命令行
        buf[strlen(buf)-1] = '\0';
        parseCommand(buf, strlen(buf) - 1, &t);
        //处理命令
        commandHandler(clientfd, &t);
    }
    close(clientfd);
    return 0;
}

