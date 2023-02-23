/*
 * Copyright (c) shinsya.G 
 */

#ifndef DCCLIVE_STRING_H
#define DCCLIVE_STRING_H

#include <cstddef>

// 有符号整数转换字符串
unsigned int int2string(long long d, char *buf, unsigned int maxLen);

// 无符号整数转换字符串
unsigned int uint2string(unsigned long long d, char *buf, unsigned int maxLen);

// 32位整形转写16进制字符串
unsigned int i32toHex(unsigned int d, char *buf, unsigned int maxLen);

// 64位整形转写16进制字符串
unsigned int i64toHex(unsigned long long d, char *buf, unsigned int maxLen);

size_t str_copy(char *dest, const char *src, size_t dest_max);
#endif //DCCLIVE_STRING_H
