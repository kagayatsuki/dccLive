/*
 * Copyright (c) shinsya.G 
 */
#include "log/env.h"

#include <csignal>
#include <cstdio>

int lifecycle_flag = 0;

int try_get_pid() {
    FILE *f = fopen(PID_FILE_PATH, "rb");
    if (!f)
        return 0;
    int pid = 0;
    fscanf(f, "%d", &pid);
    fclose(f);
    if (send_signal(pid, 0))
        pid = 0;
    return pid;
}

int try_set_pid() {
    // 当前进程pid
    auto pid = getpid();
    FILE *f = fopen(PID_FILE_PATH, "w");
    if (!f)
        return -1;
    if (fprintf(f, "%d", pid) < 0) {
        fclose(f);
        return -1;
    }
    fclose(f);
    return 0;
}