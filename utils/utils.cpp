/*
 * Copyright (c) shinsya.G 
 */
#include "./string.h"
#include "socket.h"

const char hexAlpha[] = "0123456789abcdef";

/** 以下是数转串操作 */
// 有符号整数(64bit)化字符串
unsigned int int2string(long long d, char *buf, unsigned int maxLen) {
    if (d >= 0)
        return uint2string(d, buf, maxLen);

    if (maxLen == 0)
        return 0;

    buf[0] = '-';
    return uint2string(-d, buf + 1, maxLen - 1) + 1;
}

// 无符号整数(64bit)化字符串
unsigned int uint2string(unsigned long long d, char *buf, unsigned int maxLen) {
    char stack[20];
    int pop = 0, push = 0;

    if (maxLen == 0)
        return 0;

    if (d == 0) {
        buf[0] = '0';
        return 1;
    }

    for (; d; d /= 10, push++) {
        stack[push] = (char)(d % 10);
    }

    for (; pop < push && pop < maxLen; pop++)
        buf[pop] = (char)('0' + stack[push - pop - 1]);
    return pop;
}

// 无符号整数(32bit)化16进制字符串
unsigned int i32toHex(unsigned int d, char *buf, unsigned int maxLen) {
    int i = 0;
    for (; i < 8 && i < maxLen; i++) {
        buf[i] = hexAlpha[(d >> (28 - i * 4)) & 0x0f];
    }
    return i;
}

// 无符号整数(64bit)化16进制字符串
unsigned int i64toHex(unsigned long long d, char *buf, unsigned int maxLen) {
    int i = 0;
    for (; i < 16 && i < maxLen; i++) {
        buf[i] = hexAlpha[(d >> (60 - i * 4)) & 0x0f];
    }
    return i;
}

size_t str_copy(char *dest, const char *src, size_t dest_max) {
    if (!dest || !dest_max)
        return 0;
    size_t len = 0;
    for (; src[len] && len < (dest_max - 1); len++)
        dest[len] = src[len];
    dest[len] = 0;
    return len;
}


/** 以下是socket操作 */
// 将指定文件描述符设置为非阻塞
void setNonBlock(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

// 创建一个非阻塞socket
int createNonBlockSocket(int domain, int type, int proto) {
    int fd = socket(domain, type, proto);
    if (fd == -1)
        return -1;
    setNonBlock(fd);
    return fd;
}