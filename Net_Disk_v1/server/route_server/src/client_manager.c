#include "../include/client_manager.h"

extern HashMap *client_manage_map;

extern time_out_queue *time_queue;

extern int log_fd;

/* -------------------- 哈希表 --------------------- */

HashMap* hashmap_create(void)
{
    HashMap *hash_map = calloc(1, sizeof(HashMap));
    hash_map->table = calloc(HASH_MAP_CAPACITY, sizeof(map_node*));
    hash_map->size = 0;
    hash_map->capacity = HASH_MAP_CAPACITY;
    hash_map->hashseed = time(NULL);

    return hash_map;
}

void hashmap_destroy(HashMap* hash_map)
{
    for(int i = 0; i < hash_map->size; i++)
    {
        map_node *pCur = hash_map->table[i];
        while(pCur)
        {
            map_node *pNext = pCur->next;
            free(pCur);
            pCur = pNext;
        }
    }

    free(hash_map->table);
    
    free(hash_map);
}

uint32_t hash(const void* key, int len, uint32_t seed)
{
    const uint32_t m = 0x5bd1e995;
    const int r = 24;
    uint32_t h = seed ^ len;
    const unsigned char* data = (const unsigned char*)key;

    while (len >= 4)
    {
        uint32_t k = *(uint32_t*)data;

        k *= m;
        k ^= k >> r;
        k *= m;

        h *= m;
        h ^= k;

        data += 4;
        len -= 4;
    }

    switch (len)
    {
    case 3: h ^= data[2] << 16;
    case 2: h ^= data[1] << 8;
    case 1: h ^= data[0];
        h *= m;
    };

    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;

    return h;
}

V hashmap_put(HashMap* hash_map, K client, V index)
{
    // 当前客户端的 fd 不存在添加到 hash_map 中
    // 当前客户端的 fd 存在 更新 slot_index 的值并返回旧值

    int pos = hash(&client->fd, sizeof(client->fd), hash_map->hashseed) % hash_map->capacity;

    map_node *pCur = hash_map->table[pos];

    while(pCur)
    {
        if(pCur->client->fd == client->fd)
        {
            V old_index = pCur->index;
            pCur->index = index;

            return old_index;
        }
        pCur = pCur->next;
    }

    map_node *pNew = calloc(1, sizeof(map_node));

    pNew->client = client;
    pNew->index = index;
    pNew->next = hash_map->table[pos];

    hash_map->table[pos] = pNew;

    hash_map->size++;

    return -1;
}

V hashmap_get(HashMap* hash_map, K client)
{
    int pos = hash(&client->fd, sizeof(client->fd), hash_map->hashseed) % hash_map->capacity;

    map_node *pCur = hash_map->table[pos];

    while(pCur)
    {
        if(pCur->client->fd == client->fd)
        {
            return pCur->index;
        }

        pCur = pCur->next;
    }

    return -1;
}

void hashmap_delete(HashMap* hash_map, K client)
{
    int pos = hash(&client->fd, sizeof(client->fd), hash_map->hashseed) % hash_map->capacity;

    map_node *pPre = NULL;
    map_node *pCur = hash_map->table[pos];

    while(pCur)
    {
        if(pCur->client->fd == client->fd)
        {
            if(pPre == NULL)
            {
                hash_map->table[pos] = pCur->next;
            }
            else
            {
                pPre->next = pCur->next;
            }

            free(pCur->client);
            free(pCur);
            hash_map->size--;

            return;
        }
        pPre = pCur;
        pCur = pCur->next;
    }
}


/* -------------------- 集合 ----------------------- */

int set_add_node(set_node **head, int client_fd)
{
    set_node *pNew = calloc(1, sizeof(set_node));
    pNew->fd = client_fd;

    if(*head == NULL)
    {
        *head = pNew;  
    }
    else
    {
        pNew->next = *head;
        *head = pNew;
    }

    return 0;
}

int set_del_node(set_node **head, int client_fd)
{
    if(*head == NULL)
    {
        return -1;
    }
    else
    {
        set_node *pPre = NULL;
        set_node *pCur = *head;
        while(pCur)
        {
            if(pCur->fd == client_fd)
            {
                if(pPre == NULL)
                {
                    *head = pCur->next;
                }
                else
                {
                    pPre->next = pCur->next;
                }
                
                free(pCur);
                return 0;
            }

            pPre = pCur;
            pCur = pCur->next;
        }

        return -1;
    }
}

/* -------------------- 超时队列 -------------------- */

