#include "process.h"

#define PROJ_SHM_MAILBOX_OFFSET 8 // 假设的 mailbox 共享内存偏移量

process::process(int num_mailboxes, int mem_size, int proj_id, const char *pathname) {
    // 获取 Tunnel 对象的共享内存 key
    key_t shm_key_tunnel = ftok(pathname, proj_id + PROJ_SHM_TUNNEL_OFFSET);
    int shmid_tunnel = shmget(shm_key_tunnel, sizeof(Tunnel), IPC_CREAT | 0666);
    if (shmid_tunnel == -1) {
        perror("shmget for tunnel failed");
        exit(EXIT_FAILURE);
    }

    // 共享内存 attach
    void *shmaddr_tunnel = shmat(shmid_tunnel, nullptr, 0);
    if (shmaddr_tunnel == (void *)-1) {
        perror("shmat for tunnel failed");
        exit(EXIT_FAILURE);
    }

    // 使用 placement new 构造 Tunnel 实例在共享内存上
    tunnel = new (shmaddr_tunnel) Tunnel(proj_id, pathname);

    // 获取 mailbox 对象的共享内存 key
    key_t shm_key_mailbox = ftok(pathname, proj_id + PROJ_SHM_MAILBOX_OFFSET);
    int shmid_mailbox = shmget(shm_key_mailbox, sizeof(mailbox), IPC_CREAT | 0666);
    if (shmid_mailbox == -1) {
        perror("shmget for mailbox failed");
        exit(EXIT_FAILURE);
    }

    // 共享内存 attach
    void *shmaddr_mailbox = shmat(shmid_mailbox, nullptr, 0);
    if (shmaddr_mailbox == (void *)-1) {
        perror("shmat for mailbox failed");
        exit(EXIT_FAILURE);
    }

    // 使用 placement new 构造 mailbox 实例在共享内存上
    mail_box = new (shmaddr_mailbox) mailbox(num_mailboxes, mem_size, proj_id, pathname);

    // 初始化车辆数组（不共享也可）
    cars.reserve(total_number_of_cars);
}

process::~process() {
    // 析构 Tunnel 对象
    tunnel->~Tunnel();
    if (shmdt(tunnel) == -1) {
        perror("shmdt for tunnel failed");
    }

    // 析构 mailbox 对象
    mail_box->~mailbox();
    if (shmdt(mail_box) == -1) {
        perror("shmdt for mailbox failed");
    }
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
            car->start_time = std::chrono::high_resolution_clock::now();
            car->state = State::INNER;
            Logger::log(LogLevel::INFO, "Car " + std::to_string(car->car_id) +
                                        " entering tunnel in direction " + std::to_string(static_cast<int>(car->direction_)) +
                                        " (empty tunnel).");
            break;
        } else if (tunnel->current_direction_ == car->direction_ && tunnel->car_count_ < maximum_number_of_cars_in_tunnel) {
            // 同一方向且未达到最大容量，进入
            (tunnel->car_count_)++;
            car->start_time = std::chrono::high_resolution_clock::now();
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

    if (tunnel->car_count_ < maximum_number_of_cars_in_tunnel) {
        // 隧道未满，可以唤醒同方向的等待车
        Signal(tunnel->block_, 0);
    }

    Signal(tunnel->mutex_, 0); // 解锁
}

