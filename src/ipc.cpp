//
// Created by elysia on 2025/4/2.
//

#include "../include/ipc.h"

key_t Ftok(int proj_id, const char *pathname) {
    key_t key = ftok(pathname, proj_id);
    if (key == -1) {
        Logger::log(LogLevel::ERROR, "Ftok.ftok failed: path " + std::string(pathname) + " not exist, please cd to linux_hw");
        exit(EXIT_FAILURE);
    }
    return key;
}


union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

int sem_get(key_t key, int nsems, bool init, int semval) {
    int semid = semget(key, nsems, IPC_CREAT | 0666);
    if (semid == -1) {
        Logger::log(LogLevel::ERROR, "sem_get.semget failed");
        exit(EXIT_FAILURE);
    }
    if (init) {
        union semun arg;
        unsigned short *values = (unsigned short *)malloc(nsems * sizeof(unsigned short));
        if (values == NULL) {
            Logger::log(LogLevel::ERROR, "sem_get.malloc failed");
            semctl(semid, 0, IPC_RMID);
            exit(EXIT_FAILURE);
        }
        for (int i = 0; i < nsems; i++) {
            values[i] = semval;
        }
        arg.array = values;
        if (semctl(semid, 0, SETALL, arg) == -1) {
            Logger::log(LogLevel::ERROR, "sem_get.semctl_SETALL failed");
            free(values);
            semctl(semid, 0, IPC_RMID);
            exit(EXIT_FAILURE);
        }
        free(values);
    }
    return semid;
}

void Wait(int semid, int sn) {
    struct sembuf op;
    op.sem_num = sn;
    op.sem_op = -1;
    op.sem_flg = 0;
    if (semop(semid, &op, 1) == -1) {
        Logger::log(LogLevel::ERROR, "Wait.semop failed");
        exit(EXIT_FAILURE);
    }
}

void Signal(int sid, int sn) {
    struct sembuf op;
    op.sem_num = sn;
    op.sem_op = 1;
    op.sem_flg = 0;
    if (semop(sid, &op, 1) == -1) {
        Logger::log(LogLevel::ERROR, "Signal.semop failed");
        exit(EXIT_FAILURE);
    }
}

void sem_del(int semid) {
    if (semctl(semid, 0, IPC_RMID) == -1) {
        Logger::log(LogLevel::ERROR, "sem_del.semctl failed");
        exit(EXIT_FAILURE);
    }
}

// 共享内存区
int shm_get(int key, int size, int oflag) {
    int shmid = shmget(key, size, oflag);
    if (shmid == -1) {
        Logger::log(LogLevel::ERROR, "shm_get.shmget failed");
        exit(EXIT_FAILURE);
    }
    return shmid;
}

void* shm_conn(int shmid) {
    void* retshm = shmat(shmid, (void*)0, 0);
    if (retshm == (void*)-1) {
        Logger::log(LogLevel::ERROR, "shm_conn.shmat failed");
        exit(EXIT_FAILURE);
    }
    return retshm;
}

void shm_disconn(void* shmaddr) {
    if (shmdt(shmaddr) == -1) {
        Logger::log(LogLevel::ERROR, "shm_disconn.shmdt failed");
        exit(EXIT_FAILURE);
    }
}

void shm_del(int shmid) {
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        Logger::log(LogLevel::ERROR, "shm_del.shmctl failed");
        exit(EXIT_FAILURE);
    }
}
