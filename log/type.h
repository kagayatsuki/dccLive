/*
 * Copyright (c) shinsya.G 
 */

#ifndef DCCLIVE_TYPE_H
#define DCCLIVE_TYPE_H

#include <unistd.h>

typedef unsigned int    log_size;
typedef char *          log_buffer;
typedef int             log_file;

typedef struct log_object {
    log_file _file;
    log_buffer _buf;
} log_object;

#define LOG_DEFAULT_BUFFER_SIZE     (4096 * 1024)

#define LOG_LIVE_CLI                0
#define LOG_LIVE_SVR                1
#define LOG_ADMIN                   2
#define LOG_SYS                     3

#define LOG_MAGIC                   0x12DC90BD

#endif //DCCLIVE_TYPE_H
