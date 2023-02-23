#include <cstring>
#include "log/env.h"
#include "service/service.h"

#include <csignal>
#include <cstdio>

const char *help_str = "dccLive Version Canary. Help information:";
const char *sig_help = "\tValid signal parameter: [reload|exit]";

enum launch_type_e {
    NORMAL,         // 正常启动模式
    HELP,           // 帮助模式，不启动各单元，打印帮助信息
    SIGNAL,         // 信号模式，不启动各单元，仅根据启动参数向已启动的进程发送信号
    DEBUG           // non daemon 模式，在前台运行，且日志将在终端打印
} launch_type = NORMAL;

int params_config(int argc, char **argv);
int start_session();
void signal_handler(int sig);

void init_log(bool with_std) {
    log_init(with_std);
    LOG2SYS("Log unit initialized. Magic 0x%x", LOG_MAGIC);
}

void start_lifecycle() {
    signal(SIG_EXIT, signal_handler);
    signal(SIG_RELOAD, signal_handler);
    while (true) {
        // Exit
        if (lifecycle_flag == 2) {
            LOG2SYS("EXIT signal received");
            break;
        }
        // Reload
        if (lifecycle_flag == 1) {
            // TODO: 立刻保存环境，重新载入配置文件
        }
        setNextAction(process_core, nullptr);
        service_once_cycle();
    }
}

int main(int argc, char **argv) {
    int tmp = params_config(argc, argv);
    if (launch_type == HELP) {
        puts(help_str);
        return 0;
    }
    else if (launch_type == SIGNAL) {
        if (tmp == 0) {
            puts("Unknown signal parameter after '-s'\n");
            return -1;
        }
        int pid = try_get_pid();
        if (!pid) {
            puts("dccLive server has not running. Cannot send signal\n");
            return -1;
        }
        send_signal(pid, tmp);
        return 0;
    }

    if (start_session()) {
        puts("Abort\n");
        return -1;
    }

    init_log(launch_type == DEBUG);
    if (service_init()) {
        puts("Abort\n");
        return -1;
    }

    start_lifecycle();
    service_shutdown();

    log_shutdown();

    return 0;
}

// 启动会话，设定pid记录，设定通信中断信号
int start_session() {
    // 检查是否已存在进程
    if (try_get_pid()) {
        fputs("One instance has already running or pid file " PID_FILE_PATH " was used\n", stderr);
        // 已有实例存在，退出当前进程
        return -1;
    }
    // 写当前进程pid
    if (try_set_pid()) {
        fputs("Cannot update pid file " PID_FILE_PATH, stderr);
        return -1;
    }
    return 0;
}

int param_signal_cmp(char *sig) {
    if (sig == nullptr)
        return 0;
    if (!strcmp("reload", sig))
        return SIG_RELOAD;
    else if (!strcmp("exit", sig))
        return SIG_EXIT;
    return 0;
}

// 解析启动参数，设定运行模式
int params_config(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        char *cur = argv[i];
        if (!strcmp("-d", cur) || !strcmp("--debug", cur))
            launch_type = DEBUG;
        else if (!strcmp("-h", cur) || !strcmp("--help", cur))
            launch_type = HELP;
        else if (!strcmp("-s", cur) || !strcmp("--sig", cur)) {
            launch_type = SIGNAL;
            if (i + 1 < argc) {
                return param_signal_cmp(argv[i + 1]);
            } else {
                fputs("The parameter '-s','--sig' should provide a extra signal param.", stderr);
                fputs(sig_help, stderr);
            }
            // signal模式不接收其它参数
            break;
        }
    }
    return 0;
}

// 信号处理
void signal_handler(int sig) {
    if (SIG_EXIT == sig) {
        lifecycle_flag = 2;
    } else if (SIG_RELOAD == sig) {
        lifecycle_flag = 1;
    }
    signal(sig, signal_handler);
}