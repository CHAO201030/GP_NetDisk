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

void do_gets_task(int client_fd, char *cmd)
{
    // 打开文件
    char file_path[300] = {0};
    sprintf(file_path, "../repository/%s", cmd);
    printf("%s\n", file_path);
    int fd = open(file_path, O_RDWR);
    if(fd == -1)
    {
        error(-1, errno, "open %s failed", file_path);
    }
    // 接收文件偏移信息
    // TODO : 大小文件判断
    int offset = 0;
    int part_size = 0;
    int big_file_boundary = (BIG_FILE_SIZE >> 1) >> 12 << 12;
    
    recvn(client_fd, &offset, sizeof(offset));
    recvn(client_fd, &part_size, sizeof(part_size));
    printf("offset = %d, part_size = %d\n", offset, part_size);

    bool is_big_file = part_size < big_file_boundary ? false : true;

    // 发送文件
    if(is_big_file)
    {
        send_big_file(client_fd, fd, offset, part_size);
    }
    else
    {
        int cur_size = 0;
        int send_size = 0;
        char file_buf[BUFFER_SIZE] = {0};

        while(send_size < part_size)
        {
            if(part_size - send_size < BUFFER_SIZE)
            {
                cur_size = part_size - send_size;
            }
            else
            {
                cur_size = BUFFER_SIZE;
            }
            bzero(file_buf, BUFFER_SIZE);

            read(fd, file_buf, cur_size);
            sendn(client_fd, file_buf, cur_size);

            send_size += cur_size;
        }
    }
    close(fd);
    close(client_fd);
    printf("[INFO] : send file %s success...\n", cmd);
}

void do_puts_task(int client_fd, char *cmd)
{
    char file_path[300] = {0};
    sprintf(file_path, "../repository/%s", cmd);
    printf("%s\n", file_path);
    int fd = open(file_path, O_RDWR|O_CREAT, 0666);
    if(fd == -1)
    {
        error(-1, errno, "open %s failed", file_path);
    }

    bool is_big_file = false;
    recvn(client_fd, &is_big_file, sizeof(is_big_file));
    off_t file_size = 0;
    recvn(client_fd, &file_size, sizeof(file_size));

    ftruncate(fd, file_size);
    
printf("%ld\n", file_size);
    if(is_big_file)
    {
        // 接收大文件
        int recv_size = 0;
        int cur_size  = 0;

        while(recv_size < file_size)
        {
            if(file_size - recv_size < MMAP_SIZE)
            {
                cur_size = file_size - recv_size;
            }
            else
            {
                cur_size = MMAP_SIZE;
            }

            void *mm_addr = mmap(NULL, cur_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, recv_size);
        
            recvn(client_fd, mm_addr, cur_size);

            munmap(mm_addr, cur_size);

            recv_size += cur_size;
        }
    }
    else
    {
        printf("file_size = %ld\n", file_size);
        // 接收小文件
        int recv_size = 0;
        int cur_size  = 0;
        char buf[BUFFER_SIZE] = {0};

        while(recv_size < file_size)
        {
            if(file_size - recv_size < BUFFER_SIZE)
            {
                cur_size = file_size - recv_size;
            }
            else
            {
                cur_size = BUFFER_SIZE;
            }

            bzero(buf, sizeof(buf));
            recvn(client_fd, buf, cur_size);
            write(fd, buf, cur_size);
            
            recv_size += file_size;
        }
    }
}
