#include "client.h"
#include "str_util.h"

int tcpConnect(const char * ip, unsigned short port)
{
    //1. 创建TCP的客户端套接字
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if(clientfd < 0) {
        perror("socket");
        return -1;
    }

    //2. 指定服务器的网络地址
    struct sockaddr_in serveraddr;
    //初始化操作,防止内部有脏数据
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;//指定IPv4
    serveraddr.sin_port = htons(port);//指定端口号
    //指定IP地址
    serveraddr.sin_addr.s_addr = inet_addr(ip);

    //3. 发起建立连接的请求
    int ret = connect(clientfd, (const struct sockaddr*)&serveraddr, sizeof(serveraddr));
    if(ret < 0) {
        perror("connect");
        close(clientfd);
        return -1;
    }
    return clientfd;
}


//其作用：确定接收len字节的数据
int recvn(int sockfd, void * buff, int len)
{
    int left = len;//还剩下多少个字节需要接收
    char * pbuf = buff;
    int ret = -1;
    while(left > 0) {
        ret = recv(sockfd, pbuf, left, 0);
        if(ret == 0) {
            break;
        } else if(ret < 0) {
            perror("recv");
            return -1;
        }

        left -= ret;
        pbuf += ret;
    }
    //当退出while循环时，left的值等于0
    return len - left;
}

//作用: 确定发送len字节的数据
int sendn(int sockfd, const void * buff, int len)
{
    int left = len;
    const char * pbuf = buff;
    int ret = -1;
    while(left > 0) {
        ret = send(sockfd, pbuf, left, 0);
        if(ret < 0) {
            perror("send");
            return -1;
        }

        left -= ret;
        pbuf += ret;
    }
    return len - left;
}

//解析命令
int parseCommand(const char * pinput, int len, train_t * pt)
{
    char * pstrs[10] = {0};
    int cnt = 0;
    splitString(pinput, pstrs, 10, &cnt);
    pt->type = getCommandType(pstrs[0]);
    //暂时限定命令行格式为：
    //1. cmd
    //2. cmd content
    if(cnt > 1) {
        pt->len = strlen(pstrs[1]);
        strncpy(pt->buff, pstrs[1], pt->len);
    }
    return 0;
}

int getCommandType(const char * str)
{
    if(!strcmp(str, "pwd")) 
        return CMD_TYPE_PWD;
    else if(!strcmp(str, "ls"))
        return CMD_TYPE_LS;
    else if(!strcmp(str, "cd"))
        return CMD_TYPE_CD;
    else if(!strcmp(str, "mkdir"))
        return CMD_TYPE_MKDIR;
    else if(!strcmp(str,"rmdir"))
        return CMD_TYPE_RMDIR;
    else if(!strcmp(str,"puts"))
        return CMD_TYPE_PUTS;
    else if(!strcmp(str,"gets"))
        return CMD_TYPE_GETS;
    else 
        return CMD_TYPE_NOTCMD;
}

void commandHandler(int sockfd, train_t *t) {
    switch(t->type){
    case CMD_TYPE_LS:
    case CMD_TYPE_CD:
    
    case CMD_TYPE_PWD:
    case CMD_TYPE_MKDIR:
    case CMD_TYPE_RMDIR:
        //要处理的逻辑
        break;
    case CMD_TYPE_PUTS:
        //要处理的逻辑
        break;
    case CMD_TYPE_GETS:
        getsHandler(sockfd, t);
        break;
    }
}