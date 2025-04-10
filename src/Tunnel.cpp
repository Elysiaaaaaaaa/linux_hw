//
// Created by elysia on 2025/4/5.
//

#include "../include/Tunnel.h"

extern int maximum_number_of_cars_in_tunnel;
Tunnel::Tunnel(int mutex_sid, int block_sid)
        : mutex_(mutex_sid), block_(block_sid), car_count_(0) {
//    设置控制车数量的信号量
    semid_tunnel_car = sem_get(IPC_PRIVATE, 1, true, maximum_number_of_cars_in_tunnel);
}
void Tunnel::enter(Car &car) {
    Wait(mutex_, 0); // 加锁

    while (true) {
        if (car_count_ == 0) {
            // 隧道里没有车，设置方向并进入
            current_direction_ = car.direction_;
            car_count_++;
            Logger::log(LogLevel::INFO, "Car " + std::to_string(car.car_id_) +
                                        " entering tunnel in direction " + std::to_string(static_cast<int>(car.direction_)) +
                                        " (empty tunnel).");
            Signal(mutex_, 0); // 解锁
            return;
        }

        if (current_direction_ == car.direction_ && car_count_ < maximum_number_of_cars_in_tunnel) {
            // 方向一致且未满，可以进入
            car_count_++;
            Logger::log(LogLevel::INFO, "Car " + std::to_string(car.car_id_) +
                                        " entering tunnel in direction " + std::to_string(static_cast<int>(car.direction_)) +
                                        " (same direction, space available).");
            Signal(mutex_, 0); // 解锁
            return;
        }

        // 方向不同或隧道已满，等待
        if (current_direction_ != car.direction_) {
            Logger::log(LogLevel::INFO,  "Car " + std::to_string(car.car_id_) +
                                         " waiting due to opposite direction (direction " + std::to_string(static_cast<int>(car.direction_)) +
                                         "), tunnel occupied by direction " + std::to_string(static_cast<int>(current_direction_)) + ".");
        } else {
            Logger::log(LogLevel::INFO,  "Car " + std::to_string(car.car_id_) +
                                         " waiting because tunnel full (direction " + std::to_string(static_cast<int>(car.direction_)) +
                                         ", cars in tunnel: " + std::to_string(car_count_) + ").");
        }

        Signal(mutex_, 0); // 解锁避免死锁
        Wait(block_, 0);   // 阻塞，等待条件变化
        Wait(mutex_, 0);   // 再次加锁后重新检查条件
    }
}


void Tunnel::leave(Car &car) {
    Wait(mutex_, 0); // 加锁

    car_count_--;
    Logger::log(LogLevel::INFO, "Car " + std::to_string(car.car_id_) +
                                " leaving tunnel, remaining cars: " + std::to_string(car_count_) + ".");

    if (car_count_ < maximum_number_of_cars_in_tunnel) {
        // 隧道未满，可以唤醒同方向的等待车
        Signal(block_, 0);
    }

    Signal(mutex_, 0); // 解锁
}