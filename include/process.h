//
// Created by elysia on 2025/4/13.
//

#ifndef LINUX_HW_PROCESS_H
#define LINUX_HW_PROCESS_H

#include "Car.h"
#include "ipc.h"   // 自己有的信号量头文件
#include "mp.h"
#include "Logger.h"
#include <stdio.h>
#include <unistd.h>
#include <vector>
#include <cstdio>
#include <unistd.h>
extern int total_number_of_cars;
extern int maximum_number_of_cars_in_tunnel;//隧道最大汽车容量
enum class Direction;
class Car;
class Tunnel;


class process {

public:
    process(int proj_id, const char *pathname);
    void init_car(txt_reader& reader);
    void enter(Car *car);
    void leave(Car *car);
    void main_process();
public:
    Tunnel* tunnel;
    std::vector<Car> cars;
};


#endif //LINUX_HW_PROCESS_H
