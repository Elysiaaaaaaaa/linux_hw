#include <unistd.h>
#include "../include/ipc.h"
#include "../include/mp.h"
#include "../include/logger.h"
#include <iostream>
#include <cstdio>
using namespace std;

// 测试函数
void test() {
    // 用于标记是否进行信号量测试
    bool sem_test = true;
    if (sem_test) {
        // 获取信号量
        key_t k = Ftok(0);
        int sem_sign = sem_get(k, 1, true, 0);
        if (sem_sign == -1) {
            cerr << "Failed to get semaphore. Exiting..." << endl;
            return;
        }

        pid_t pidChild = Fork();
        if (pidChild == -1) {
            cerr << "Failed to fork child process. Exiting..." << endl;
            sem_del(sem_sign);  // 释放信号量
            return;
        } else if (pidChild == 0) {
            // 子进程
            // 模拟一些操作后释放信号量
            sleep(1);  // 暂停1秒，模拟子进程的其他操作
            Signal(sem_sign, 0);
//            struct sembuf op;
//            op.sem_num = 0;
//            op.sem_op = 1;
//            op.sem_flg = 0;
//            semop(sem_sign, &op, 1);
            printf("c\n");  // 输出子进程标识
            fflush(stdout);  // 刷新输出缓冲区，确保立即输出
        } else {
            // 父进程
            Wait(sem_sign, 0);

//            struct sembuf op;
//            op.sem_num = 0;
//            op.sem_op = -1;
//            op.sem_flg = 0;
//            semop(sem_sign, &op, 1);

            printf("f\n");  // 输出父进程标识
            fflush(stdout);  // 刷新输出缓冲区，确保立即输出
            // 父进程等待子进程完成后，删除信号量
            sem_del(sem_sign);
        }
    }
}

int main() {
    test();
    return 0;
}