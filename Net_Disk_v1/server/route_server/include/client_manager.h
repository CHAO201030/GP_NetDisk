#ifndef __H_CLIENT_MANAGER_H__
#define __H_CLIENT_MANAGER_H__

#include "head.h"

#define HASH_MAP_CAPACITY 10
#define HASH_MAP_LOAD_FACTOR 0.75
#define FDS_SIZE 64
#define TIME_OUT 30

/* -------------------- 哈希表 --------------------- */

// 记录 client_fd 在哪个 slot 里 {client_fd : slot_idx}
typedef client_t* K; // client_t *
typedef int V;

typedef struct map_node{
    K client;
    V index;
    struct map_node *next;
}map_node;

typedef struct {
    map_node **table;
    int size;
    int capacity;
    uint32_t hashseed;
}HashMap;

// 创建哈希表
HashMap* hashmap_create(void);

// 销毁哈希表
void hashmap_destroy(HashMap* hash_map);

// 将 {client_fd:slot_idx} 放入到哈希表中
V hashmap_put(HashMap* hash_map, K client, V index);

// 获得 client_fd 当前的 slot_idx
V hashmap_get(HashMap* hash_map, K client);

// 将 {client_fd:slot_idx} 从哈希表中删除
void hashmap_delete(HashMap* hash_map, K client);



/* -------------------- 集合 -----------------------  */

// 存储客户端登录用户的uid

typedef struct set_node{
    int fd;
    struct set_node *next;
}set_node;

typedef struct set{
    set_node *head;
    bool clean_flag;
}set;

int set_add_node(set_node **head, int client_fd);

int set_del_node(set_node **head, int client_fd);

/* -------------------- 超时队列 -------------------- */

typedef struct{
    int cur_index;
    set slot[TIME_OUT];
}time_out_queue;


// 创建超时队列
time_out_queue* tq_create(void);

// 将client_fd插入到超时队列中
int tq_push(time_out_queue *tq, int client_fd);

// 将client_fd从超时队列中删除
int tq_pop(time_out_queue *tq, int slot_idx, int client_fd);

// 清理已经超时的client_fd
int tq_clean_fd(time_out_queue *tq);

// 当 client_fd 与服务器交互时 将其移动到 slot[0] 中
int tq_update_conn(time_out_queue *tq, int old_index, int client_fd);

// 定时器每 1s 就让 client_fd 集合移动到下一个 time_slot 中
int tq_update_timer(time_out_queue *tq);



/* ----------------------- API --------------------- */

// 客户端管理结构由哈希表和超时队列共同完成

// 将 client_fd 添加到客户端管理结构中
int add_client(HashMap *hash_map, time_out_queue *tq, client_t* new_client);

// 将 client_fd 从客户端管理结构中删除
int del_client(HashMap *hash_map, time_out_queue *tq, client_t* new_client);

// 客户端与服务器交互后要更新 client_fd 在客户端管理结构中的位置
int update_client(HashMap *hash_map, time_out_queue *tq, client_t* new_client);

// 定时器 启动！
void timer_start();

client_t *get_cur_client(HashMap *hash_map, int trigger_fd);

#endif