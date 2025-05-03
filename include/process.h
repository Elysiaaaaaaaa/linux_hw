//
// Created by elysia on 2025/4/13.
//

#ifndef LINUX_HW_PROCESS_H
#define LINUX_HW_PROCESS_H

#include "Car.h"
#include "mailbox.h"
#include "ipc.h"   // 自己有的信号量头文件
#include "mp.h"
#include "Logger.h"
#include <stdio.h>
#include <unistd.h>
#include <vector>
#include <cstdio>
#include <unistd.h>
#include <pthread.h> // 引入 pthread 库用于互斥锁
#include <semaphore.h>
#include <chrono>
#define SEM_NAME "/my_semaphore"

extern int total_number_of_cars;
extern int maximum_number_of_cars_in_tunnel;//隧道最大汽车容量
enum class Direction;
class Car;
class Tunnel;

class process {

public:
    process(int num_mailboxes, int mem_size, int proj_id, const char *pathname);
    ~process(); // 销毁互斥锁
    void init_car(txt_reader& reader);
    void enter(Car *car);
    void leave(Car *car);
    void main_process();
    bool isGreenLight(Direction dir);

    void switchDirection();
public:
    Tunnel* tunnel;
    mailbox* mail_box;
    std::vector<Car> cars;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time; // 添加起始时间成员变量
    bool direction_switched;
};


#endif //LINUX_HW_PROCESS_H
