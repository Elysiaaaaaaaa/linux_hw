//
// Created by elysia on 2025/4/5.
//

#include "../include/Tunnel.h"

extern int maximum_number_of_cars_in_tunnel;
Tunnel::Tunnel(int mutex_sid, int block_sid)
        : mutex_(mutex_sid), block_(block_sid), car_count_(0) {
//    ���ÿ��Ƴ��������ź���
    semid_tunnel_car = sem_get(IPC_PRIVATE, 1, true, maximum_number_of_cars_in_tunnel);
}
void Tunnel::enter(Car &car) {
    Wait(mutex_, 0); // ����

    while (true) {
        if (car_count_ == 0) {
            // �����û�г������÷��򲢽���
            current_direction_ = car.direction_;
            car_count_++;
            Logger::log(LogLevel::INFO, "Car " + std::to_string(car.car_id_) +
                                        " entering tunnel in direction " + std::to_string(static_cast<int>(car.direction_)) +
                                        " (empty tunnel).");
            Signal(mutex_, 0); // ����
            return;
        }

        if (current_direction_ == car.direction_ && car_count_ < maximum_number_of_cars_in_tunnel) {
            // ����һ����δ�������Խ���
            car_count_++;
            Logger::log(LogLevel::INFO, "Car " + std::to_string(car.car_id_) +
                                        " entering tunnel in direction " + std::to_string(static_cast<int>(car.direction_)) +
                                        " (same direction, space available).");
            Signal(mutex_, 0); // ����
            return;
        }

        // ����ͬ������������ȴ�
        if (current_direction_ != car.direction_) {
            Logger::log(LogLevel::INFO,  "Car " + std::to_string(car.car_id_) +
                                         " waiting due to opposite direction (direction " + std::to_string(static_cast<int>(car.direction_)) +
                                         "), tunnel occupied by direction " + std::to_string(static_cast<int>(current_direction_)) + ".");
        } else {
            Logger::log(LogLevel::INFO,  "Car " + std::to_string(car.car_id_) +
                                         " waiting because tunnel full (direction " + std::to_string(static_cast<int>(car.direction_)) +
                                         ", cars in tunnel: " + std::to_string(car_count_) + ").");
        }

        Signal(mutex_, 0); // ������������
        Wait(block_, 0);   // �������ȴ������仯
        Wait(mutex_, 0);   // �ٴμ��������¼������
    }
}


void Tunnel::leave(Car &car) {
    Wait(mutex_, 0); // ����

    car_count_--;
    Logger::log(LogLevel::INFO, "Car " + std::to_string(car.car_id_) +
                                " leaving tunnel, remaining cars: " + std::to_string(car_count_) + ".");

    if (car_count_ < maximum_number_of_cars_in_tunnel) {
        // ���δ�������Ի���ͬ����ĵȴ���
        Signal(block_, 0);
    }

    Signal(mutex_, 0); // ����
}