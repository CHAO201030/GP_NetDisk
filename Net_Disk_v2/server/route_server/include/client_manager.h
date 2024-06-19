#ifndef __H_CLIENT_MANAGER_H__
#define __H_CLIENT_MANAGER_H__

#include "head.h"

#define HASH_MAP_CAPACITY 10
#define HASH_MAP_LOAD_FACTOR 0.75
#define FDS_SIZE 64
#define TIME_OUT 30

/* ---------------------------------------- 哈希表 ---------------------------------------- */

typedef client_t* K;

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


/**
 * @brief 创建用户管理结构的哈希表
 * 
 * @return 返回一个堆空间分配的哈希表
*/
HashMap* hashmap_create(void);

/**
 * @brief 销毁整个哈希表
 * 
 * @param hash_map 哈希表
*/
void hashmap_destroy(HashMap* hash_map);

/**
 * @brief 根据 client 的 fd 值进行哈希并将其放入到 hash_map 中
 *        如果是新结点则直接插入并返回-1, 如果是旧结点则更新index并将old_index返回
 * 
 * @param client 指向一个客户端的指针
 * @param index 指向当前客户端在超时队列中的索引
 * @return 返回当前客户端在超时队列中的索引
*/
V hashmap_put(HashMap* hash_map, K client, V index);

/**
 * @brief 获得 client 在超时队列中的索引
 * 
 * @param hash_map 客户端管理哈希表
 * @param client 指向一个客户端的指针
 * @return 成功返回 client 在超时队列中的索引, 失败返回-1
*/
V hashmap_get(HashMap* hash_map, K client);

/**
 * @brief 将 client 从客户端管理结构体中删除
 * 
 * @param hash_map 客户端管理哈希表
 * @param client 指向一个客户端的指针
*/
void hashmap_delete(HashMap* hash_map, K client);


/* ---------------------------------------- 集合 -------------------------------------------  */

typedef struct set_node{
    int fd;
    struct set_node *next;
}set_node;

typedef struct set{
    set_node *head;
    bool clean_flag;
}set;


/**
 * @brief 将结点插入到集合中
 * 
 * @param head 集合链表的头结点
 * @param client_fd 对应一个客户端的fd
 * @return 返回0
*/
int set_add_node(set_node **head, int client_fd);

/**
 * @brief 将结点从集合中删除
 * 
 * @param head 集合链表的头结点
 * @param client_fd 对应一个客户端的fd
 * @return 成功返回0, 失败返回-1
*/
int set_del_node(set_node **head, int client_fd);


/* ---------------------------------------- 超时队列 ---------------------------------------- */

typedef struct{
    int cur_index;
    set slot[TIME_OUT];
}time_out_queue;

/**
 * @brief 创建超时队列
 * 
 * @return 返回一个堆空间分配的超时队列
 */
time_out_queue* tq_create(void);

/**
 * @brief 将client_fd插入到超时队列中
 * 
 * @param tq 超时队列
 * @param client_fd 客户端fd 
 * @return int 返回0
 */
int tq_push(time_out_queue *tq, int client_fd);

/**
 * @brief 将client_fd从超时队列中删除
 * 
 * @param tq 超时队列
 * @param slot_idx client_fd 在超时队列中的索引
 * @param client_fd 客户端fd
 * @return int 成功返回0, 失败返回-1
 */
int tq_pop(time_out_queue *tq, int slot_idx, int client_fd);

/**
 * @brief 清除连接已经超时的fd 一次清除一个
 * 
 * @param tq 超时队列
 * @return int 成功返回被清除的fd 失败返回-1
 */
int tq_clean_fd(time_out_queue *tq);

/**
 * @brief 由于客户端与服务器进行交互 更新其在超时队列中的位置
 * 
 * @param tq 超时队列
 * @param old_index 客户端在超时队列中的旧索引值
 * @param client_fd 客户端fd
 * @return int 返回0
 */
int tq_update_conn(time_out_queue *tq, int old_index, int client_fd);

/**
 * @brief 定时器每一秒触发一次 让超时队列的index指针向下走一格 并将删除标志设为true
 * 
 * @param tq 超时队列
 * @return int 返回0
 */
int tq_update_timer(time_out_queue *tq);


/* ------------------------------------------ API ------------------------------------------ */

// 客户端管理结构由哈希表和超时队列共同完成

/**
 * @brief 将客户端 new_client 添加到客户端管理结构 hash_map 和 tq 中
 * 
 * @param hash_map 客户端管理结构
 * @param tq 超时队列
 * @param new_client 指向要插入的新客户端
 * @return int 返回0
 */
int add_client(HashMap *hash_map, time_out_queue *tq, client_t* new_client);

/**
 * @brief 将客户端 new_client 从客户端管理结构 hash_map 和 tq 中删除
 * 
 * @param hash_map 客户端管理结构
 * @param tq 超时队列
 * @param cur_client 指向要删除的客户端
 * @return int 成功返回0
 */
int del_client(HashMap *hash_map, time_out_queue *tq, client_t* cur_client);

/**
 * @brief 客户端与服务器交互后更新 client_fd 在超时队列中的位置
 * 
 * @param hash_map 客户端管理结构
 * @param tq 超时队列
 * @param cur_client 指向要更新的客户端
 * @return int 成功返回0
 */
int update_client(HashMap *hash_map, time_out_queue *tq, client_t* cur_client);

// 定时器 启动！
void timer_start();

/**
 * @brief 获得当前触发 epoll 时间的客户端
 * 
 * @param hash_map 客户端管理结果
 * @param trigger_fd 触发 epoll 的 fd
 * @return client_t* 返回触发 epoll 的客户端结构体指针
 */
client_t *get_cur_client(HashMap *hash_map, int trigger_fd);

#endif