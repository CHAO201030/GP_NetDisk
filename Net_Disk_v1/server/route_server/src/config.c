#include "../include/config.h"

void read_config(const char *conf_file, config_table_t *conf_table)
{
    FILE *conf_fp = fopen(conf_file, "rw");
    if(conf_fp == NULL)
    {
        printf("[INFO] : Open config file %s failed\n", conf_file);
        return;
    }

    char buf[128] = {0};
    while(fgets(buf, sizeof(buf), conf_fp) != NULL)
    {
        int buf_len = strlen(buf);
        buf[--buf_len] = '\0';  // \n to \0
        
        char *key = strtok(buf, " = ");
        char *value = strtok(NULL, " = ");

        config_table_insert(key, value, conf_table);
    }

    fclose(conf_fp);

    return;
}

config_table_t* config_table_create()
{
    config_table_t *t = calloc(1, sizeof(config_table_t));
    if(t == NULL)
    {
        error(-1, errno, "Config table init failed\nReason");
    }
    
    return t;
}

unsigned int cfg_hash(const char *key)
{
    unsigned int hashVal = 0;  
    while (*key != '\0')
    {  
        hashVal = (hashVal << 5) + hashVal + *key++;  
    }

    return hashVal % CONFIG_TB_SIZE;      
}

void config_table_insert(const char *key, const char *value, config_table_t *conf_table)
{
    unsigned int pos = cfg_hash(key);

    while(conf_table->table[pos].value[0] != 0)
    {
        pos = (pos + 1) % CONFIG_TB_SIZE;
        if(strcmp(conf_table->table[pos].key, key) == 0)
        {
            strncpy(conf_table->table[pos].value, value, strlen(value));
            return;
        }
    }

    strncpy(conf_table->table[pos].key, key, strlen(key));
    strncpy(conf_table->table[pos].value, value, strlen(value));
    conf_table->size++;

    return ;
}

const char* config_table_find(const config_table_t *conf_table, const char *key)
{
    unsigned int pos = cfg_hash(key);
    while(conf_table->table[pos].key[0] != 0)
    {
        if(strcmp(conf_table->table[pos].key, key) == 0)
        {
            return conf_table->table[pos].value;
        }
        pos = (pos + 1) % CONFIG_TB_SIZE;
    }
    
    return NULL;
}

void config_table_destroy(config_table_t *conf_table)
{
    free(conf_table);
}
