/*
 * Copyright (c) shinsya.G 
 */

#include "pool.h"
#include "service.h"
#include "../log/env.h"

#include <unistd.h>

Pool::Pool():
event_buf(new epoll_event[EVENTS_IN_ONCE]), epoll_fd(epoll_create(1))
{}

int Pool::Reg(int fd, PoolEventHandler handler, void *data) const {
    epoll_event ev{};
    auto *pev = new PoolEvent;
    pev->fd = fd;
    pev->data = data;
    pev->handler = handler;
    ev.data.ptr = (void *)pev;
    ev.events = EPOLLIN | EPOLLET;
    return epoll_ctl(this->epoll_fd, EPOLL_CTL_ADD, fd, &ev);
}

int Pool::remove_fd(int fd) const {
    int st = epoll_ctl(this->epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
    close(fd);
    return st;
}

Pool::~Pool() {
    close(this->epoll_fd);
    delete[] this->event_buf;
}

void Pool::event_handle(epoll_event *e) {
    auto *event = (PoolEvent *)e->data.ptr;
    if (!event->handler || event->handler(event)) {
        this->remove_fd(event->fd);
        delete event;
    }
}

void Pool::CheckOut() {
    while (true) {
        int event_count = epoll_wait(this->epoll_fd, this->event_buf, EVENTS_IN_ONCE, EPOLL_WAIT_MILL_SEC);
        // 无事件
        if (event_count == 0)
            break;
        if (event_count == -1) {
            LOG2SYS("Unhandled Epoll exception in Pool %p", this);
            break;
        }
        for (int i = 0; i < event_count; ++i)
            event_handle(event_buf + i);
    }
}


