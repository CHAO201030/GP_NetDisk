#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "head.h"

#define CONFIG_TB_SIZE 32

typedef struct{
    char key[32];
    char value[32];
}KV_Pair;

typedef struct{
    KV_Pair table[CONFIG_TB_SIZE];
    int size;
}config_table_t;

/**
 * @brief 初始化 config 表
 * 
 * @return config_table_t* 返回堆空间分配的 config 表
 */
config_table_t* config_table_create();

/**
 * @brief 将 K-V 键值对插入到 conf_table 中
 * 
 * @param key 键
 * @param value 值
 * @param conf_table 配置信息表 
 */
void config_table_insert(const char *key, const char *value, config_table_t *conf_table);

/**
 * @brief 根据 key 查找配置信息
 * 
 * @param conf_table 配置信息表
 * @param key 键
 * @return const char* 返回 key 对应的配置信息
 */
const char* config_table_find(const config_table_t *conf_table, const char *key);

/**
 * @brief 销毁配置信息表
 * 
 * @param conf_table 配置信息表
 */
void config_table_destroy(config_table_t *conf_table);

/**
 * @brief 读取 config 文件
 * 
 * @param conf_file 配置文件路径
 * @param conf_table 配置信息表
 */
void read_config(const char *conf_file, config_table_t *conf_table);

#endif