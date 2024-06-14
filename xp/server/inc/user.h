#ifndef __WD_USER_H
#define __WD_USER_H 

#define MAXLINE 1024
#define STR_MAXLEN 256

typedef enum {
    RECV_CMD,
    RECV_FILE
} recv_status_e;

typedef struct
{
    char pwd[256];
    int connfd;
    bool cmd_analysis;
    char name[256];
    bool online;
    int id;
    recv_status_e recv_status;
}User_t;





#endif