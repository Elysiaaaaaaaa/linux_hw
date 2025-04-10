//
// Created by elysia on 2025/4/5.
//

#ifndef CAR_H
#define CAR_H


// Car.h
// Created by elysia on 2025/4/5.
// Car class using IPC (Semaphore + Shared Memory)

#include "ipc.h"
#include "logger.h"
#include "txt_reader.h"
#include <string>
#include <vector>
enum class Direction { Eastbound, Westbound }; // 新增方向枚举

// 存储每条操作

struct Operation {
    bool isWrite; // true 表示写操作，false 表示读操作
    std::string data; // 写操作的数据
    int time; // 操作时间
    int mailbox; // 邮箱编号
    int length; // 读操作的数据长度
};
class Car {
public:
    Car(int proj_id, const std::string& path, int shm_size, int car_id, Direction dir, txt_reader& reader);
    ~Car();

    void enter() const;    // Request access (decrease semaphore)
    void leave() const;    // Release access (increase semaphore)

    void* getSharedMemory(); // Get pointer to shared memory

    // 新增辅助接口
    int getCarId() const;
    Direction getDirection() const;
    std::string getDirectionStr() const; // 返回方向的字符串，方便打印

    // 添加操作到操作列表
    void addOperation(const Operation& op);
    const std::vector<Operation>& getOperations() const;
    void show() const;

private:
    key_t key_;        // IPC key
    int semid_;        // Semaphore ID
    int shmid_;        // Shared memory ID
    void* shmaddr_;    // Shared memory address
public:
    int car_id_;       // 车的编号
    Direction direction_; // 方向
    std::vector<Operation> operations; // 操作列表
};

#endif // CAR_H

