#include <func.h>
#include "../inc/ThreadPool.h"
#include "../inc/Network.h"
#include "../inc/user.h"
#include "../inc/mysql_disk.h"
#include "../inc/login.h"
#include "../inc/Bussiness.h"

User_t user_info[1024] = {0};

void cmd_handler(char* cmd, User_t* user, ThreadPool* pool, int epfd);

int exitpipes[2];
void exit_handler(int signo) {
    write(exitpipes[1], "1", 1);
}

int main(int argc, char *argv[])
{
    pipe(exitpipes);
    pid_t pid = fork();
    if (pid != 0) {
        // 父进程
        close(exitpipes[0]);
        // 捕获 SIGURS1 信号
        if (signal(SIGUSR1, exit_handler) == SIG_ERR) {
            error(1, errno, "signal");
        }
        // 等待子进程结束
        wait(NULL);
        exit(0);
    }
    // 子进程
    close(exitpipes[1]);

    // 创建进程池
    // 父进程应该保留子进程的数据
    int num_threads = 8;
    ThreadPool* pool = create_thread_pool(num_threads);


    int epfd = epoll_create(1);
    epoll_add(epfd, exitpipes[0]);

    // 监听端口
    int listenfd = tcp_listen();
    printf("INFO: waiting for connection...\n");
    epoll_add(epfd, listenfd);

    int max_events = 1024;
    struct epoll_event* ready_events = 
        (struct epoll_event*) calloc(max_events, sizeof(struct epoll_event));
    
    for(int i = 0; i < 1024; i++) {
        user_info[i].pwd[0] = '/';
        user_info[i].online = false;
        user_info[i].recv_status = RECV_CMD;
    }

    mysql_disk_init();

    // Main loop
    for(;;) {
        int n = epoll_wait(epfd, ready_events, max_events, -1);

        for (int i = 0; i < n; i++) {
            if (ready_events[i].data.fd == listenfd) {
                // 有新的连接
                struct sockaddr_storage client_addr;
                socklen_t addrlen = sizeof(client_addr);

                int connfd = accept(listenfd, 
                                    (struct sockaddr*)&client_addr,
                                    &addrlen);
                // 打印客户端地址
                char ip_str[50];
                inet_ntop(client_addr.ss_family, 
                          get_ip_addr((struct sockaddr*)&client_addr), 
                          ip_str, 50);

                printf("INFO: new connection from %s, on socket %d\n", ip_str, connfd);
                epoll_add(epfd, connfd);

            } else if (ready_events[i].data.fd == exitpipes[0]) {
                printf("INFO: received exit signal\n");
                // 通知各个子线程退出
                for(int j = 0; j < pool->num_threads; j++) {
                    // pthread_cancel(pool->threads[j]);
                    task_t task = {-1};
                    blockq_push(pool->task_queue, task);
                }
                // 等待各个子线程退出
                for(int j = 0; j < pool->num_threads; j++) {
                    pthread_join(pool->threads[j], NULL);
                }
                printf("INFO: main thread exits\n");
                pthread_exit(0);

            } else {
                // 客户端有请求
                int connfd = ready_events[i].data.fd;
                user_info[connfd].connfd = connfd;

                if(user_info[connfd].online == true && user_info[connfd].recv_status == RECV_CMD) {
                    //接收命令长度
                    int cmd_len;
                    if(recvn(connfd, &cmd_len, sizeof(cmd_len)) == 0) {
                        continue;
                    }
                    //接收命令内容
                    char cmd[MAXLINE];                    
                    recvn(connfd, cmd, cmd_len);
                    printf("INFO fd = %d cmd = %s\n", connfd, cmd);
                    cmd_handler(cmd, &user_info[connfd], pool, epfd);
                }

                // 检查登录状态
                if(login_handler(&user_info[connfd]) == -1) {
                    continue;
                }
                
            }
        }
    }

    return 0;
}

