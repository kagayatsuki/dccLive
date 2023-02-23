/*
 * Copyright (c) shinsya.G 
 */

#include "sha1.h"
#include <string.h>

#define Rotate(bits, word) (((word) << (bits)) | ((word) >> (32 - (bits))))
#define F1(b, c, d) (((b) & (c)) | ((~(b)) & (d)))
#define F2(b, c, d)  ((b) ^ (c) ^ (d))
#define F3(b, c, d)  (((b) & (c)) | ((b) & (d)) | ((c) & (d)))

#define Step(f, a, b, c, d, e, w, t)                                          \
    temp = Rotate(5, (a)) + f((b), (c), (d)) + (e) + (w) + (t);               \
    (e) = (d);                                                                \
    (d) = (c);                                                                \
    (c) = Rotate(30, (b));                                                    \
    (b) = (a);                                                                \
    (a) = temp;

#define GET(n)                                                                \
    ((uint32_t) p[n * 4 + 3] |                                                \
    ((uint32_t) p[n * 4 + 2] << 8) |                                          \
    ((uint32_t) p[n * 4 + 1] << 16) |                                         \
    ((uint32_t) p[n * 4] << 24))



uint8_t *sha1_append(hash_sha1_ctx *ctx, uint8_t *data, size_t len) {
    uint32_t a, b, c, d, e, temp;
    uint32_t la, lb, lc, ld, le;
    uint32_t words[80];
    uintptr_t i, j;
    uint8_t *p;

    p = data;
    a = ctx->state[0];
    b = ctx->state[1];
    c = ctx->state[2];
    d = ctx->state[3];
    e = ctx->state[4];

    do {
        la = a;
        lb = b;
        lc = c;
        ld = d;
        le = e;

        for (i = 0; i < 16; i++)
            words[i] = GET(i);

        for (i = 16; i < 80; i++) {
            words[i] = Rotate(1, words[i - 3] ^ words[i - 8]
                                 ^ words[i - 14] ^ words[i - 16]);
        }

        for (j = 0; j < 20; j++) {
            Step(F1, a, b, c, d, e, words[j], 0x5a827999)
        }
        for (; j < 40; j++) {
            Step(F2, a, b, c, d, e, words[j], 0x6ed9eba1)
        }
        for (; j < 60; j++) {
            Step(F3, a, b, c, d, e, words[j], 0x8f1bbcdc)
        }
        for (; j < 80; j++) {
            Step(F2, a, b, c, d, e, words[j], 0xca62c1d6)
        }

        a += la;
        b += lb;
        c += lc;
        d += ld;
        e += le;

        p += 64;
    } while (len -= 64);

    ctx->state[0] = a;
    ctx->state[1] = b;
    ctx->state[2] = c;
    ctx->state[3] = d;
    ctx->state[4] = e;
    return p;
}

void sha1_init(hash_sha1_ctx *ctx) {
    ctx->state[0] = 0x67452301;
    ctx->state[1] = 0xefcdab89;
    ctx->state[2] = 0x98badcfe;
    ctx->state[3] = 0x10325476;
    ctx->state[4] = 0xc3d2e1f0;
    ctx->count = 0;
}

void sha1_update(hash_sha1_ctx *ctx, void *data, size_t len) {
    size_t used_c, free_c;
    used_c = (size_t)(ctx->count & 0x3f);
    ctx->count += len;
    if (used_c) {
        free_c = 64 - used_c;
        if (len < free_c) {
            memcpy(&ctx->buffer[used_c], data, len);
            return;
        }
        memcpy(&ctx->buffer[used_c], data, free_c);
        data = (uint8_t *)data + free_c;
        len -= free_c;
        sha1_append(ctx, ctx->buffer, 64);
    }
    if (len >= 64) {
        data = sha1_append(ctx, (uint8_t *)data, len & ~(size_t)0x3f);
        len &= 0x3f;
    }
    memcpy(ctx->buffer, data, len);
}

void sha1_final(hash_sha1_ctx *ctx, uint8_t result[20]) {
    size_t used_c, free_c;
    used_c = (size_t)(ctx->count & 0x3f);
    ctx->buffer[used_c++] = 0x80;
    free_c = 64 - used_c;
    if (free_c < 8) {
        memset(&ctx->buffer[used_c], 0, free_c);
        sha1_append(ctx, ctx->buffer, 64);
        used_c = 0;
        free_c = 64;
    }
    memset(&ctx->buffer[used_c], 0, free_c - 8);

    ctx->count <<= 3;
    for (int i = 0; i < 8; i++)
        ctx->buffer[56 + i] = ctx->count >> (56 - 8 * i);

    sha1_append(ctx, ctx->buffer, 64);

    for (int i = 0; i < 5; i++) {
        result[i * 4] = (uint8_t)(ctx->state[i] >> 24);
        result[i * 4 + 1] = (uint8_t)(ctx->state[i] >> 16);
        result[i * 4 + 2] = (uint8_t)(ctx->state[i] >> 8);
        result[i * 4 + 3] = (uint8_t)(ctx->state[i]);
    }

    memset(ctx, 0, sizeof(hash_sha1_ctx));
}

void sha1_string(const uint8_t digest[20], char buf[40], int8_t upper_case) {
    const char *alpha_bet[] = { "0123456789abcdef", "0123456789ABCDEF" };
    const char *set = alpha_bet[upper_case != 0];
    char *p = buf;
    for (int i = 0; i < 20; i++) {
        *p++ = set[digest[i] >> 4];
        *p++ = set[digest[i] & 0x0f];
    }
}