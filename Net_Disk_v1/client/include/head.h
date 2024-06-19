#ifndef __HEAD_H__
#define __HEAD_H__

#include <func.h>

#define ROUTE_IP "192.168.7.121"
#define ROUTE_PORT "9527"

#define BUFFER_SIZE 1024

#define MMAP_SIZE 4096

#define BIG_FILE_SIZE ((off_t)(1<<20) * 100)


typedef enum{
    CMD_MSG,		// 表示传输消息
    CMD_LS,			// LS命令
    CMD_CD,			// CD命令
    CMD_RM,			// RM命令
    CMD_PWD,		// PWD命令
    CMD_GETS,		// GETS命令
    CMD_PUTS,		// PUTS命令
    CMD_EXIT,       // EXIT命令
    CMD_MKDIR,		// MKDIR命令
    CMD_RMDIR,		// RMDIR命令
    CMD_TOKEN,		// TOKEN验证
    CMD_LOGIN,		// 登录
    CMD_REGISTER	// 注册
}CMD_TYPE;

typedef struct{
	int data_len;				// data_buf的长度
	CMD_TYPE state;				// 当前传输的信息类型
	char data_buf[BUFFER_SIZE];	// 传输内容
}train_t;

typedef struct{
    CMD_TYPE state;             // 命令类型 gets or puts
    char cur_cmd[BUFFER_SIZE];  // 当前的命令 gets 10001  or puts 10422
}thread_args;

typedef struct{
    int fd;         // 要下载的文件的fd
    int sfd;        // 文件服务器的fd
    int offset;     // 从哪个位置开始下载
    int part_size;  // 从这个位置开始要下载多少
}pth_gets_args;

#endif