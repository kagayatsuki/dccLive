/*
 * Copyright (c) shinsya.G 
 */

#ifndef DCCLIVE_ENV_H
#define DCCLIVE_ENV_H

#include "type.h"

#define PID_FILE_PATH           "/var/run/toplive.pid"

#define LOG_FILE_LIVE_CLI       "./log/live_client"
#define LOG_FILE_LIVE_SVR       "./log/live_server"
#define LOG_FILE_ADMIN          "./log/admin"
#define LOG_FILE_SYSTEM         "./log/sys"

// 初始化日志环境
void log_init(bool with_std);

// 写入日志内容
log_size log_write(int type, char *fmt, ...);

// 关闭日志系统
void log_shutdown();

#define LOG2SYS(fmt, ...) log_write(LOG_SYS, (char *)fmt, ##__VA_ARGS__)
#define LOG2CLI(fmt, ...) log_write(LOG_LIVE_CLI, (char *)fmt, ##__VA_ARGS__)
#define LOG2SVR(fmt, ...) log_write(LOG_LIVE_SVR, (char *)fmt, ##__VA_ARGS__)
#define LOG2ADM(fmt, ...) log_write(LOG_ADMIN, (char *)fmt, ##__VA_ARGS__)

// 尝试获取已存在的进程pid
int try_get_pid();
// 重新设置pid记录
int try_set_pid();

// 进程交互信号
#define SIG_EXIT            SIGUSR1
#define SIG_RELOAD          SIGUSR2
#define send_signal(a, b)   kill(a, b)

// lifecycle 标志位, 0-continue, 1-reload, 2-exit
extern int lifecycle_flag;

#endif //DCCLIVE_ENV_H
