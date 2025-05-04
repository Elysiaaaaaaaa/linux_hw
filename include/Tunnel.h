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

#define PROJ_MUTEX_KEY_OFFSET 0
#define PROJ_BLOCK_KEY_OFFSET 1
#define PROJ_CARCOUNT_KEY_OFFSET 2
#define PROJ_MAXCARS_KEY_OFFSET 3
#define PROJ_TOTALCARS_KEY_OFFSET 4
#define PROJ_SHM_MAILBOX_OFFSET 8 // 假设的 mailbox 共享内存偏移量
#define PROJ_DIRECTION_TUNNEL_OFFSET 9
#define PROJ_ZERO_CAR_OFFSET 10

#define PROJ_SHM_TUNNEL_OFFSET 11

extern int total_number_of_cars;
extern int maximum_number_of_cars_in_tunnel;//隧道最大汽车容量
enum class Direction;
class Car;
// 定义可共享的数据结构
struct SharedTunnelData {
    Direction current_direction_;
    int car_count_;
    // 可以添加其他需要共享的基本数据类型成员
};

class Tunnel {
public:
    Tunnel(int proj_id, const char *pathname);
    ~Tunnel();
//    void enter(Car *car);
//    void leave(Car *car);
    void show(); // 新增 show 函数声明
//    void main_process();
public:
//    控制隧道中车数量
    int tunnel_number_of_cars;
//    控制车总的数量
    int total_number_of_cars_tunnel;
    int car_count_;               // 隧道中车辆数量
    key_t mutex_key;
    key_t block_key;
    key_t car_count_key;
    key_t direction_changed_key;
    key_t zero_car_key;
    int mutex_;    // 用来保护内部状态
    int block_;    // 用来阻塞超过容量的车
    int direction_changed_;    // 用来阻塞不符合方向的车
    int zero_car_;    // 用来标识车辆是否为0
    Direction current_direction_; // 当前隧道中车的方向
};



#endif //LINUX_HW_TUNNEL_H
