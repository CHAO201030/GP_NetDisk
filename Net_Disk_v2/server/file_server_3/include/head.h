#ifndef __HEAD_H__
#define __HEAD_H__

#include <func.h>
#include <mysql/mysql.h>

#define QUEUE_SIZE 1024

#define BUFFER_SIZE 1024

#define BIG_FILE_SIZE ((off_t)(1<<20) * 100)

#define MMAP_SIZE 4096


#define CLUSTER_3_IP "192.168.7.121"
#define CLUSTER_3_PORT "9530"

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
    int task_queue[QUEUE_SIZE];      // 循环队列
}block_queue_t;

typedef struct{
    pthread_t *pthid;           // 每个线程的TID
    int thread_num;             // 线程数量
    block_queue_t *block_queue; // 阻塞队列
}thread_pool_t;


#endif