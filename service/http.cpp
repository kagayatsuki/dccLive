/*
 * Copyright (c) shinsya.G 
 */
#include "service.h"
#include "pool.h"
#include "env.h"

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

int init_service(Pool *pool) {
    http::connectionPool = pool;
    return 0;
}

void shut_service(Pool *pool) {
    LOG2SYS("http shutdown");
}