time_out_queue* tq_create(void)
{
    time_out_queue *tq = calloc(1, sizeof(time_out_queue));
    
    return tq;
}

int tq_push(time_out_queue *tq, int client_fd)
{  
    return set_add_node(&tq->slot[tq->cur_index].head, client_fd);
}

int tq_pop(time_out_queue *tq, int slot_idx, int client_fd)
{
    return set_del_node(&tq->slot[slot_idx].head, client_fd);
}

int tq_clean_fd(time_out_queue *tq)
{
    // 一次清除一个并返回fd
    set_node *pCur = tq->slot[tq->cur_index].head;

    if(tq->slot[tq->cur_index].clean_flag == true && pCur)
    {
        tq->slot[tq->cur_index].head = pCur->next;
        int cur_fd = pCur->fd;
        free(pCur);
        return cur_fd;
    }

    return -1;
}

int tq_update_conn(time_out_queue *tq, int old_index, int client_fd)
{
    set_del_node(&tq->slot[old_index].head, client_fd);
    set_add_node(&tq->slot[tq->cur_index].head, client_fd);

    return 0;
}

int tq_update_timer(time_out_queue *tq)
{
    tq->slot[tq->cur_index].clean_flag = true;
    tq->cur_index = (tq->cur_index + 1) % TIME_OUT;

    return 0;
}


/* --------------------- API ----------------------- */

int add_client(HashMap *hash_map, time_out_queue *tq, client_t* new_client)
{
    /**
     * 1. 新客户端发来连接请求由 client_fd 和 slot_index 构成键值对放入到哈希表中
     * 2. 将 client_fd 放入到超时队列中
    */
    hashmap_put(hash_map, new_client, tq->cur_index);
    tq_push(tq, new_client->fd);

    return 0;
}

int del_client(HashMap *hash_map, time_out_queue *tq, client_t* cur_client)
{
    /**
     * 1. 获得 client_fd 的 slot_index
     * 2. 将 client 从哈希表中删除
     * 2. 根据 slot_index 将 client_fd 从超时队列中删除
    */
    V slot_idx = hashmap_get(hash_map, cur_client);
    hashmap_delete(hash_map, cur_client);
    tq_pop(tq, slot_idx, cur_client->fd);

    return 0;
}

int update_client(HashMap *hash_map, time_out_queue *tq, client_t* cur_client)
{
    /**
     * 1. 更新 client_fd 在哈希表中的 {client_fd:slot_idx}
     * 2. 更新 client_fd 在超时队列中的位置到 slot[cur_index]
    */
    V old_index = hashmap_put(hash_map, cur_client, tq->cur_index);
    tq_update_conn(tq, old_index, cur_client->fd);
    // printf("Now client in slot[%d]\n", tq->cur_index);
    return 0;
}

int clean_client(HashMap *hash_map, time_out_queue *tq)
{
    /**
     * 1. 从超时队列中找到超时结点并删除掉
     * 2. 从哈希表中删除结点
     * 3. 关闭对应的文件描述符
    */
    int cur_fd = -1;
    while((cur_fd = tq_clean_fd(tq)) != -1)
    {
        client_t *cur_client = get_cur_client(hash_map, cur_fd);

        if(cur_client == NULL)return -1;

        // 打印日志
        LOG_INFO("user %s time out\n", cur_client->name);
        
        hashmap_delete(hash_map, cur_client);

        close(cur_fd);
    }
    tq->slot[tq->cur_index].clean_flag = false;

    return 0;
}

client_t *get_cur_client(HashMap *hash_map, int trigger_fd)
{
    int pos = hash(&trigger_fd, sizeof(trigger_fd), hash_map->hashseed) % hash_map->capacity;
    
    map_node *pCur = hash_map->table[pos];
    
    while(pCur)
    {
        if(pCur->client->fd == trigger_fd)
        {
            return pCur->client;
        }
        
        pCur = pCur->next;
    }

    return NULL;
}

void timer_func(int sig_num)
{
    tq_update_timer(time_queue);
    // printf("Timer trigger tq->cur_idx = %d\n", time_queue->cur_index);
    clean_client(client_manage_map, time_queue);
}

void timer_start()
{
    if(signal(SIGALRM, timer_func) == SIG_ERR)
    {
        error(-1, errno, "Set signal SIGALRM\n");
    }

    struct itimerval value;
    struct timeval interval_value = {1, 0};
    value.it_interval = interval_value;
    value.it_value = interval_value;

    int err = setitimer(ITIMER_REAL, &value, NULL);
    if (err)
    {
        error(1, errno, "setitimer");
    }
}
