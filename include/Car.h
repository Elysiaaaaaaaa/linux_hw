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
enum class Direction { Eastbound, Westbound }; // 新增方向枚举
enum class State { WAITING, INNER, OUT }; // 新增状态枚举
class Tunnel;
// 存储每条操作
extern int total_number_of_mailboxes;
struct Operation {
    bool isWrite; // true 表示写操作，false 表示读操作
    std::string data; // 写操作的数据
    int time; // 操作时间
    int mailbox; // 邮箱编号
    int length; // 读操作的数据长度
};
class Car {
public:
//    Car(int proj_id, const std::string& path, int shm_size, int car_id, Direction dir, txt_reader& reader);
    Car(int car_id, Direction dir, txt_reader& reader);
    ~Car();

    void enter(int semid_tunnel_can_enter, Tunnel* tunnel);    // Request access (decrease semaphore)
    void leave(int semid_tunnel_can_enter, Tunnel* tunnel);    // Release access (increase semaphore)

//    void* getSharedMemory(); // Get pointer to shared memory

    // 新增辅助接口
    int getCarId() const;
    Direction getDirection() const;
    std::string getDirectionStr() const; // 返回方向的字符串，方便打印

    // 添加操作到操作列表
    void addOperation(const Operation& op);
    const std::vector<Operation>& getOperations() const;
    bool exet_op();
    bool overtime(time_t ct = -1);
    void show() const;
    bool main_process(int semid_tunnel_can_enter, Tunnel* tunnel);

private:
//    key_t key_;        // IPC key
//    int semid_tunnel_can_enter;        // 隧道容量可否继续进入车
//    int shmid_;        // Shared memory ID
//    void* shmaddr_;    // Shared memory address
    vector<int> m;      //每个邮箱维护的读指针

public:
    int car_id;       // 车的编号
    Direction direction_; // 方向
    time_t start_time;//汽车穿越隧道当前使用的时间
    int cost_time;//汽车穿越隧道使用的时间
    State state;//汽车的当前状态，1未进入隧道，2已经隧道中，3出隧道
    string model_str;//手机内存
    std::vector<Operation> operations; // 操作列表
    bool set_value();
};

#endif // CAR_H

