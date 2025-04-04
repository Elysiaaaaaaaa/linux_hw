//
// Created by elysia on 2025/4/2.
//

#ifndef LINUX_HW_IPC_H
#define LINUX_HW_IPC_H



#pragma once
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <iostream>
#include <sys/sem.h>
#include <sys/shm.h>
#include <stdlib.h>
using namespace std;

//信号灯

// 用于semctl
union semun;

// 用于semop
struct sembuf;

int sem_get(int key,int nsems,bool init= false, int semval=0);
void Wait(int sid,int sem_num);
void Signal(int sid,int sem_num);
void sem_del(int semid);


//共享内存区
int create_shm(int mykey,int size);
void* get_shm(int shmid);
void disconnect_shm(void *shmaddr);
void delete_shm(int shmid);


#endif //LINUX_HW_IPC_H
