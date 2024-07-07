#include "../include/file_server.h"

int recv_train(int client_fd, train_t *cmd_train)
{
    // 接收命令长度
    int ret = recvn(client_fd, &cmd_train->data_len, sizeof(cmd_train->data_len));

    if(ret == 0)return 0;   // 对端断开处理
    
    // 接收命令类型
    recvn(client_fd, &cmd_train->state, sizeof(cmd_train->state));

    // 接收命令内容
    recvn(client_fd, cmd_train->data_buf, cmd_train->data_len);

    return 1;
}

void task_analyse(int new_client_fd)
{
    train_t cmd_train = {0};
    
    if(recv_train(new_client_fd, &cmd_train) == 0)return;

    switch (cmd_train.state)
    {
    case CMD_GETS:do_gets_task(new_client_fd, cmd_train.data_buf);break;
    case CMD_PUTS:do_puts_task(new_client_fd, cmd_train.data_buf);break;
    default:
        break;
    }
}

void do_gets_task(int client_fd, char *file_name)
{
    char file_path[300] = {0};
    sprintf(file_path, "../repository/%s", file_name);
    printf("gets %s\n", file_path);
    
    // 打开文件
    int fd = open(file_path, O_RDWR);
    if(fd == -1)
    {
        error(-1, errno, "open %s failed", file_path);
    }

    // 接收文件偏移信息
    int offset = 0;
    int part_size = 0;
    int big_file_boundary = (BIG_FILE_SIZE >> 1) >> 12 << 12;
    
    recvn(client_fd, &offset, sizeof(offset));
    recvn(client_fd, &part_size, sizeof(part_size));

    bool is_big_file = part_size < big_file_boundary ? false : true;

    // 发送文件
    if(is_big_file)
    {
        if((send_big_file(client_fd, fd, offset, part_size)) <= 0)
        {
            printf("[INFO] : Send %s failed\n", file_name);
            
            close(fd);
            close(client_fd);
            
            return;
        }
    }
    else
    {
        if((send_small_file(client_fd, fd, offset, part_size)) <= 0)
        {
            printf("[INFO] : Send %s failed\n", file_name);
            
            close(fd);
            close(client_fd);
            
            return;
        }    
    }

    close(fd);
    close(client_fd);
    printf("[INFO] : Send file %s success...\n", file_name);
}

void do_puts_task(int client_fd, char *file_name)
{
    char file_path[300] = {0};
    sprintf(file_path, "../repository/%s", file_name);
    printf("puts %s\n", file_path);
    
    // 打开文件
    int fd = open(file_path, O_RDWR|O_CREAT, 0666);
    if(fd == -1)
    {
        error(-1, errno, "open %s failed", file_path);
    }

    bool is_big_file = false;
    recvn(client_fd, &is_big_file, sizeof(is_big_file));

    // 接收文件大小
    off_t file_size = 0;
    recvn(client_fd, &file_size, sizeof(file_size));

    ftruncate(fd, file_size);

    if(is_big_file)
    {
        if((recv_big_file(client_fd, fd, file_size)) <= 0)
        {
            printf("[INFO] : Recv file %s error\n", file_name);
            
            close(fd);
            close(client_fd);
            
            return;
        }
    }
    else
    {
        if((recv_small_file(client_fd, fd, file_size)) <= 0)
        {
            printf("[INFO] : Recv file %s error\n", file_name);
            
            close(fd);
            close(client_fd);
            
            return;
        }
    }

    close(fd);
    close(client_fd);
    printf("[INFO] : Recv file %s success...\n", file_name);
}
