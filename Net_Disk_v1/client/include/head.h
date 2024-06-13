#ifndef __HEAD_H__
#define __HEAD_H__

#include <func.h>

#define BUFFER_SIZE 1024

#define MMAP_SIZE 4096

#define BIG_FILE_SIZE ((off_t)(1<<20) * 100)

#define FILE_NAME "file1.txt"

typedef struct{
	int fd; 			// 客户端的fd 
	int uid; 			// SQL : 用户的UID
	int code; 			// SQL : 当前目录的层级
	int pre_code; 		// SQL : 上一级目录的层级
	time_t conn_time; 	// 上一次与服务器交互的时间
	char name[32]; 		// SQL : 用户名称
	char path[128]; 	// SQL : VFS路径
	char token[128]; 	// token信息
}client_t;

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

#endif