//
// Created by elysia on 2025/4/5.
//

#ifndef LINUX_HW_TUNNEL_H
#define LINUX_HW_TUNNEL_H


#include "Car.h"
#include "ipc.h"   // ���Լ��е��ź���ͷ�ļ�
#include "mp.h"
#include "Logger.h"
#include <vector>
#include <cstdio>
#include <unistd.h>

class Tunnel {
private:
    int mutex_;    // ���������ڲ�״̬
    int block_;    // �������������Ϸ���ĳ�
    Direction current_direction_; // ��ǰ����г��ķ���
    int car_count_;               // ����г�������
public:
    Tunnel(int mutex_sid, int block_sid);
    void enter(Car &car);
    void leave(Car &car);
public:
//    �������������
    int semid_tunnel_car;
};



#endif //LINUX_HW_TUNNEL_H
