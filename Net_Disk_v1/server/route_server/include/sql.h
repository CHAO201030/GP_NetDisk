#ifndef __SQL_H__
#define __SQL_H__

#include "head.h"

/**
 * @brief 连接数据库
 * 
 * @return int 返回0
 */
int sql_connect_to_database(void);

/**
 * @brief 断开数据库连接
 * 
 * @return int 返回0
 */
int sql_disconnect_to_database(void);

/**
 * @brief SQL 注册新用户
 * 
 * @param user_name 用户名 
 * @param salt 盐值
 * @param encrypted_passwd 加密后的密码 
 * @return int 成功返回0 失败返回1
 */
int sql_do_register(const char *user_name, const char *salt, const char *encrypted_passwd);

/**
 * @brief SQL 注册时判读是否重名了 重名不允许注册
 * 
 * @param user_name 用户名
 * @return int 成功返回0 失败返回-1
 */
int sql_check_dup_user_name(const char *user_name);

/**
 * @brief SQL 用户登录 根据用户名获取数据库中的盐值 加密密码 UID
 * 
 * @param user_name 用户名
 * @param salt 数据库中的盐值
 * @param sql_passwd 数据库中的加密密码
 * @param uid 数据库中的UID
 * @return int 成功返回0 失败返回-1
 */
int sql_do_login(const char *user_name, char *salt, char *sql_passwd, int *uid);

/**
 * @brief SQL 查询数据库进行 LS 操作
 * 
 * @param client 指向当前的客户
 * @param server_msg 服务器要给客户端发送的 LS 结果信息
 * @return int 返回0
 */
int sql_do_ls(const client_t* client, char *server_msg);

/**
 * @brief SQL 查询数据库进行 CD 操作
 * 
 * @param uid 用户UID
 * @param code 用户所在的当前目录
 * @param pre_code 用户当前目录的上一级目录
 * @param target_dir 目标文件夹
 * @return int 成功返回0 失败返回-1
 */
int sql_do_cd(int uid, int *code, int *pre_code, const char *target_dir);

/**
 * @brief SQL 查询数据库进行 RM 操作
 * 
 * @param client 指向当前的客户
 * @param target_file 目标文件
 * @return int 成功返回0 失败返回-1
 */
int sql_do_rm(const client_t *client, const char *target_file);

/**
 * @brief SQL 查询数据库进行 GETS 操作
 * 
 * @param client 指向当前的客户端
 * @param target_file 目标文件名
 * @param md5sum 目标文件的 MD5 值
 * @param ip1 存储该文件的文件服务器1的IP
 * @param port1 存储该文件的文件服务器1的PORT
 * @param part1_size 存储该文件的文件服务器1应该发送的分片大小
 * @param ip2 存储该文件的文件服务器2的IP
 * @param port2 存储该文件的文件服务器2的PORT
 * @param part2_size 存储该文件的文件服务器2应该发送的分片大小
 * @return int 成功返回0 失败返回-1
 */
int sql_do_gets(const client_t *client, const char *target_file, unsigned char *md5sum, 
                char *ip1, char *port1, int *part1_size,
                char *ip2, char *port2, int *part2_size);

/**
 * @brief SQL 判断当前文件是否在VFS表中
 * 
 * @param md5sum 文件的MD5
 * @return int 成功返回0 失败返回1
 */
int sql_is_file_exist(const char *md5sum);

/**
 * @brief SQL 查询数据库进行秒传操作
 * 
 * @param client 指向当前的客户端
 * @param md5sum 文件的MD5
 * @param target_file 目标文件名
 * @return int 成功返回0 失败返回1
 */
int sql_do_quick_puts(const client_t *client, const char *md5sum, const char *target_file);

/**
 * @brief SQL 查询数据库进行 PUTS 操作
 * 
 * @param client 指向当前的客户端
 * @param file_name 要上传的文件名
 * @param md5sum 文件MD5
 * @param file_size 文件总大小
 * @param ip1 文件服务器1的IP
 * @param port1 文件服务器1的PORT
 * @param part1_size 文件服务器1的分片大小
 * @param ip2 文件服务器2的IP
 * @param port2 文件服务器2的PORT
 * @param part2_size 文件服务器2的分片大小
 * @return int 成功返回0 失败返回-1
 */
int sql_do_puts(const client_t *client, const char *file_name, const char *md5sum, int file_size,
                const char *ip1, const char *port1, int part1_size,
                const char *ip2, const char *port2, int part2_size);

/**
 * @brief SQL 查询数据库进行 MKDIR 操作
 * 
 * @param client 指向当前的客户端
 * @param target_dir 目标文件夹
 * @return int 成功返回0 失败返回-1
 */
int sql_do_mkdir(const client_t *client, const char *target_dir);

/**
 * @brief SQL 查询数据库进行 RMDIR 操作
 * 
 * @param client 指向当前的客户端
 * @param target_dir 目标文件夹
 * @return int 成功返回0 失败返回错误码
 */
int sql_do_rmdir(const client_t *client, const char *target_dir);


#endif