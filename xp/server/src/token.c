#include "../inc/token.h"

char* secret_key = "password";

void encode_token(char** jwt, int id) {
    size_t jwt_length; // Token 的长度

    struct l8w8jwt_encoding_params params;
    l8w8jwt_encoding_params_init(&params);

    params.alg = L8W8JWT_ALG_HS512;

    params.iat = l8w8jwt_time(NULL);
    params.exp = l8w8jwt_time(NULL) + 60 * 60 * 24; /* Set to expire after 10 minutes (600 seconds). */

    params.secret_key = (unsigned char*)secret_key;
    params.secret_key_length = strlen(secret_key);

    struct l8w8jwt_claim claims;
    char id_str[20];
    sprintf(id_str, "%d", id);
    claims.key = "id";
    claims.key_length = 2;
    claims.value = id_str;
    claims.value_length = strlen(id_str);
    claims.type = 0;
    params.additional_payload_claims = &claims;
    params.additional_payload_claims_count = 1;

    params.out = jwt;
    params.out_length = &jwt_length;

    l8w8jwt_encode(&params);

    /* Always free the output jwt string! */
    //l8w8jwt_free(*jwt);
}

int decode_token(const char* jwt, int* id) {
    struct l8w8jwt_decoding_params params;
    l8w8jwt_decoding_params_init(&params);

    params.alg = L8W8JWT_ALG_HS512;

    params.jwt = (char*)jwt;
    params.jwt_length = strlen(jwt);

    params.verification_key = (unsigned char*)secret_key;
    params.verification_key_length = strlen(secret_key);

    /* Expiration validation set to false here only because the above example token is already expired! */
    params.validate_exp = 0;
    params.exp_tolerance_seconds = 60;

    params.validate_iat = 1;
    params.iat_tolerance_seconds = 60;

    enum l8w8jwt_validation_result validation_result;
    struct l8w8jwt_claim *claims = NULL;
    size_t claim_count;
    int decode_result = l8w8jwt_decode(&params, &validation_result, &claims, &claim_count);
    
    if (decode_result == L8W8JWT_SUCCESS && validation_result == L8W8JWT_VALID) {
        for (size_t i = 0; i < claim_count; i++) {
            if (strcmp((char*)claims[i].key, "id") == 0) {
                *id = atoi(claims[i].value);
            }
        }
        return 0;
    } else {
        return -1;
    }
}