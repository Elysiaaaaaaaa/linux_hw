//
// Created by elysia on 2025/4/5.
//

#include "../include/Tunnel.h"

void Tunnel::enter(Car &car) {
    Wait(mutex_, 0); // ����

    if (car_count_ == 0) {
        // �����û�г������õ�ǰ����
        current_direction_ = car.direction_;
        car_count_++;

        Logger::log(LogLevel::INFO, "Car " + std::to_string(car.car_id_) +
                                    " entering tunnel in direction " + std::to_string(static_cast<int>(car.direction_)) +
                                    " (empty tunnel).");

        Signal(mutex_, 0); // ����
        return;
    }

    if (current_direction_ == car.direction_) {
        // ����һ�£�����ֱ�ӽ�
        car_count_++;
        Logger::log(LogLevel::INFO, "Car " + std::to_string(car.car_id_) +
                    " entering tunnel in direction " + std::to_string(static_cast<int>(car.direction_)) +
                    " (same direction).");

        Signal(mutex_, 0); // ����
    } else {
        // ����ͬ���ȵ���ǰ����ĳ�ȫ������
        Logger::log(LogLevel::INFO,  "Car " + std::to_string(car.car_id_) +
                    " waiting to enter tunnel (direction " + std::to_string(static_cast<int>(car.direction_)) +
                    "), tunnel occupied by direction " + std::to_string(static_cast<int>(current_direction_)) + ".");

        Signal(mutex_, 0); // �Ƚ�������������

        Wait(block_, 0); // ������ֱ��������

        // �����Ѻ��ٽ����
        Wait(mutex_, 0);
        current_direction_ = car.direction_;
        car_count_++;
        Logger::log(LogLevel::INFO, "Car " + std::to_string(car.car_id_) +
                    " entering tunnel after wait, new direction " + std::to_string(static_cast<int>(car.direction_)) + ".");
        Signal(mutex_, 0);
    }
}

void Tunnel::leave(Car &car) {
    Wait(mutex_, 0); // ����

    car_count_--;
    Logger::log(LogLevel::INFO, "Car " + std::to_string(car.car_id_) +
                " leaving tunnel, remaining cars: " + std::to_string(car_count_) + ".");

    if (car_count_ == 0) {
        // �����գ����ѵȴ��ĳ�
        Signal(block_, 0);
    }

    Signal(mutex_, 0); // ����
}
