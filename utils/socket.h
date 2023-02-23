/*
 * Copyright (c) shinsya.G 
 */

#ifndef DCCLIVE_SOCKET_H
#define DCCLIVE_SOCKET_H

#include <sys/socket.h>
#include <fcntl.h>

void setNonBlock(int fd);
int createNonBlockSocket(int domain, int type, int proto);

#endif //DCCLIVE_SOCKET_H
