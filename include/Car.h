//
// Created by elysia on 2025/4/5.
//

#ifndef CAR_H
#define CAR_H


// Car.h
// Created by elysia on 2025/4/5.
// Car class using IPC (Semaphore + Shared Memory)
#include "mp.h"
#include "ipc.h"
#include "logger.h"
#include "txt_reader.h"
#include "tunnel.h"
#include <string>
#include <ctime>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <random>
#include<algorithm>
using namespace std;


enum class Direction { Eastbound, Westbound }; // 新增方向枚举
enum class State { WAITING, INNER, OUT }; // 新增状态枚举
class Tunnel;
// 存储每条操作
extern int total_number_of_mailboxes;
struct Operation {
    bool isWrite; // true 表示写操作，false 表示读操作
    string data; // 写操作的数据
    int time; // 操作时间
    int mailbox; // 邮箱编号
    int length; // 读操作的数据长度
};
class Car {
public:
//    Car(int proj_id, const std::string& path, int shm_size, int car_id, Direction dir, txt_reader& reader);
    Car(int car_id, Direction dir, txt_reader& reader);
    ~Car();

//    void enter(int semid_tunnel_can_enter, Tunnel* tunnel);    // Request access (decrease semaphore)
//    void leave(int semid_tunnel_can_enter, Tunnel* tunnel);    // Release access (increase semaphore)

//    void* getSharedMemory(); // Get pointer to shared memory

    // 新增辅助接口
    int getCarId() const;
    Direction getDirection() const;
    std::string getDirectionStr() const; // 返回方向的字符串，方便打印

    // 添加操作到操作列表
    void addOperation(const Operation& op);
    void show() const;
private:
    std::chrono::milliseconds calculateTravelTime();
    void parseOperations(txt_reader& reader);

private:
    vector<int> m;      //每个邮箱维护的读指针
    int adjusted_travel_time;

public:
    int car_id;       // 车的编号
    Direction direction_; // 方向
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time; // 汽车开始穿越隧道的时间
    std::chrono::milliseconds cost_time; // 汽车穿越隧道预计使用的时间
    State state;//汽车的当前状态，1未进入隧道，2已经隧道中，3出隧道
    string model_str;//手机内存
    std::vector<Operation> operations; // 操作列表
    int handel;
    int wait_handel;
    bool out = false;
};
#endif // CAR_H

