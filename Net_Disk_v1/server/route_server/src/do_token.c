#include "../include/route_task.h"
#include "../include/client_manager.h"

extern HashMap *client_manage_map;

int decode_token(const char* token, int *client_main_thread_fd)
{
    struct l8w8jwt_decoding_params params;
    l8w8jwt_decoding_params_init(&params);

    params.alg = L8W8JWT_ALG_HS512;

    params.jwt = (char*)token;
    params.jwt_length = strlen(token);

    params.verification_key = (unsigned char*)ROUTE_TOKEN_KEY;
    params.verification_key_length = strlen(ROUTE_TOKEN_KEY);

    params.validate_sub = "Net Disk Token";
    params.validate_iss = "Administrator"; 

    params.validate_exp = 1; // 设置为 1 代表会检查是否失效 为 0 则代表永久有效
    params.exp_tolerance_seconds = 60;

    params.validate_iat = 1;
    params.iat_tolerance_seconds = 60;
 
    enum l8w8jwt_validation_result validation_result;

    struct l8w8jwt_claim *out_claim = NULL;
    size_t out_claim_len = 0;
    
    int decode_result = l8w8jwt_decode(&params, &validation_result, &out_claim, &out_claim_len);

    if (decode_result == L8W8JWT_SUCCESS && validation_result == L8W8JWT_VALID) 
    {
        for(int i = 0; i < out_claim_len; i++)
        {
            if(strcmp(out_claim[i].key, "client_fd") == 0)
            {
                *client_main_thread_fd = atoi(out_claim[i].value);
            }
        }

        l8w8jwt_free_claims(out_claim, out_claim_len);

        return 0;
    }
    else
    {
        return -1;
    }
}

int do_token(client_t *client, char *token)
{
    int client_main_thread_fd = -1;
    bool is_valid = false;

    if(decode_token(token, &client_main_thread_fd) == 0)
    {
        // TOKEN 验证成功
        is_valid = true;
        sendn(client->fd, &is_valid, sizeof(is_valid));

        client_t *client_main_thread =  get_cur_client(client_manage_map, client_main_thread_fd);

        client->code = client_main_thread->code;
        client->pre_code = client_main_thread->pre_code;
        client->uid = client_main_thread->uid;

        return 0;
    }
    else
    {
        // TOKEN 验证失败
        sendn(client->fd, &is_valid, sizeof(is_valid));
        
        return -1;
    }
}
