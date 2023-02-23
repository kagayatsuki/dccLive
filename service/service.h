/*
 * Copyright (c) shinsya.G 
 */

#ifndef DCCLIVE_SERVICE_H
#define DCCLIVE_SERVICE_H

#ifndef CONSOLE_PORT
#define CONSOLE_PORT        6068
#endif
#ifndef SERVICE_PORT
#define SERVICE_PORT        7788
#endif

#ifndef SERVICE_BACKLOG
#define SERVICE_BACKLOG     300
#endif
#ifndef CONSOLE_BACKLOG
#define CONSOLE_BACKLOG     10
#endif

#ifndef EPOLL_WAIT_MILL_SEC
#define EPOLL_WAIT_MILL_SEC 10
#endif

#define DEFAULT_SESSION_EXPIRE          1800        // 默认session过期时间1800秒

// 下个生命循环过程处理动作，若返回非0表示遇到错误，将终止本次循环
typedef int(*CycleAction)(void *);
typedef struct CycleActionBlock {
  CycleAction action;
  void *data;
} CycleActionBlock;

extern void setNextAction(CycleAction action, void *data);
extern void service_once_cycle();
extern int process_core(void *);

extern int service_init();
extern void service_shutdown();
#endif //DCCLIVE_SERVICE_H
