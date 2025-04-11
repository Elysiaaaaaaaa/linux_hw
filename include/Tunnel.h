//
// Created by elysia on 2025/4/5.
//

#ifndef LINUX_HW_TUNNEL_H
#define LINUX_HW_TUNNEL_H


#include "Car.h"
#include "ipc.h"   // 你自己有的信号量头文件
#include "mp.h"
#include "Logger.h"
#include <stdio.h>
#include <unistd.h>
#include <vector>
#include <cstdio>
#include <unistd.h>
extern int total_number_of_cars;
class Tunnel {
private:
    int mutex_;    // 用来保护内部状态
    int block_;    // 用来阻塞不符合方向的车
    Direction current_direction_; // 当前隧道中车的方向
    int car_count_;               // 隧道中车辆数量
public:
    Tunnel(int proj_id, const char *pathname, std::vector<Car>& cars);
    void enter(Car &car);
    void leave(Car &car);
    void main_process();
public:
//    控制隧道车数量
    int semid_tunnel_car;
    std::vector<Car> cars;
};



#endif //LINUX_HW_TUNNEL_H
