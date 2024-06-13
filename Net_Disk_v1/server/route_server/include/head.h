#ifndef __HEAD_H__
#define __HEAD_H__

#include <func.h>
#include <mysql/mysql.h>
#include <l8w8jwt/encode.h>

#define QUEUE_SIZE 1024

#define BUFFER_SIZE 1024

#define MMAP_SIZE 4096

#define BIG_FILE_SIZE ((off_t)(1<<20) * 100)

#define FILE_NAME "file1.txt"

#define ROUTE_IP "192.168.7.121"
#define ROUTE_PORT "9527"

#define CLUSTER_1_IP "192.168.7.121"
#define CLUSTER_1_PORT "9528"

#define CLUSTER_2_IP "192.168.7.121"
#define CLUSTER_2_PORT "9529"

typedef struct{
	int fd; 			// 客户端的fd 
	int uid; 			// SQL : 用户的UID
	int code; 			// SQL : 当前目录的层级
	int pre_code; 		// SQL : 上一级目录的层级
	time_t conn_time; 	// 上一次与服务器交互的时间
	char name[32]; 		// SQL : 用户名称
	char path[128]; 	// SQL : VFS路径
	char token[256]; 	// TOKEN信息
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

typedef struct{
    int front;                          // 队头 
    int rear;                           // 队尾 
    int size;                           // 队列大小 
    pthread_mutex_t mutex;              // 队列锁
    pthread_cond_t not_empty;           // 非空条件变量
    pthread_cond_t not_full;            // 非满条件变量
    client_t client_table[QUEUE_SIZE];  // 循环队列
}block_queue_t;

typedef struct{
    pthread_t *pthid;           // 每个线程的TID
    int thread_num;             // 线程数量
    block_queue_t *block_queue; // 阻塞队列
}thread_pool_t;

#endif