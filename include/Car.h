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
enum class Direction { Eastbound, Westbound }; // ��������ö��

// �洢ÿ������

struct Operation {
    bool isWrite; // true ��ʾд������false ��ʾ������
    std::string data; // д����������
    int time; // ����ʱ��
    int mailbox; // ������
    int length; // �����������ݳ���
};
class Car {
public:
    Car(int proj_id, const std::string& path, int shm_size, int car_id, Direction dir, txt_reader& reader);
    ~Car();

    void enter() const;    // Request access (decrease semaphore)
    void leave() const;    // Release access (increase semaphore)

    void* getSharedMemory(); // Get pointer to shared memory

    // ���������ӿ�
    int getCarId() const;
    Direction getDirection() const;
    std::string getDirectionStr() const; // ���ط�����ַ����������ӡ

    // ��Ӳ����������б�
    void addOperation(const Operation& op);
    const std::vector<Operation>& getOperations() const;
    void show() const;

private:
    key_t key_;        // IPC key
    int semid_;        // Semaphore ID
    int shmid_;        // Shared memory ID
    void* shmaddr_;    // Shared memory address
public:
    int car_id_;       // ���ı��
    Direction direction_; // ����
    std::vector<Operation> operations; // �����б�
};

#endif // CAR_H

