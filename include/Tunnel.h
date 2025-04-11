//
// Created by elysia on 2025/4/5.
//

#ifndef LINUX_HW_TUNNEL_H
#define LINUX_HW_TUNNEL_H


#include "Car.h"
#include "ipc.h"   // ���Լ��е��ź���ͷ�ļ�
#include "mp.h"
#include "Logger.h"
#include <stdio.h>
#include <unistd.h>
#include <vector>
#include <cstdio>
#include <unistd.h>
extern int total_number_of_cars;
class Tunnel {
private:
    int mutex_;    // ���������ڲ�״̬
    int block_;    // �������������Ϸ���ĳ�
    Direction current_direction_; // ��ǰ����г��ķ���
    int car_count_;               // ����г�������
public:
    Tunnel(int proj_id, const char *pathname, std::vector<Car>& cars);
    void enter(Car &car);
    void leave(Car &car);
    void main_process();
public:
//    �������������
    int semid_tunnel_car;
    std::vector<Car> cars;
};



#endif //LINUX_HW_TUNNEL_H