void cmd_handler(char* cmd, User_t* user, ThreadPool* pool, int epfd) {
    char* cmd_head = strtok(cmd, " ");

    if(strcmp(cmd_head, "cd") == 0) {
        char* dir = strtok(NULL, "\n");
        char pwd[STR_MAXLEN];
        strcpy(pwd, user->pwd);

        if(strcmp(dir, "..") == 0) {
            char* p = pwd;
            char* q = p;
            while(*p) {
                if(*p == '/') {
                    q = p;
                }
                p++;
            }
            if(q == pwd) {
                q[1] = '\0';
            } else {
                q[0] = '\0';
            }    
        } else {
            if(dir[0] == '/') {
                strcpy(pwd, dir);
            } else {
                if(pwd[1] == '\0') {
                    strcat(pwd, dir);
                } else {
                    strcat(pwd, "/");
                    strcat(pwd, dir);
                }
            }            
        }

        //更新用户当前目录
        char pwd_old[STR_MAXLEN];
        strcpy(pwd_old, user->pwd);
        strcpy(user->pwd, pwd);
        if(mysql_pwd_to_id(user) == -1) {
            strcpy(user->pwd, pwd_old);
            send_string(user->connfd, "Dir not find");
        } else {
            char update[MAXLINE];
            sprintf(update, "UPDATE users SET pwd = '%s' \
                            WHERE id = '%d'", 
                            user->pwd, user->id);
            mysql_disk_update(update);
        }


    }

    else if(strcmp(cmd_head, "ls\n") == 0) {
        int parent_id = mysql_pwd_to_id(user);
        char query[STR_MAXLEN];
        sprintf(query, "SELECT filename FROM files \
                        WHERE parent_id = '%d' AND tomb <> '1' AND owner_id = '%d'", 
                        parent_id, user->id);
        
        MYSQL_RES* result = mysql_disk_query(query);
        MYSQL_ROW row;
        char msg[MAXLINE] = {0};
        while((row = mysql_fetch_row(result)) != NULL) {
            strcat(msg, row[0]);
            strcat(msg, "\t");
        }
        send_string(user->connfd, msg);
        mysql_free_result(result);
    }

    else if(strcmp(cmd_head, "pwd\n") == 0) {
        send_string(user->connfd, user->pwd);
    }

    else if(strcmp(cmd_head, "puts") == 0) {
        char* file_name = strtok(NULL, "\n");
        task_t task = {user->connfd, RECV, user->pwd};
        strcpy(task.file_name, file_name);
        user->recv_status = RECV_FILE;
        epoll_del(epfd, user->connfd);
        blockq_push(pool->task_queue, task);
    }

    else if(strcmp(cmd_head, "gets") == 0) {
        char* file_name = strtok(NULL, "\n");
        task_t task = {user->connfd, SEND, user->pwd};
        strcpy(task.file_name, file_name);
        user->recv_status = RECV_FILE;
        epoll_del(epfd, user->connfd);
        blockq_push(pool->task_queue, task);
    }

    else if(strcmp(cmd_head, "rm") == 0) {
        char* file_name = strtok(NULL, "\n");
        int parent_id = mysql_pwd_to_id(user);
        char update[STR_MAXLEN];
        sprintf(update, "UPDATE files SET tomb = '1' \
                        WHERE parent_id = '%d' AND filename = '%s'", 
                        parent_id, file_name);
        mysql_disk_update(update);
    }

    else if(strcmp(cmd_head, "mkdir") == 0) {
        char* dir_name = strtok(NULL, "\n");
        int parent_id = mysql_pwd_to_id(user);
        char insert[STR_MAXLEN];
        sprintf(insert, "INSERT INTO files (parent_id,filename,owner_id,md5,filesize,type) \
                                         VALUES('%d','%s','%d','0','0','d')", 
                                        parent_id, dir_name, user->id);
        mysql_disk_insert(insert);
    }

}