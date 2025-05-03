//
// Created by elysia on 2025/4/2.
//

#include "ipc.h"

key_t Ftok(const char *pathname, int proj_id) {
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
    /**
     * brief 获取或创建一个信号量集，并可选择对其进行初始化
     *
     * @param key 用于标识信号量集的键值，通常由 ftok 函数生成
     * @param nsems 信号量集中信号量的数量
     * @param init 是否对信号量集进行初始化的标志
     * @param semval 初始化时信号量的初始值
     * @return int 若操作成功，返回信号量集的标识符；若失败，返回 -1
     */
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

int sem_get_val(int semid, int sn) {
    return semctl(semid, sn, GETVAL);
}

void Wait(int semid, int sn) {
    /**
     * @brief 对指定信号量执行 P 操作（等待操作）
     *
     * @param semid 信号量集的标识符
     * @param sn 要操作的信号量的编号
     * @return bool 操作成功返回 true，失败返回 false
     */
    struct sembuf op;
    op.sem_num = sn;
    op.sem_op = -1;
    op.sem_flg = 0;
//    if (semop(semid, &op, 1) == -1) {
//        Logger::log(LogLevel::ERROR, "Wait.semop failed");
//        exit(EXIT_FAILURE);
//    }
    semop(semid, &op, 1);
}

void Signal(int sid, int sn) {
    /**
     * @brief 对指定信号量执行 V 操作（激活操作）
     *
     * @param semid 信号量集的标识符
     * @param sn 要操作的信号量的编号
     * @return bool 操作成功返回 true，失败返回 false
     */
    struct sembuf op;
    op.sem_num = sn;
    op.sem_op = 1;
    op.sem_flg = 0;
//    if (semop(sid, &op, 1) == -1) {
//        Logger::log(LogLevel::ERROR, "Signal.semop failed");
//        exit(EXIT_FAILURE);
//    }
    semop(sid, &op, 1);
}

void sem_del(int semid) {
    if (semctl(semid, 0, IPC_RMID) == -1) {
        Logger::log(LogLevel::ERROR, "sem_del.semctl failed");
        exit(EXIT_FAILURE);
    }
}

// 共享内存区
int shm_init(int key, int size, int oflag) {
    /**
     * @brief 获取共享内存段的标识符
     *
     * 该函数使用 shmget 系统调用，根据指定的键值、大小和标志来获取或创建共享内存段。
     * 如果成功，返回共享内存段的标识符；如果失败，记录错误日志并终止程序。
     *
     * @param key 用于标识共享内存段的键值，通常由 ftok 函数生成
     * @param size 共享内存段的大小（字节）
     * @param oflag 标志位，用于指定创建和访问共享内存段的权限和行为
     * @return int 成功时返回共享内存段的标识符，失败时程序终止
     */

    int shmid = shmget(key, size, oflag);
    if (shmid == -1) {
        Logger::log(LogLevel::ERROR, "shm_init.shmget failed");
        exit(EXIT_FAILURE);
    }
    return shmid;
}

void* shm_conn(int shmid) {
    /**
     * @brief 将共享内存段连接到当前进程的地址空间
     *
     * @param shmid 共享内存段的标识符
     * @return void* 成功时返回共享内存段在当前进程中的起始地址，失败时返回 (void*)-1
     */
    void* retshm = shmat(shmid, (void*)0, 0);
    if (retshm == (void*)-1) {
        Logger::log(LogLevel::ERROR, "shm_conn.shmat failed");
        exit(EXIT_FAILURE);
    }
    return retshm;
}

void shm_disconn(void* shmaddr) {
    /**
     * @brief 将共享内存段从当前进程的地址空间分离
     *
     * @param shmaddr 共享内存段在当前进程中的起始地址
     */
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
