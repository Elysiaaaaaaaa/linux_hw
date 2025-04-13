//
// Created by elysia on 2025/4/5.
//

#include "../include/Tunnel.h"

extern int maximum_number_of_cars_in_tunnel;
Tunnel::Tunnel(int proj_id, const char *pathname){
    // 生成 IPC 键
    cars.reserve(total_number_of_cars); // 预分配足够的内存
    mutex_key = Ftok(proj_id, pathname);
    block_key = Ftok(proj_id + 1, pathname);
    // 使用不同的 proj_id 以确保不同的键

    // 创建或获取用于tunnel保护内部状态的信号量集，初始值为 1
    mutex_ = sem_get(mutex_key, 1, true, 1);

    // 创建或获取用于阻塞不符合方向的车的信号量集，初始值为 0
    block_ = sem_get(block_key, 1, true, 0);
//    设置控制车数量的信号量，传入Car内部
    this->tunnel_number_of_cars = sem_get(IPC_PRIVATE, 1, true, maximum_number_of_cars_in_tunnel);
    this->total_number_of_cars_tunnel = sem_get(IPC_PRIVATE, 1, true, total_number_of_cars);
    car_count_ = 0;
}


void Tunnel::init_car(txt_reader& reader){
    int idx;
    int direct;
    reader.buf >> idx >> direct;
    cout<<idx<<' '<<direct<<endl;
    cars.emplace_back(idx, static_cast<Direction>(direct), reader);
}


void Tunnel::enter(Car *car) {
    Wait(mutex_, 0); // 加锁
    while (true) {
        cout<<car_count_<<endl;
        if (car_count_ == 0) {
            // 隧道里没有车，设置方向并进入
            current_direction_ = car->direction_;
            this->car_count_++;
            car->start_time = time(0);
            car->state = State::INNER;
            Logger::log(LogLevel::INFO, "Car " + std::to_string(car->car_id) +
                                        " entering tunnel in direction " + std::to_string(static_cast<int>(car->direction_)) +
                                        " (empty tunnel).");
            Signal(mutex_, 0); // 解锁
            return;
        }else if (current_direction_ == car->direction_ && car_count_ < maximum_number_of_cars_in_tunnel) {
            // 方向一致且未满，可以进入
            car_count_++;
            car->start_time = time(0);
            car->state = State::INNER;
            Logger::log(LogLevel::INFO, "Car " + std::to_string(car->car_id) +
                                        " entering tunnel in direction " + std::to_string(static_cast<int>(car->direction_)) +
                                        " (same direction, space available).");
            Signal(mutex_, 0); // 解锁
            return;
        }else if (current_direction_ != car->direction_) {
            // 方向不同，等待
            Logger::log(LogLevel::INFO, "Car " + std::to_string(car->car_id) +
                                        " waiting due to opposite direction (direction " + std::to_string(static_cast<int>(car->direction_)) +
                                        "), tunnel occupied by direction " + std::to_string(static_cast<int>(current_direction_)) + ".");
        } else {
            // 隧道已满，等待
            Logger::log(LogLevel::INFO, "Car " + std::to_string(car->car_id) +
                                        " waiting because tunnel full (direction " + std::to_string(static_cast<int>(car->direction_)) +
                                        ", cars in tunnel: " + std::to_string(car_count_) + ").");
        }

        Wait(block_, mutex_);
    }
}


void Tunnel::leave(Car *car) {
    Wait(mutex_, 0); // 加锁

    car_count_--;
    car->state = State::OUT;

    Logger::log(LogLevel::INFO, "Car " + std::to_string(car->car_id) +
                                " leaving tunnel, remaining cars: " + std::to_string(car_count_) + ".");

    if (car_count_ < maximum_number_of_cars_in_tunnel) {
        // 隧道未满，可以唤醒同方向的等待车
        Signal(block_, 0);
    }

    Signal(mutex_, 0); // 解锁
}

void Tunnel::main_process() {
    Logger::log(LogLevel::INFO, "TUNNEL BEGAN");

    pid_t id;
    int i = 0;

    // 为每辆车创建一个子进程
    for (; i < total_number_of_cars; ++i) {
        usleep(1000000);
        id = Fork();
        if (id == 0) {
            // 子进程
            break;
        }
    }

    if (id == 0) {
        // 子进程逻辑
//        Wait(total_number_of_cars_tunnel, 0);   // 等待信号量资源

        cars[i].main_process(
                tunnel_number_of_cars,
                this); // 车辆主流程

//        Signal(total_number_of_cars_tunnel, 0); // 完成后释放信号量
        exit(0); // 子进程完成后退出，避免继续执行父进程代码
    } else {
        // 父进程逻辑

        // 等待所有子进程退出
        int status;
        for (int j = 0; j < total_number_of_cars; ++j) {
            wait((int*)0);
        }

        Logger::log(LogLevel::INFO, "TUNNEL FINISH");
    }
}
