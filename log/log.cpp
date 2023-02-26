/*
 * Copyright (c) shinsya.G 
 */

#include "env.h"
#include "../utils/string.h"

#include <cstdarg>
#include <fcntl.h>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <ctime>
#include <cstring>
#include <dirent.h>
#include <sys/stat.h>

log_object *log_objs[4] = {};
const char *log_filename[] = {
        LOG_FILE_LIVE_CLI,
        LOG_FILE_LIVE_SVR,
        LOG_FILE_ADMIN,
        LOG_FILE_SYSTEM
};

bool std_out = false;

void log_init(bool with_std) {
    std_out = with_std;
    // log 目录检查
    DIR *dir = opendir("./log");
    if (dir)
        closedir(dir);
    else {
        if (mkdir("./log", S_IRWXU | S_IRGRP)) {
            fprintf(stderr, "Can not create directory for log, code: %d\n", errno);
            exit(-1);
        }
    }

    for (int i = 0; i < 4; i++) {
        log_objs[i] = new log_object;
        log_objs[i]->_buf = new char[LOG_DEFAULT_BUFFER_SIZE + 1];  // 长度+1是为保证日志内容即使被截断也要保证换行符 '\n' 能正常写入
        log_objs[i]->_file = open(log_filename[i],
                                  O_APPEND | O_CREAT | O_WRONLY,
                                  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        // 无法打开日志文件
        if (-1 == log_objs[i]->_file) {
            fprintf(stderr, "Can not open log file '%s' code %d\n", log_filename[i], errno);
            exit(-1);
        }
    }
    LOG2SYS("Launching with debug mode");
}

#define BUF_CAPACITY (LOG_DEFAULT_BUFFER_SIZE - size)
log_size log_write(int type, char *fmt, ...) {
    // 有效类型检查
    switch (type) {
        case LOG_ADMIN:
        case LOG_LIVE_CLI:
        case LOG_LIVE_SVR:
        case LOG_SYS:
            break;
        default:
            return 0;
    }
    log_file file = log_objs[type]->_file;
    // 日志对象有效性检查
    if (-1 == file)
        return 0;

    log_buffer buf = log_objs[type]->_buf;
    log_size size = 0;
    // 多线程安全
    memset(buf, 0, LOG_DEFAULT_BUFFER_SIZE);

    // 多参环境
    va_list args;
    va_start(args, fmt);

    // 时间戳
    time_t now = time(nullptr);
    struct tm *now_tm = localtime(&now);
    size += sprintf(buf, "[%d/%d/%d %02d:%02d:%02d] ",
                    now_tm->tm_year + 1900,
                    now_tm->tm_mon + 1,
                    now_tm->tm_mday,
                    now_tm->tm_hour,
                    now_tm->tm_min,
                    now_tm->tm_sec);

    // 格式解析
    unsigned int offset = 1;        // fmt offset
    char ch = fmt[0];

    for (; size < LOG_DEFAULT_BUFFER_SIZE && ch != 0; ch = fmt[offset++]) {
        // 读入单字符
        if (ch != '%') {
            buf[size++] = ch;
            continue;
        }
        ch = fmt[offset++];
        // 格式化参数
        switch (ch) {
            case '%': {         // '%' 百分号
                buf[size++] = '%';
                break;
            }
            case 'd': {         // 有符号整数(32bit)
                size += int2string(va_arg(args, int), buf + size, BUF_CAPACITY);
                break;
            }
            case 'u': {         // 无符号整数
                ch = fmt[offset + 1];
                unsigned long long tmp = 0;
                if (ch == 'l') {    // 无符号整数(64bit)
                    tmp = va_arg(args, unsigned long long);
                    offset++;
                }
                else                // 32bit
                    tmp = va_arg(args, unsigned int);
                size += uint2string(tmp, buf + size, BUF_CAPACITY);
                break;
            }
            case 'l': {         // 有符号整数(64bit)
                size += int2string(va_arg(args, long long), buf + size, BUF_CAPACITY);
                break;
            }
            case 'x': {         // 32位整型16进制
                size += i32toHex(va_arg(args, unsigned int), buf + size, BUF_CAPACITY);
                break;
            }
            case 'p': {         // 64位整型16进制
                size += i64toHex(va_arg(args, unsigned long long), buf + size, BUF_CAPACITY);
                break;
            }
            case 's': {         // '\0'结尾utf8字符串
                char *str = va_arg(args, char *);
                if (str == nullptr)
                    continue;
                for (; *str && size < LOG_DEFAULT_BUFFER_SIZE; size++, str++)
                    buf[size] = *str;
                break;
            }
            default: {
                buf[size++] = '%';
                if (size < LOG_DEFAULT_BUFFER_SIZE)
                    buf[size++] = ch;
                break;
            }
        }
    }
    va_end(args);

    buf[size++] = '\n';
    write(file, buf, size);
    if (std_out)
        fputs(buf, stdout);
    // 返回写入日志字节数
    return size;
}
#undef BUF_CAPACITY

void log_shutdown() {
    for (auto &log_obj: log_objs) {
        if (log_obj->_file != -1) {
            close(log_obj->_file);
            log_obj->_file = -1;
        }
    }
}
