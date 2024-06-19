#include "thread_pool.h"
#include <crypt.h>

#define INITIAL_BUFFER_SIZE 1024

//1.服务端会先判断用户名是否存在并发送信号
//2.如果用户存在，服务端会判断密码是否正确并发送信号
void getSalt(char* salt,char* passwd){
    int i,j;
    for(i=0,j=0;passwd[i]&&j!=4;++i){
        if(passwd[i]=='$'){
            ++j;
        }
    }
    strncpy(salt,passwd,i-1);
}

int passwd_verify(char* pUsername,char* pPasswd){
    struct spwd* sp;
    char salt[INITIAL_BUFFER_SIZE]={0};
    sp=getspnam(pUsername);
    if(sp==NULL){
        printf("用户不存在\n");
        return 1;//用户不存在信号1
    }
    getSalt(salt,sp->sp_pwdp);//获取盐值

    //验证密码-成功信号2-成功信号3
    if(strcmp(sp->sp_pwdp,crypt(pPasswd,salt))==0){
        printf("验证成功！\n");
        return 2;
    }
    else{
        printf("验证失败！\n");
        return 3;
    }

    return 0;
}