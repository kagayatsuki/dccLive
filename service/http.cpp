/*
 * Copyright (c) shinsya.G 
 */
#include "service.h"
#include "pool.h"
#include "env.h"
#include "../utils/socket.h"
#include "proto.h"
#include "router/router.h"

#include <netinet/in.h>
#include <cstring>

#define SERVICE_BUFFER_SIZE         8192


int service_request_process(PoolEvent *e);

char *service_buffer;

namespace http {
    Pool *connectionPool;
}

int process_pool(void *d) {
    http::connectionPool->CheckOut();
    return 0;
}

int process_service(void *d) {
    setNextAction(process_pool, nullptr);
    return 0;
}

int service_conn_process(PoolEvent *e) {
    sockaddr_in addr{};
    socklen_t len;
    while (true) {
        int cli = accept(e->fd, (sockaddr *)&addr, &len);
        if (cli > 0) {
            setNonBlock(cli);
            http::connectionPool->Reg(cli, service_request_process, nullptr);
            continue;
        }
        break;
    }
    return 0;
}

int init_service(Pool *pool) {
    http::connectionPool = pool;
    service_buffer = new char[SERVICE_BUFFER_SIZE];
    int fd = createNonBlockSocket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd == -1) {
        LOG2SVR("open service socket failed");
        return -1;
    }
    sockaddr_in addr{};
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(SERVICE_PORT);
    addr.sin_family = AF_INET;
    if (bind(fd, (sockaddr *)&addr, sizeof(addr)) != 0) {
        LOG2SVR("Cannot bind service socket to port %d", (int)SERVICE_PORT);
        return -1;
    }
    if (listen(fd, SERVICE_BACKLOG) != 0) {
        LOG2SVR("Cannot listening on service socket");
        return -1;
    }
    if (pool->Reg(fd, service_conn_process, nullptr)) {
        LOG2SVR("Pool exception while register service socket");
        return -1;
    }
    return 0;
}

void shut_service(Pool *pool) {
    delete[] service_buffer;
    LOG2SYS("http shutdown");
}

int service_request_process(PoolEvent *e) {
    memset(service_buffer, 0, SERVICE_BUFFER_SIZE);
    size_t len = read(e->fd, service_buffer, SERVICE_BUFFER_SIZE);
    if (len == 0 || (len == -1 && errno != EAGAIN))
        return 1;
    char _;
    while (read(e->fd, &_, 1) > 0);
    ProtocolHeader request(service_buffer, len);
    Response response;
    if (request.GetMethod() == UNSUPPORTED)
        return 1;
    std::string path = request.GetPath();
    LOG2SVR("Access %s", path.c_str());
    process_route(request, response, route_service);
    size_t write_len = response.Write(e->fd);
    if (write_len <= 0)
        return 1;
    if (!response.KeepAlive(false, false))
        return 1;
    return 0;
}