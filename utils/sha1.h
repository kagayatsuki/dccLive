/*
 * Copyright (c) shinsya.G 
 */

#ifndef DCCLIVE_SHA1_H
#define DCCLIVE_SHA1_H

#include <unistd.h>
#include <stdint.h>

typedef struct {
    uint32_t state[5];
    uint64_t count;
    uint8_t buffer[64];
} hash_sha1_ctx;

void sha1_init(hash_sha1_ctx *ctx);
void sha1_update(hash_sha1_ctx *ctx, void *data, size_t len);
void sha1_final(hash_sha1_ctx *ctx, uint8_t digest[20]);
void sha1_string(const uint8_t digest[20], char buf[40], int8_t upper_case);

#endif //DCCLIVE_SHA1_H
