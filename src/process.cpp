//
// Created by elysia on 2025/4/13.
//

#include "process.h"
process::process(int proj_id, const char *pathname) {
    // 获取 Tunnel 对象的共享内存 key
    key_t shm_key = ftok(pathname, proj_id + PROJ_SHM_TUNNEL_OFFSET);
    int shmid = shmget(shm_key, sizeof(Tunnel), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget failed");
        exit(EXIT_FAILURE);
    }

    // 共享内存 attach
    void *shmaddr = shmat(shmid, nullptr, 0);
    if (shmaddr == (void *)-1) {
        perror("shmat failed");
        exit(EXIT_FAILURE);
    }

    // 使用 placement new 构造 Tunnel 实例在共享内存上
    tunnel = new (shmaddr) Tunnel(proj_id, pathname);

    // 初始化车辆数组（不共享也可）
    std::vector<Car> cars;
    cars.reserve(total_number_of_cars);
}



process::~process() {
    // 销毁互斥锁
    delete tunnel;
}
void process::init_car(txt_reader& reader) {
    int idx;
    int direct;
    reader.buf >> idx >> direct;
    cars.emplace_back(idx, static_cast<Direction>(direct), reader);

}
void process::enter(Car *car){
    Wait(tunnel->mutex_, 0); // 获取信号量
    while (true) {
        if (tunnel->car_count_ == 0) {
            // 隧道没有车，设置方向并进入
            tunnel->current_direction_ = car->direction_;
            tunnel->car_count_ += 1;
            car->start_time = time(0);
            car->state = State::INNER;
            Logger::log(LogLevel::INFO, "Car " + std::to_string(car->car_id) +
                                        " entering tunnel in direction " + std::to_string(static_cast<int>(car->direction_)) +
                                        " (empty tunnel).");
            break;
        } else if (tunnel->current_direction_ == car->direction_ && tunnel->car_count_ < maximum_number_of_cars_in_tunnel) {
            // 同一方向且未达到最大容量，进入
            (tunnel->car_count_)++;
            car->start_time = time(0);
            car->state = State::INNER;
            Logger::log(LogLevel::INFO, "Car " + std::to_string(car->car_id) +
                                        " entering tunnel in direction " + std::to_string(static_cast<int>(car->direction_)) +
                                        " (same direction, space available).");
            break;
        } else if (tunnel->current_direction_ != car->direction_) {
            // 方向不同，等待
            Logger::log(LogLevel::WARN, "Car " + std::to_string(car->car_id) +
                                        " waiting due to opposite direction (direction " + std::to_string(static_cast<int>(car->direction_)) +
                                        "), tunnel occupied by direction " + std::to_string(static_cast<int>(tunnel->current_direction_)) + ".");
        } else {
            // 隧道已满，等待
            Logger::log(LogLevel::WARN, "Car " + std::to_string(car->car_id) +
                                        " waiting because tunnel full (direction " + std::to_string(static_cast<int>(car->direction_)) +
                                        ", cars in tunnel: " + std::to_string(tunnel->car_count_) + ").");
        }
        Signal(tunnel->mutex_, 0); // 释放信号量
        Wait(tunnel->block_, 0); // 等待阻塞信号量
        Wait(tunnel->mutex_, 0); // 再次获取信号量
    }
    Signal(tunnel->mutex_, 0); // 释放信号量
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
//        sleep(1);
    }

    if (id == 0) {
        // 子进程逻辑
//        Wait(total_number_of_cars_tunnel, 0);   // 等待信号量资源
        enter(&cars[i]);
        tunnel->show();
//        隧道内的每辆车都可以访问和修改一个用以模拟隧道邮箱系统的共享内存段（可以看成是一
//        个数组，访问操作操作包括：r和w），这样，隧道内的车辆就在进隧道后保持其手机通讯（隧
//        道将阻塞手机信号）。隧道外的汽车则不需要访问该共享内存段。

        for (const auto& op : cars[i].operations) {
            if (op.isWrite) {
                std::cout << "  Write operation: "
                          << "Data: " << op.data << ", "
                          << "Time: " << op.time << ", "
                          << "Mailbox: " << op.mailbox << ", "
                          << "Length: " << op.length << std::endl;
            } else {
                std::cout << "  Read operation: "
                          << "Time: " << op.time << ", "
                          << "Mailbox: " << op.mailbox << ", "
                          << "Length: " << op.length << std::endl;
            }
        }


        leave(&cars[i]);


//        Signal(total_number_of_cars_tunnel, 0); // 完成后释放信号量
        exit(0); // 子进程完成后退出，避免继续执行父进程代码
    } else {
        // 父进程逻辑

        // 等待所有子进程退出
        for (int j = 0; j < total_number_of_cars; ++j) {
            wait((int*)0);
        }

        Logger::log(LogLevel::INFO, "PROCESS FINISH");
    }
}
