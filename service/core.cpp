/*
 * Copyright (c) shinsya.G 
 */
#include "service.h"
#include "pool.h"
#include "env.h"

extern int init_console(Pool *pool);
extern int init_service(Pool *pool);
extern void shut_console(Pool *pool);
extern void shut_service(Pool *pool);

extern int process_console(void *);

CycleActionBlock lifetimeNextAction;
Pool *connectionPool;

void setNextAction(CycleAction action, void *data) {
    lifetimeNextAction.action = action;
    lifetimeNextAction.data = data;
}

void service_once_cycle() {
    CycleAction currentAction = lifetimeNextAction.action;
    while (currentAction) {
        lifetimeNextAction.action = nullptr;
        if (currentAction(lifetimeNextAction.data))
            break;
        currentAction = lifetimeNextAction.action;
    }
}

int service_init() {
    connectionPool = new Pool();
    if (init_console(connectionPool) || init_service(connectionPool))
        return -1;
    LOG2SVR("service started");
    return 0;
}

void service_shutdown() {
    shut_service(connectionPool);
    shut_console(connectionPool);
    LOG2SVR("service shutdown");
    delete connectionPool;
}

int process_core(void *d) {
    setNextAction(process_console, d);
    return 0;
}