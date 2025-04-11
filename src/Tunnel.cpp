//
// Created by elysia on 2025/4/5.
//

#include "../include/Tunnel.h"

extern int maximum_number_of_cars_in_tunnel;
Tunnel::Tunnel(int proj_id, const char *pathname, std::vector<Car>& cars):cars(cars) {
    // 生成 IPC 键
    key_t mutex_key = Ftok(proj_id, pathname);
    key_t block_key = Ftok(proj_id + 1, pathname);
    // 使用不同的 proj_id 以确保不同的键

    // 创建或获取用于tunnel保护内部状态的信号量集，初始值为 1
    mutex_ = sem_get(mutex_key, 1, true, 1);

    // 创建或获取用于阻塞不符合方向的车的信号量集，初始值为 0
    block_ = sem_get(block_key, 1, true, 0);
//    设置控制车数量的信号量，传入Car内部
    semid_tunnel_car = sem_get(IPC_PRIVATE, 1, true, maximum_number_of_cars_in_tunnel);
    car_count_ = 0;
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
        }else if (current_direction_ == car.direction_ && car_count_ < maximum_number_of_cars_in_tunnel) {
            // 方向一致且未满，可以进入
            car_count_++;
            Logger::log(LogLevel::INFO, "Car " + std::to_string(car.car_id_) +
                                        " entering tunnel in direction " + std::to_string(static_cast<int>(car.direction_)) +
                                        " (same direction, space available).");
            Signal(mutex_, 0); // 解锁
            return;
        }else if (current_direction_ != car.direction_) {
            // 方向不同，等待
            Logger::log(LogLevel::INFO,  "Car " + std::to_string(car.car_id_) +
                                         " waiting due to opposite direction (direction " + std::to_string(static_cast<int>(car.direction_)) +
                                         "), tunnel occupied by direction " + std::to_string(static_cast<int>(current_direction_)) + ".");
        } else {
            // 隧道已满，等待
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

void Tunnel::main_process(){
    Logger::log(LogLevel::INFO,"TUNNEL BEGAN");
//    隧道主进程，用来操作车辆
    pid_t id;
    int i = 0;
    for(; i<total_number_of_cars; i++){
//        为每辆车创建进程
        id = fork();
        cout<<id<<"|"<<endl;
        usleep(100);
        if(int(id)==0){
            cout<<"11111111111"<<endl;
//            子进程（车辆 启动
            cars[i].main_process();
            break;
        }
    }
    usleep(100);
    sleep(2);
    cout<<sem_get_val(semid_tunnel_car)<<endl;
    while (sem_get_val(semid_tunnel_car)<maximum_number_of_cars_in_tunnel){
//        车辆还没有全走完
        usleep(100);
    }
    usleep(100);
    Logger::log(LogLevel::INFO,"TUNNEL FINISH");

}