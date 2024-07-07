#include "../include/task.h"

extern char *TOKEN;

bool verify_token(int route_new_fd)
{
    // 发送 TOKEN 信息
    train_t token_train = {0};
    token_train.data_len = strlen(TOKEN);
    token_train.state = CMD_TOKEN;
    strncpy(token_train.data_buf, TOKEN, token_train.data_len);
    
    send_cmd(&token_train, CMD_TOKEN, route_new_fd);

    bool is_valid = false;
    recvn(route_new_fd, &is_valid, sizeof(is_valid));
    
    return is_valid;
}
