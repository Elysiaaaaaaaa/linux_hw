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

enum class Direction { Eastbound, Westbound }; // ��������ö��

class Car {
public:
    Car(int proj_id, const std::string& path, int shm_size, int car_id, Direction dir);
    ~Car();

    void enter();    // Request access (decrease semaphore)
    void leave();    // Release access (increase semaphore)

    void* getSharedMemory(); // Get pointer to shared memory

    // ���������ӿ�
    int getCarId() const;
    Direction getDirection() const;
    std::string getDirectionStr() const; // ���ط�����ַ����������ӡ

private:
    key_t key_;        // IPC key
    int semid_;        // Semaphore ID
    int shmid_;        // Shared memory ID
    void* shmaddr_;    // Shared memory address
public:
    int car_id_;       // ���ı��
    Direction direction_; // ����
};

#endif // CAR_H

