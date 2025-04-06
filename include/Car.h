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
#include <string>

enum class Direction { Eastbound, Westbound }; // 新增方向枚举

class Car {
public:
    Car(int proj_id, const std::string& path, int shm_size, int car_id, Direction dir);
    ~Car();

    void enter();    // Request access (decrease semaphore)
    void leave();    // Release access (increase semaphore)

    void* getSharedMemory(); // Get pointer to shared memory

    // 新增辅助接口
    int getCarId() const;
    Direction getDirection() const;
    std::string getDirectionStr() const; // 返回方向的字符串，方便打印

private:
    key_t key_;        // IPC key
    int semid_;        // Semaphore ID
    int shmid_;        // Shared memory ID
    void* shmaddr_;    // Shared memory address
public:
    int car_id_;       // 车的编号
    Direction direction_; // 方向
};

#endif // CAR_H

