/*
 * Copyright (c) shinsya.G 
 */
#include "../service/pool.h"
#include "../utils/socket.h"
#include "../service/service.h"
#include "env.h"

#include <netinet/in.h>

Pool *pool;
int exit_flag = 0;
int recv_count = 0;

int recv_handler(PoolEvent *event) {
    char buf[64] = {};
    ssize_t len;
    // 确保数据读完，同时配合epoll ET模式特性（为了高性能），避免数据被截断
    while ((len = read(event->fd, buf, 63)) > 0) {
        buf[len] = 0;
        LOG2SVR("Recv msg: %s", buf);
    }
    if (len <= 0 && errno != EAGAIN) {
        LOG2SVR("Client disconnected");
        return 1;
    }
    recv_count++;
    if (recv_count > 4)
        exit_flag = 1;
    return 0;
}

int accept_handler(PoolEvent *event) {
    LOG2SVR("New client connection in");
    sockaddr_in cli_addr{};
    socklen_t socklen;
    int cli = accept(event->fd, (sockaddr *)&cli_addr, &socklen);
    if (cli == -1) {
        LOG2SVR("Accepting failure");
        return -1;
    }
    setNonBlock(cli);
    pool->Reg(cli, recv_handler, nullptr);
    return 0;
}

void pool_test() {
    pool = new Pool;
    int fd = createNonBlockSocket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in addr{};
    addr.sin_port = htons(CONSOLE_PORT);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(fd, (sockaddr *)&addr, sizeof(addr)) == -1) {
        LOG2SVR("Cannot bind socket to 0.0.0.0:%d", (int)CONSOLE_PORT);
        return;
    }

    listen(fd, CONSOLE_BACKLOG);
    LOG2SVR("service started");
    pool->Reg(fd, accept_handler, nullptr);
    while (!exit_flag)
        pool->CheckOut();
    LOG2SVR("pool test passed in %d times", recv_count);
    delete pool;
}