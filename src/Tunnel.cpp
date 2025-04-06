//
// Created by elysia on 2025/4/5.
//

#include "../include/Tunnel.h"

void Tunnel::enter(Car &car) {
    Wait(mutex_, 0); // 加锁

    if (car_count_ == 0) {
        // 隧道里没有车，设置当前方向
        current_direction_ = car.direction_;
        car_count_++;

        Logger::log(LogLevel::INFO, "Car " + std::to_string(car.car_id_) +
                                    " entering tunnel in direction " + std::to_string(static_cast<int>(car.direction_)) +
                                    " (empty tunnel).");

        Signal(mutex_, 0); // 解锁
        return;
    }

    if (current_direction_ == car.direction_) {
        // 方向一致，可以直接进
        car_count_++;
        Logger::log(LogLevel::INFO, "Car " + std::to_string(car.car_id_) +
                    " entering tunnel in direction " + std::to_string(static_cast<int>(car.direction_)) +
                    " (same direction).");

        Signal(mutex_, 0); // 解锁
    } else {
        // 方向不同，等到当前方向的车全部走完
        Logger::log(LogLevel::INFO,  "Car " + std::to_string(car.car_id_) +
                    " waiting to enter tunnel (direction " + std::to_string(static_cast<int>(car.direction_)) +
                    "), tunnel occupied by direction " + std::to_string(static_cast<int>(current_direction_)) + ".");

        Signal(mutex_, 0); // 先解锁，避免死锁

        Wait(block_, 0); // 阻塞，直到被唤醒

        // 被唤醒后再进隧道
        Wait(mutex_, 0);
        current_direction_ = car.direction_;
        car_count_++;
        Logger::log(LogLevel::INFO, "Car " + std::to_string(car.car_id_) +
                    " entering tunnel after wait, new direction " + std::to_string(static_cast<int>(car.direction_)) + ".");
        Signal(mutex_, 0);
    }
}

void Tunnel::leave(Car &car) {
    Wait(mutex_, 0); // 加锁

    car_count_--;
    Logger::log(LogLevel::INFO, "Car " + std::to_string(car.car_id_) +
                " leaving tunnel, remaining cars: " + std::to_string(car_count_) + ".");

    if (car_count_ == 0) {
        // 隧道清空，唤醒等待的车
        Signal(block_, 0);
    }

    Signal(mutex_, 0); // 解锁
}