void process::main_process(){
    start_time = std::chrono::high_resolution_clock::now(); // 记录起始时间
    Logger::setBaseTime(start_time);
    Logger::log(LogLevel::INFO, "PROCESS BEGAN");

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t id;
    int i = 0;
    // 为每辆车创建一个子进程
    for (; i < total_number_of_cars; ++i) {
        id = Fork();
        if (id == 0) {
            // 子进程
            close(pipefd[0]); // 关闭读端
            break;
        }
//        sleep(1);
    }

    if (id == 0) {
        // 子进程逻辑
//        Wait(total_number_of_cars_tunnel, 0);   // 等待信号量资源
        enter(&cars[i]);
        tunnel->show();
//        sleep(1);
//        隧道内的每辆车都可以访问和修改一个用以模拟隧道邮箱系统的共享内存段（可以看成是一
//        个数组，访问操作操作包括：r和w），这样，隧道内的车辆就在进隧道后保持其手机通讯（隧
//        道将阻塞手机信号）。隧道外的汽车则不需要访问该共享内存段。
        int t = 0;
        for (const auto& op : cars[i].operations) {
            t++;
            auto current_time = std::chrono::high_resolution_clock::now();
            auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(
                    current_time - start_time).count();
            if (elapsed_time > op.time) {
                if(op.isWrite){
                    Logger::log(LogLevel::WARN,
                                "Car " + std::to_string(cars[i].car_id) + " has timed out for write " + to_string(op.mailbox));
                }else{
                    Logger::log(LogLevel::WARN,
                                "Car " + std::to_string(cars[i].car_id) + " has timed out for read "+to_string(op.mailbox));
                }
            } else {
//                auto remaining_time = op.time - elapsed_time;
//                if (remaining_time > 10) {
//                    usleep((remaining_time - 10) * 1000);
//                }
                Logger::log(LogLevel::WARN,
                            "Car " + std::to_string(cars[i].car_id) + " lock " + to_string(op.mailbox));
                if (op.isWrite) {
                    mail_box->writeMailbox(op.mailbox - 1, op.data, op.time, start_time);
//                    std::cout << "  Write operation: "
//                              << "Data: " << op.data << ", "
//                              << "Time: " << op.time << ", "
//                              << "Mailbox: " << op.mailbox << ", "
//                              << "Length: " << op.length << std::endl;
                } else {
                    std::string readResult;
                    mail_box->readMailbox(op.mailbox - 1, readResult, op.time, start_time);
                    std::cout << "!!!!!!"<<readResult<<readResult.length() << std::endl;
                    cars[i].model_str = cars[i].model_str + readResult;
                    std::cout << "!!!---!!!"<<cars[i].model_str<<readResult.length() << std::endl;
                }
                Logger::log(LogLevel::WARN,
                            "Car " + std::to_string(cars[i].car_id) + " unlock " + to_string(op.mailbox));
            }
        }

        // 计算预计结束时间
        auto end_time = cars[i].start_time + cars[i].cost_time;

        // 获取当前时间
        auto current_time = std::chrono::high_resolution_clock::now();

        // 计算需要睡眠的时间
        auto sleep_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - current_time);

        // 如果需要睡眠（即当前时间还未到预计结束时间）
        if (sleep_duration.count() > 0) {
            // 使用 std::this_thread::sleep_for 进行阻塞（usleep 不支持 std::chrono 类型，这里使用 std::this_thread::sleep_for）
            usleep(sleep_duration.count()*1000);
        }
        leave(&cars[i]);

        // 发送数据到管道
        const std::string& data = cars[i].model_str;
        write(pipefd[1], data.c_str(), data.size());
        close(pipefd[1]);

//        Logger::log(LogLevel::INFO,cars[i].model_str);
//        Signal(total_number_of_cars_tunnel, 0); // 完成后释放信号量
        exit(0); // 子进程完成后退出，避免继续执行父进程代码
    } else {
        // 关闭写端
        close(pipefd[1]);

        // 等待所有子进程退出
        for (int j = 0; j < total_number_of_cars; ++j) {
            wait((int*)0);
        }

        // 从管道读取数据
        for (int i = 0; i < total_number_of_cars; ++i) {
            char buffer[1024];
            ssize_t n = read(pipefd[0], buffer, sizeof(buffer));
            if (n > 0) {
                buffer[n] = '\0';
                cars[i].model_str = buffer;
            }
        }
        close(pipefd[0]);

        for (int i = 0; i < total_number_of_cars; ++i) {
            Logger::log(LogLevel::INFO, "Car " + std::to_string(cars[i].car_id) +
                                        " leaving tunnel,"+" Reader: " + cars[i].model_str + ".");
        }
        mail_box->show();

        Logger::log(LogLevel::INFO, "PROCESS FINISH");
    }
}