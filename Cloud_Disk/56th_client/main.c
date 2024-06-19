#include "client.h"

int main()
{
    int clientfd = tcpConnect("192.168.125.131", 8080);

    //登录
    login(clientfd);
    
    char buf[1024] = {0};//lhc

    //4. 使用select进行监听
    fd_set rdset;
    train_t t;
    while(1) {
        FD_ZERO(&rdset);
        FD_SET(STDIN_FILENO, &rdset);
        FD_SET(clientfd, &rdset);
        
        int nready = select(clientfd + 1, &rdset, NULL, NULL, NULL);
        printf("nready:%d\n", nready);
        if(FD_ISSET(STDIN_FILENO, &rdset)) {
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
            parseCommand(buf, strlen(buf), &t);
            sendn(clientfd, &t, 4 + 4 + t.len);
        } else if(FD_ISSET(clientfd, &rdset)) {
            recv(clientfd, buf, sizeof(buf), 0);
            printf("recv:%s\n", buf);
        }
    }
    close(clientfd);
    return 0;
}

