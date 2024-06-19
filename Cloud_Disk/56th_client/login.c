#include "client.h"
#define INITIAL_BUFFER_SIZE 1024

int login(int clientfd){
    // 1.用户输入用户名和密码
    // 2.发送用户名和密码给服务端，交由服务端判断
    // 3.接收验证结果信号并提示用户
    //用户不存在信号1
    //验证密码-成功信号2-失败信号3

// 1.用户输入用户名和密码
    printf("请输入用户名和密码：\n");
    char username[INITIAL_BUFFER_SIZE]={0};
    char passwd[INITIAL_BUFFER_SIZE]={0};
    int login_success=0;
while(1){
    memset(username,0,sizeof(username));
    memset(passwd,0,sizeof(passwd));
    printf(">username: ");
    scanf("%s", username);
    //fputc('\n', stdout);
    printf(">passwd: ");
    scanf("%s", passwd);
    //fputc('\n', stdout);
    //fflush(stdin);

// 2.发送用户名，发送密码
    train_t train_username;
    train_username.len=strlen(username);
    strcpy(train_username.buff,username);
    sendn(clientfd,&train_username.len,sizeof(int));
    sendn(clientfd,&train_username.buff,train_username.len);

    train_t train_passwd;
    train_passwd.len=strlen(passwd);
    strcpy(train_passwd.buff,passwd);
    sendn(clientfd,&train_passwd.len,sizeof(int));
    sendn(clientfd,&train_passwd.buff,train_passwd.len);

// 3.接收验证结果信号并提示用户
    //用户不存在信号1
    //验证密码-成功信号2-失败信号3
    int res_verify;
    recv_verify_res(clientfd,&res_verify);
    switch (res_verify)
    {
    case 1:
        printf("用户不存在\n");
        break;
    case 2:
        printf("密码正确\n");
        login_success=1;
        break;
 //     goto end_verify;
    case 3:
        printf("密码错误\n");
        break;
    default:
        printf("其他出错\n");
        break;
    }
    if(login_success){
        break;
    }
}

//end_verify: 
//  printf("结束验证");
    return 0;
}

int  recv_verify_res(int clientfd,int *p_res_verify){
    int recv_res;
    int ret=recv(clientfd,&recv_res,sizeof(recv_res),0);
    ERROR_CHECK(ret,-1,"recv recv_res");

    if(ret==0){
        printf("未收到信号\n");
        return 0;
    }

    if(recv_res!=NULL){
        *p_res_verify=recv_res;
    }
    return recv_res;
}