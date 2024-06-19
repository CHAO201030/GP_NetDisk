#include "thread_pool.h"
#include <crypt.h>

#define INITIAL_BUFFER_SIZE 1024

int login_verify(int peerfd){
    char username[INITIAL_BUFFER_SIZE]={0};
    char passwd[INITIAL_BUFFER_SIZE]={0};

    while(1)
{   
    memset(username,0,sizeof(username));
    memset(passwd,0,sizeof(passwd));

    int length;
    ssize_t sret;
   
    sret = recvn(peerfd,&length,sizeof(length));
    ERROR_CHECK(sret,-1,"username length recv");
    sret = recvn(peerfd,username,length);
    ERROR_CHECK(sret,-1,"username recv");
    sret = recvn(peerfd,&length,sizeof(length));
    ERROR_CHECK(sret,-1,"passwd length recv");
    sret = recvn(peerfd,passwd,length);
    ERROR_CHECK(sret,-1,"passwd recv");
    if(sret == 0)
    {
        close(peerfd);
        printf("connection break\n");
        break;
    }
    printf("username:%s passwd:%s\n",username,passwd);

//密码验证-发送验证结果信号给客户端
    int res_verify=passwd_verify(username,passwd);
    printf("验证结果信号：%d\n",res_verify);//换行必须要有

    int ret=sendn(peerfd,&res_verify,sizeof(res_verify));
    ERROR_CHECK(ret,-1,"send res_verify");
    if(ret==0){break;}
    if(res_verify==2){break;}
}
    return 0;
}