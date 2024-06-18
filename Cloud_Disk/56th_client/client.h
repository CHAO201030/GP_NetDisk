#pragma once

#include <func.h>

typedef enum {
    CMD_TYPE_PWD=1,
    CMD_TYPE_LS,
    CMD_TYPE_CD,
    CMD_TYPE_MKDIR,
    CMD_TYPE_RMDIR,
    CMD_TYPE_NOTCMD  //不是命令
}CmdType;

typedef struct 
{
    int len;//记录内容长度
    CmdType type;//消息类型
    char buff[1000];//记录内容本身
}train_t;

int tcpConnect(const char * ip, unsigned short port);
int recvn(int sockfd, void * buff, int len);
int sendn(int sockfd, const void * buff, int len);

int parseCommand(const char * input, int len, train_t * pt);

//判断一个字符串是什么命令
int getCommandType(const char * str);

