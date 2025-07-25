//
// Created by elysia on 2025/4/2.
//

#ifndef LINUX_HW_IPC_H
#define LINUX_HW_IPC_H

#include <stdio.h>
#include <sys/types.h>
#include <iostream>
#include <sys/sem.h>
#include <sys/shm.h>
#include <stdlib.h>
#include "logger.h"
using namespace std;




//信号灯

// 用于semctl
union semun;

// 用于semop
//struct sembuf;

int sem_get(int key,int nsems,bool init= false, int semval=0);
void Wait(int sid,int sem_num);
void Signal(int sid,int sem_num);
void sem_del(int semid);
int sem_get_val(key_t semid, int nsems=0);

//共享内存区
int shm_init(int key,int size,int oflag=IPC_CREAT|0666);
void* shm_conn(int shmid);
void shm_disconn(void *shmaddr);
void shm_del(int shmid);

#endif //LINUX_HW_IPC_H

