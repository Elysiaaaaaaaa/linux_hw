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
extern int total_number_of_cars;
extern int maximum_number_of_cars_in_tunnel;//��������������
enum class Direction;
class Car;
class Tunnel {
private:

public:
    Tunnel(int proj_id, const char *pathname);
    void init_car(txt_reader& reader);
    void enter(Car *car);
    void leave(Car *car);
    void main_process();
public:
//    ��������г�����
    int tunnel_number_of_cars;
//    ���Ƴ��ܵ�����
    int total_number_of_cars_tunnel;
    int car_count_;               // ����г�������
    key_t mutex_key;
    key_t block_key;
    key_t car_count_key;
    std::vector<Car> cars;
    int mutex_;    // ���������ڲ�״̬
    int block_;    // �������������Ϸ���ĳ�
    Direction current_direction_; // ��ǰ����г��ķ���
};



#endif //LINUX_HW_TUNNEL_H
