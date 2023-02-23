/*
 * Copyright (c) shinsya.G 
 */

#ifndef DCCLIVE_POOL_H
#define DCCLIVE_POOL_H

#define EVENTS_IN_ONCE      80

#include <sys/epoll.h>
#include <memory>

typedef struct PoolEvent PoolEvent;
// 当Handler返回非0时，fd将被从Pool中移除
typedef int(*PoolEventHandler)(PoolEvent *);

struct PoolEvent {
    int fd;
    PoolEventHandler handler;
    void *data;
};

class Pool {
private:
    int epoll_fd;
    epoll_event *event_buf;
private:
    int remove_fd(int fd) const;
    void event_handle(epoll_event *e);
public:
    Pool();
    int Reg(int fd, PoolEventHandler handler, void *data) const;
    ~Pool();
    void CheckOut();
};


#endif //DCCLIVE_POOL_H
