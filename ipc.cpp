//
// Created by elysia on 2025/4/2.
//

#include "ipc.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <iostream>
#include <stdlib.h>
using namespace std;


struct sembuf
{
    unsigned short sem_num; //信号在信号集中的索引
    short sem_op; //操作类型
    short sem_flg; //操作标志
};

union semun
{
    int val;	//信号灯的初值,SETVAL
    struct semid_ds *buf;   //IPC_SET
    unsigned short *array;  //SETALL
};

//建立信号量
int sem_get(int key,int nsems,bool init, int semval)
{
/*
 * key:创建/打开的信号灯集合标识
 * nsems:包含的信号灯数量
 * init:是否重新创建 false
 * semval:信号量初始值 0
 * return 信号量集合标识
 */
    //创建信号量
    int semid = semget(key, nsems, IPC_CREAT | 0666);
    if (semid == -1)
    {
        perror("ERROR:sem_get.semget");
        exit(EXIT_FAILURE);
    }
    if (init) {
        //初始化信号量
        union semun arg;
        unsigned short *values = (unsigned short *) malloc(nsems * sizeof(unsigned short));
        if (values == NULL) {
            perror("ERROR:sem_get.malloc");
            exit(EXIT_FAILURE);
        }
        for (int i = 0; i < nsems; i++) {
            values[i] = semval;
        }
        arg.array = values;
        if (semctl(semid, 0, SETALL, arg) == -1) {
            perror("ERROR:sem_get.semctl_SETALL");
            free(values);
            exit(EXIT_FAILURE);
        }
        free(values);
    }
    return semid;
}


//等待信号量
void Wait(int sid, int sn)
{
    struct sembuf op;
    op.sem_num = sn;
    op.sem_op = -1;
    op.sem_flg = 0;
    if (semop(sid, &op, 1) == -1)
    {
        perror("ERROR:Wait.semop");
        exit(EXIT_FAILURE);
    }
}

//通过信号量
void Signal(int sid, int sn)
{
    struct sembuf op;
    op.sem_num = sn;
    op.sem_op = 1;
    op.sem_flg = 0;
    if (semop(sid, &op, 1) == -1)
    {
        perror("ERROR:Signal.semop");
        exit(EXIT_FAILURE);
    }
}

//删除信号量
void sem_del(int semid)
{
    if (semctl(semid, 0, IPC_RMID) == -1)
    {
        perror("ERROR:sem_del.semctl");
        exit(EXIT_FAILURE);
    }
}








//创建共享内存区
int create_shm(int mykey,int size)
{
	int shmid=shmget(mykey,size,IPC_CREAT|0666);
	if(shmid==-1)
	{
		printf("shmget error!");
		exit(EXIT_FAILURE);
	}
	return shmid;
}

//返回制定的共享内存区
//retshm指向该块内存区
void* get_shm(int shmid)
{
	void* retshm=shmat(shmid,(void*)0,0);
	if(shmid==-1)
	{
		printf("shmat error!\n");
		exit(EXIT_FAILURE);
	}
	return retshm;
}

//断开共享内存区
void disconnect_shm(void *shmaddr)
{
	if(shmdt(shmaddr)==-1)
	{
		printf("shmdt error!\n");
		exit(EXIT_FAILURE);
	}
}

//删除共享内存区
void delete_shm(int shmid)
{
	if(shmctl(shmid,IPC_RMID,NULL)==-1)
	{
		printf("shmctl error!\n");
		exit(EXIT_FAILURE);
	}
}


