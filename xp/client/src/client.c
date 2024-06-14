#include <func.h>
#include "../inc/Bussiness.h"
#include "../inc/Network.h"
#include "../inc/ThreadPool.h"

#define SERVPORT "12306"
#define SERVIP "127.0.0.1"
#define MAXLINE 1024

typedef enum {
    RECV_RESPONSE,
    RECV_FILE,
    RECV_MD5_COMMENT
} recv_status_e;


recv_status_e recv_status = RECV_RESPONSE;
bool online = false;

void cmd_handler(char* cmd, int sockfd, ThreadPool* pool);
void send_token(char* cmd, task_t* task, ThreadPool* pool);

int main(int argc, char *argv[])
{
    int sockfd = tcp_connect(SERVIP, SERVPORT);
    printf("sockfd = %d\n", sockfd);

    int num_threads = 8;
    ThreadPool* pool = create_thread_pool(num_threads);

    int epfd = epoll_create(1);
    epoll_add(epfd, STDIN_FILENO);
    epoll_add(epfd, sockfd);
    struct epoll_event ev[2] = {0};

    
    bool token_write = false;

    for(;;) {
        char cmd[MAXLINE] = {0};
        char response[MAXLINE] = {0};
        
        int n = epoll_wait(epfd, ev, 2, -1);
        for (int i = 0; i < n; i++) {
            // 接收用户输入命令
            if (ev[i].data.fd == STDIN_FILENO) {
                read(STDIN_FILENO, cmd, MAXLINE);
                // 发送命令到服务器
                char cmd2[MAXLINE];
                strcpy(cmd2, cmd);
                cmd_handler(cmd, sockfd, pool);
                if(recv_status == RECV_FILE) {
                    recv_status = RECV_RESPONSE;
                    continue;
                } else {
                    send_string(sockfd, cmd2);
                }
                
                
            } else if(ev[i].data.fd == sockfd) {
                // 接收服务器执行的结果
                if(online == false) {
                    recv_string(sockfd, response);
                    printf("%s\n", response);
                    if(strstr(response, "success") != NULL) {

                        token_write = true;
                        online = true;
                    }
                    fflush(stdout);
                } else if(token_write == true) {
                    char token[MAXLINE];
                    recv_string(sockfd, token);
                    int fd = open("token", O_RDWR | O_CREAT, 0666);
                    write(fd, token, strlen(token));
                    close(fd);
                    token_write = false;
                } else {
                    recv_string(sockfd, response);
                    printf("%s\n", response);
                    fflush(stdout);
                }
            }
        }
    }

    close(sockfd);

    return 0;
}

void cmd_handler(char* cmd, int sockfd, ThreadPool* pool) {
    char cmd2[MAXLINE];
    strcpy(cmd2, cmd);
    char* cmd_head = strtok(cmd, " ");
    task_t task;

    if(strcmp(cmd_head, "puts") == 0) {
        recv_status = RECV_FILE;
        task.type = SEND;
    }

    else if(strcmp(cmd_head, "gets") == 0) {
        recv_status = RECV_FILE;
        task.type = RECV;
    }

    if(recv_status == RECV_FILE) {
        char* file_name = strtok(NULL, "\n");
        strcpy(task.file_name, file_name);
        send_token(cmd2, &task, pool);
    } else if(online == false) {
        int token_len = 0;
        sendn(sockfd, &token_len, sizeof(token_len));
    }
}

void send_token(char* cmd, task_t* task, ThreadPool* pool) {
    int sockfd2 = tcp_connect(SERVIP, SERVPORT);
    int fd = open("token", O_RDONLY);
    char token[MAXLINE];
    read(fd, token, MAXLINE);
    send_string(sockfd2, token);
    close(fd);
    send_string(sockfd2, cmd);
    task->connfd = sockfd2;
    blockq_push(pool->task_queue, *task);
}

