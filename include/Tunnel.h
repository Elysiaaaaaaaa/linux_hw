//
// Created by elysia on 2025/4/5.
//

#ifndef LINUX_HW_TUNNEL_H
#define LINUX_HW_TUNNEL_H


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
class Tunnel {
private:

public:
    Tunnel(int proj_id, const char *pathname);
    void init_car(txt_reader& reader);
    void enter(Car *car);
    void leave(Car *car);
    void main_process();
public:
//    控制隧道中车数量
    int tunnel_number_of_cars;
//    控制车总的数量
    int total_number_of_cars_tunnel;
    int car_count_;               // 隧道中车辆数量
    key_t mutex_key;
    key_t block_key;
    key_t car_count_key;
    std::vector<Car> cars;
    int mutex_;    // 用来保护内部状态
    int block_;    // 用来阻塞不符合方向的车
    Direction current_direction_; // 当前隧道中车的方向
};



#endif //LINUX_HW_TUNNEL_H
