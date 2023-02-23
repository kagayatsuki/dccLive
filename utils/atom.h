/*
 * Copyright (c) shinsya.G 
 */

#ifndef DCCLIVE_ATOM_H
#define DCCLIVE_ATOM_H

// 原子锁结构
typedef struct atom_s {
    int ticket;
    int turn;
} atom;

// 初始化原子锁
void atom_init(atom *a);
// 获取锁
void atom_lock(atom *a);
// 释放锁
void atom_unlock(atom *a);

#endif //DCCLIVE_ATOM_H
