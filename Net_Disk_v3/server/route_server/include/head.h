#ifndef __HEAD_H__
#define __HEAD_H__

#define OPENSSL_API_COMPAT 0x00908000L

#include <func.h>
#include <sys/time.h>
#include <mysql/mysql.h>
#include <l8w8jwt/encode.h>
#include <l8w8jwt/decode.h>
#include <openssl/md5.h>

#define QUEUE_SIZE 1024

#define BUFFER_SIZE 1024

#define MMAP_SIZE 4096

#define BIG_FILE_SIZE ((off_t)(1<<20) * 100)

#define ROUTE_IP "route_server_ip"
#define ROUTE_PORT "route_server_port"
#define ROUTE_TOKEN_KEY "route token key hope 2025 SDU to PKU"

#define FILE_SERVER1_IP "file_server1_ip"
#define FILE_SERVER1_PORT "file_server1_port"

#define FILE_SERVER2_IP "file_server2_ip"
#define FILE_SERVER2_PORT "file_server2_port"

#define FILE_SERVER3_IP "file_server3_ip"
#define FILE_SERVER3_PORT "file_server3_port"

#define DEBUG_LOG() do{printf("%s, %d\n", __FILE__, __LINE__);}while(0)

#define LOG_INFO(format, ...) do{\
    time_t t = time(NULL);\
    char log_buf[256] = {0};\
    sprintf(log_buf, "[INFO] : %s", ctime(&t));\
    int log_buf_len = strlen(log_buf);\
    log_buf[log_buf_len - 1] = ' ';\
    sprintf(log_buf + log_buf_len, format, ##__VA_ARGS__);\
    log_buf_len = strlen(log_buf);\
    write(log_fd, log_buf, log_buf_len);\
    printf("%s", log_buf);\
}while(0)

typedef struct{
	int  fd; 			// 客户端的fd 
	int  uid; 			// SQL : 用户的UID
	int  code; 			// SQL : 当前目录的层级
	int  pre_code; 		// SQL : 上一级目录的层级
	char name[32]; 		// SQL : 用户名称
	char path[128]; 	// SQL : VFS路径
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

typedef enum{
    NORMAL_RETURN,
    CLIENT_EXIT,
    TOKEN_FAILED
}cmd_analyse_exit_value;

typedef struct{
	int data_len;				// data_buf的长度
	CMD_TYPE state;				// 当前传输的信息类型
	char data_buf[BUFFER_SIZE];	// 传输内容
}train_t;

typedef struct{
    client_t* p_client;     // 对端 fd
    CMD_TYPE state;         // 任务类型
    char task_info[1024];   // 任务信息
}task_t;

typedef struct{
    int front;                          // 队头 
    int rear;                           // 队尾 
    int size;                           // 队列大小 
    pthread_mutex_t mutex;              // 队列锁
    pthread_cond_t not_empty;           // 非空条件变量
    pthread_cond_t not_full;            // 非满条件变量
    task_t task_queue[QUEUE_SIZE];      // 循环队列
}block_queue_t;

typedef struct{
    pthread_t *pthid;           // 每个线程的TID
    int thread_num;             // 线程数量
    block_queue_t *block_queue; // 阻塞队列
}thread_pool_t;

#endif