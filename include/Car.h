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
enum class Direction { Eastbound, Westbound }; // ��������ö��
enum class State { WAITING, INNER, OUT }; // ����״̬ö��
class Tunnel;
// �洢ÿ������
extern int total_number_of_mailboxes;
struct Operation {
    bool isWrite; // true ��ʾд������false ��ʾ������
    std::string data; // д����������
    int time; // ����ʱ��
    int mailbox; // ������
    int length; // �����������ݳ���
};
class Car {
public:
//    Car(int proj_id, const std::string& path, int shm_size, int car_id, Direction dir, txt_reader& reader);
    Car(int car_id, Direction dir, txt_reader& reader);
    ~Car();

    void enter(int semid_tunnel_can_enter, Tunnel* tunnel);    // Request access (decrease semaphore)
    void leave(int semid_tunnel_can_enter, Tunnel* tunnel);    // Release access (increase semaphore)

//    void* getSharedMemory(); // Get pointer to shared memory

    // ���������ӿ�
    int getCarId() const;
    Direction getDirection() const;
    std::string getDirectionStr() const; // ���ط�����ַ����������ӡ

    // ��Ӳ����������б�
    void addOperation(const Operation& op);
    const std::vector<Operation>& getOperations() const;
    bool exet_op();
    bool overtime(time_t ct = -1);
    void show() const;
    bool main_process(int semid_tunnel_can_enter, Tunnel* tunnel);

private:
//    key_t key_;        // IPC key
//    int semid_tunnel_can_enter;        // ��������ɷ�������복
//    int shmid_;        // Shared memory ID
//    void* shmaddr_;    // Shared memory address
    vector<int> m;      //ÿ������ά���Ķ�ָ��

public:
    int car_id;       // ���ı��
    Direction direction_; // ����
    time_t start_time;//������Խ�����ǰʹ�õ�ʱ��
    int cost_time;//������Խ���ʹ�õ�ʱ��
    State state;//�����ĵ�ǰ״̬��1δ���������2�Ѿ�����У�3�����
    string model_str;//�ֻ��ڴ�
    std::vector<Operation> operations; // �����б�
    bool set_value();
};

#endif // CAR_H

