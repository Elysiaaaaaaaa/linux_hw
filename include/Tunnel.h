//
// Created by elysia on 2025/4/5.
//

#ifndef LINUX_HW_TUNNEL_H
#define LINUX_HW_TUNNEL_H


#include "Car.h"
#include "ipc.h"   // �Լ��е��ź���ͷ�ļ�
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
#define PROJ_SHM_MAILBOX_OFFSET 8 // ����� mailbox �����ڴ�ƫ����
#define PROJ_DIRECTION_TUNNEL_OFFSET 9
#define PROJ_ZERO_CAR_OFFSET 10

#define PROJ_SHM_TUNNEL_OFFSET 11

extern int total_number_of_cars;
extern int maximum_number_of_cars_in_tunnel;//��������������
enum class Direction;
class Car;
// ����ɹ�������ݽṹ
struct SharedTunnelData {
    Direction current_direction_;
    int car_count_;
    // �������������Ҫ����Ļ����������ͳ�Ա
};

class Tunnel {
public:
    Tunnel(int proj_id, const char *pathname);
    ~Tunnel();
//    void enter(Car *car);
//    void leave(Car *car);
    void show(); // ���� show ��������
//    void main_process();
public:
//    ��������г�����
    int tunnel_number_of_cars;
//    ���Ƴ��ܵ�����
    int total_number_of_cars_tunnel;
    int car_count_;               // ����г�������
    key_t mutex_key;
    key_t block_key;
    key_t car_count_key;
    key_t direction_changed_key;
    key_t zero_car_key;
    int mutex_;    // ���������ڲ�״̬
    int block_;    // �����������������ĳ�
    int direction_changed_;    // �������������Ϸ���ĳ�
    int zero_car_;    // ������ʶ�����Ƿ�Ϊ0
    Direction current_direction_; // ��ǰ����г��ķ���
};



#endif //LINUX_HW_TUNNEL_H
