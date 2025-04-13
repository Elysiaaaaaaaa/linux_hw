//
// Created by elysia on 2025/4/13.
//

#include "process.h"
process::process(int proj_id, const char *pathname){
    tunnel = new Tunnel(proj_id,pathname);
    std::vector<Car> cars;
    cars.reserve(total_number_of_cars); // 预分配足够的内存

}

void process::init_car(txt_reader& reader){
    int idx;
    int direct;
    reader.buf >> idx >> direct;
    cars.emplace_back(idx, static_cast<Direction>(direct), reader);
}

void process::enter(Car *car){
    Wait(tunnel->mutex_, 0); // 加锁
    while (true) {
//        cout<<car_count_<<endl;
        if (tunnel->car_count_ == 0) {
            // 隧道里没有车，设置方向并进入
            tunnel->current_direction_ = car->direction_;
            tunnel->car_count_ += 1;
            car->start_time = time(0);
            car->state = State::INNER;
            Logger::log(LogLevel::INFO, "Car " + std::to_string(car->car_id) +
                                        " entering tunnel in direction " + std::to_string(static_cast<int>(car->direction_)) +
                                        " (empty tunnel).");
            break;
        }else if (tunnel->current_direction_ == car->direction_ && tunnel->car_count_ < maximum_number_of_cars_in_tunnel) {
            // 方向一致且未满，可以进入
            (tunnel->car_count_)++;
            car->start_time = time(0);
            car->state = State::INNER;
            Logger::log(LogLevel::INFO, "Car " + std::to_string(car->car_id) +
                                        " entering tunnel in direction " + std::to_string(static_cast<int>(car->direction_)) +
                                        " (same direction, space available).");
            break;
        }else if (tunnel->current_direction_ != car->direction_) {
            // 方向不同，等待
            Logger::log(LogLevel::INFO, "Car " + std::to_string(car->car_id) +
                                        " waiting due to opposite direction (direction " + std::to_string(static_cast<int>(car->direction_)) +
                                        "), tunnel occupied by direction " + std::to_string(static_cast<int>(tunnel->current_direction_)) + ".");
        } else {
            // 隧道已满，等待
            Logger::log(LogLevel::INFO, "Car " + std::to_string(car->car_id) +
                                        " waiting because tunnel full (direction " + std::to_string(static_cast<int>(car->direction_)) +
                                        ", cars in tunnel: " + std::to_string(tunnel->car_count_) + ").");
        }
        Signal(tunnel->mutex_,0);
        Wait(tunnel->block_,0);
        Wait(tunnel->mutex_,0);
    }
    Signal(tunnel->mutex_, 0); // 解锁
}
void process::leave(Car *car){
    Wait(tunnel->mutex_, 0); // 加锁

    (tunnel->car_count_)--;
    car->state = State::OUT;

    Logger::log(LogLevel::INFO, "Car " + std::to_string(car->car_id) +
                                " leaving tunnel, remaining cars: " + std::to_string(tunnel->car_count_) + ".");

    if (tunnel->car_count_ < maximum_number_of_cars_in_tunnel) {
        // 隧道未满，可以唤醒同方向的等待车
        Signal(tunnel->block_, 0);
    }

    Signal(tunnel->mutex_, 0); // 解锁
}
void process::main_process(){
    Logger::log(LogLevel::INFO, "PROCESS BEGAN");

    pid_t id;
    int i = 0;

    // 为每辆车创建一个子进程
    for (; i < total_number_of_cars; ++i) {
        id = Fork();
        if (id == 0) {
            // 子进程
            break;
        }
        sleep(1);
    }

    if (id == 0) {
        // 子进程逻辑
//        Wait(total_number_of_cars_tunnel, 0);   // 等待信号量资源
        enter(&cars[i]);
        sleep(3);
        leave(&cars[i]);
//        cars[i].main_process(
//                tunnel->tunnel_number_of_cars,
//                *tunnel); // 车辆主流程

//        Signal(total_number_of_cars_tunnel, 0); // 完成后释放信号量
        exit(0); // 子进程完成后退出，避免继续执行父进程代码
    } else {
        // 父进程逻辑

        // 等待所有子进程退出
        int status;
        for (int j = 0; j < total_number_of_cars; ++j) {
            wait((int*)0);
        }

        Logger::log(LogLevel::INFO, "PROCESS FINISH");
    }
}
