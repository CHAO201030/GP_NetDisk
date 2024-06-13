#ifndef __BLOCK_QUEUE_H__
#define __BLOCK_QUEUE_H__

#include "head.h"

block_queue_t* block_queue_create(void);

client_t block_queue_pop(block_queue_t* q);

client_t block_queue_peek(block_queue_t* q);

void block_queue_push(block_queue_t* q, client_t val);

void block_queue_destroy(block_queue_t* q);

bool block_queue_is_empty(block_queue_t* q);

bool block_queue_is_full(block_queue_t* q);


#endif