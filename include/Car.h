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


enum class Direction { Eastbound, Westbound }; // ��������ö��
enum class State { WAITING, INNER, OUT }; // ����״̬ö��
class Tunnel;
// �洢ÿ������
extern int total_number_of_mailboxes;
struct Operation {
    bool isWrite; // true ��ʾд������false ��ʾ������
    string data; // д����������
    int time; // ����ʱ��
    int mailbox; // ������
    int length; // �����������ݳ���
};
class Car {
public:
//    Car(int proj_id, const std::string& path, int shm_size, int car_id, Direction dir, txt_reader& reader);
    Car(int car_id, Direction dir, txt_reader& reader);
    ~Car();

//    void enter(int semid_tunnel_can_enter, Tunnel* tunnel);    // Request access (decrease semaphore)
//    void leave(int semid_tunnel_can_enter, Tunnel* tunnel);    // Release access (increase semaphore)

//    void* getSharedMemory(); // Get pointer to shared memory

    // ���������ӿ�
    int getCarId() const;
    Direction getDirection() const;
    std::string getDirectionStr() const; // ���ط�����ַ����������ӡ

    // ��Ӳ����������б�
    void addOperation(const Operation& op);
    void show() const;
private:
    std::chrono::milliseconds calculateTravelTime();
    void parseOperations(txt_reader& reader);

private:
    vector<int> m;      //ÿ������ά���Ķ�ָ��
    int adjusted_travel_time;

public:
    int car_id;       // ���ı��
    Direction direction_; // ����
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time; // ������ʼ��Խ�����ʱ��
    std::chrono::milliseconds cost_time; // ������Խ���Ԥ��ʹ�õ�ʱ��
    State state;//�����ĵ�ǰ״̬��1δ���������2�Ѿ�����У�3�����
    string model_str;//�ֻ��ڴ�
    std::vector<Operation> operations; // �����б�
    int handel;
    int wait_handel;
    bool out = false;
};
#endif // CAR_H

