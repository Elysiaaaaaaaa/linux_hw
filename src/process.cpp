#include "process.h"


process::process(int switch_time,int num_mailboxes, int mem_size, int proj_id, const char *pathname) {
    this->switch_time = switch_time;
    // 获取 Tunnel 对象的共享内存 key
    key_t shm_key_tunnel = ftok(pathname, proj_id + PROJ_SHM_TUNNEL_OFFSET);
    int shmid_tunnel = shmget(shm_key_tunnel, sizeof(Tunnel), IPC_CREAT | 0666);
    if (shmid_tunnel == -1) {
        Logger::log(LogLevel::WARN,"shmget for tunnel failed");
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

    tunnel->current_direction_ = Direction::Eastbound;

    Signal(tunnel->direction_changed_,0);

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


void process::enter(Car *car) {
    if(!use_rg) {
//        无红绿灯
        Wait(tunnel->mutex_, 0); // 获取信号量
        while (true) {
            if (tunnel->car_count_ == 0) {
                // 隧道没有车，设置方向并进入
                tunnel->current_direction_ = car->direction_;
                tunnel->car_count_ += 1;
                car->start_time = std::chrono::high_resolution_clock::now();
                car->state = State::INNER;
                Logger::log(LogLevel::INFO, "Car " + std::to_string(car->car_id) +
                                            " entering tunnel in direction " +
                                            std::to_string(static_cast<int>(car->direction_)) +
                                            " (empty tunnel).");
                break;
            } else if (tunnel->current_direction_ == car->direction_ &&
                       tunnel->car_count_ < maximum_number_of_cars_in_tunnel) {
                // 同一方向且未达到最大容量，进入
                (tunnel->car_count_)++;
                car->start_time = std::chrono::high_resolution_clock::now();
                car->state = State::INNER;
                Logger::log(LogLevel::INFO, "Car " + std::to_string(car->car_id) +
                                            " entering tunnel in direction " +
                                            std::to_string(static_cast<int>(car->direction_)) +
                                            " (same direction, space available).");
                break;
            } else if (tunnel->current_direction_ != car->direction_) {
                // 方向不同，等待
                Logger::log(LogLevel::WARN, "Car " + std::to_string(car->car_id) +
                                            " waiting due to opposite direction (direction " +
                                            std::to_string(static_cast<int>(car->direction_)) +
                                            "), tunnel occupied by direction " +
                                            std::to_string(static_cast<int>(tunnel->current_direction_)) + ".");
            } else {
                // 隧道已满，等待
                Logger::log(LogLevel::WARN, "Car " + std::to_string(car->car_id) +
                                            " waiting because tunnel full (direction " +
                                            std::to_string(static_cast<int>(car->direction_)) +
                                            ", cars in tunnel: " + std::to_string(tunnel->car_count_) + ").");
            }
            Signal(tunnel->mutex_, 0); // 释放信号量
            Wait(tunnel->block_, 0); // 等待阻塞信号量
            Wait(tunnel->mutex_, 0); // 再次获取信号量
        }
        Signal(tunnel->mutex_, 0); // 释放信号量

    }else{
    //    有红绿灯
        Wait(tunnel->mutex_, 0); // 获取信号量
        while (true) {
            if (isGreenLight(car->direction_) && tunnel->car_count_ < maximum_number_of_cars_in_tunnel) {
                // 同一方向且未达到最大容量，车辆可以进入
                if(tunnel->car_count_==0){
                    Wait(tunnel->zero_car_,0);
                }
                (tunnel->car_count_)++;
                car->start_time = std::chrono::high_resolution_clock::now();
                car->state = State::INNER;
                Logger::log(LogLevel::INFO, "Car " + std::to_string(car->car_id) +
                                            " entering tunnel in direction " +
                                            std::to_string(static_cast<int>(car->direction_)) +
                                            " (same direction, space available).");
                break;
            } else if (tunnel->current_direction_ != car->direction_) {
    //            方向不同
                Signal(tunnel->mutex_, 0); // 释放信号量
                if(car->direction_ == Direction::Eastbound) {
                    Wait(tunnel->direction_changed_, 0); // 等待方向变化的信号量
                    Signal(tunnel->direction_changed_, 0); // 等待方向变化的信号量
                    Wait(tunnel->zero_car_, 0); // 等待隧道空的信号量
                    Signal(tunnel->zero_car_, 0); // 等待隧道空的信号量
                }
                else{
                    Wait(tunnel->direction_changed_, 1); // 等待方向变化的信号量
                    Signal(tunnel->direction_changed_, 1); // 等待方向变化的信号量
                    Wait(tunnel->zero_car_, 0); // 等待隧道空的信号量
                    Signal(tunnel->zero_car_, 0); // 等待隧道空的信号量
                }

                Wait(tunnel->mutex_, 0); // 再次获取信号量
                Logger::log(LogLevel::WARN, "Car " + std::to_string(car->car_id) +
                                            " waiting due to opposite direction (direction " +
                                            std::to_string(static_cast<int>(car->direction_)) +
                                            "), tunnel occupied by direction " +
                                            std::to_string(static_cast<int>(tunnel->current_direction_)) + ".");
            } else {
                Logger::log(LogLevel::WARN, "Car " + std::to_string(car->car_id) +
                                            " waiting because tunnel full (direction " +
                                            std::to_string(static_cast<int>(car->direction_)) +
                                            ", cars in tunnel: " + std::to_string(tunnel->car_count_) + ").");
    //            车容量
                Signal(tunnel->mutex_, 0); // 释放信号量
                Wait(tunnel->block_, 0); // 等待信号量
                Wait(tunnel->mutex_, 0); // 再次获取信号量
            }
        }

        Signal(tunnel->mutex_, 0); // 释放信号量
    }
}


void process::leave(Car *car){
    Wait(tunnel->mutex_, 0); // 加锁

    (tunnel->car_count_)--;
    car->state = State::OUT;

    if (tunnel->car_count_ == 0 && use_rg) {
//         隧道kong，可以唤醒同方向的等待车
        Signal(tunnel->zero_car_,0);
    }
    Signal(tunnel->block_, 0);
//    }

    Signal(tunnel->mutex_, 0); // 解锁
    Logger::log(LogLevel::INFO, "Car " + std::to_string(car->car_id) + " Leave.");
}
bool process::isGreenLight(Direction dir) {
    return tunnel->current_direction_ == dir; // 当前方向为绿灯
}

void process::switchDirection() {
    if(tunnel->current_direction_ == Direction::Eastbound){
        tunnel->current_direction_ = Direction::Westbound;
        Signal(tunnel->direction_changed_,1);
        Wait(tunnel->direction_changed_,0);
    }else{
        tunnel->current_direction_ = Direction::Eastbound;
        Signal(tunnel->direction_changed_,0);
        Wait(tunnel->direction_changed_,1);
    }
    Logger::log(LogLevel::INFO, "Tunnel direction switched to " + std::string((tunnel->current_direction_ == Direction::Eastbound) ? "Eastbound" : "Westbound"));
}
void process::main_process(){
    start_time = std::chrono::high_resolution_clock::now(); // 记录起始时间
    Logger::setBaseTime();
    Logger::log(LogLevel::INFO, "PROCESS BEGAN");
    Logger::log(LogLevel::INFO, "Tunnel direction switched to " + std::string((tunnel->current_direction_ == Direction::Eastbound) ? "Eastbound" : "Westbound"));

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
                auto remaining_time = op.time - elapsed_time;
                if (remaining_time > 10) {
                    usleep((remaining_time - 10) * 1000);
                }
                cars[i].handel++;
                Logger::log(LogLevel::INFO,
                            "Car " + std::to_string(cars[i].car_id) +" lock " + to_string(op.mailbox));
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
                    cars[i].model_str = cars[i].model_str + readResult;
                }
                Logger::log(LogLevel::INFO, "Car " + std::to_string(cars[i].car_id) +
                                            " Reader: " + cars[i].model_str + ".");
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
        std::string data = cars[i].model_str + "|" + std::to_string(cars[i].handel) + "|" + std::to_string(cars[i].wait_handel);
        size_t data_length = data.length();
        write(pipefd[1], &data_length, sizeof(data_length));
        write(pipefd[1], data.c_str(), data_length);
        close(pipefd[1]);


//        Logger::log(LogLevel::INFO,cars[i].model_str);
//        Signal(total_number_of_cars_tunnel, 0); // 完成后释放信号量
        exit(0); // 子进程完成后退出，避免继续执行父进程代码
    } else {
        // 关闭写端
        close(pipefd[1]);

        // 等待所有子进程退出
        auto last_switch_time = start_time;
        while (true) {
            auto current_time = std::chrono::high_resolution_clock::now();
            auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - last_switch_time).count();
            if (int(elapsed_time/this->switch_time)%2&&use_rg) {
                switchDirection();
                last_switch_time = current_time;
            }

            // 检查是否所有子进程都已退出
            int status;
            pid_t pid = waitpid(-1, &status, WNOHANG);
            if (pid == -1) {
                // 没有更多子进程
                break;
            }
        }

        // 从管道读取数据
        for (int i = 0; i < total_number_of_cars; ++i) {
            size_t data_length;
            ssize_t n = read(pipefd[0], &data_length, sizeof(data_length));
            if (n == sizeof(data_length)) {
                std::string data(data_length, '\0');
                ssize_t bytes_read = 0;
                while (bytes_read < static_cast<ssize_t>(data_length)) {
                    ssize_t result = read(pipefd[0], &data[bytes_read], data_length - bytes_read);
                    if (result <= 0) {
                        break;
                    }
                    bytes_read += result;
                }
                if (bytes_read == static_cast<ssize_t>(data_length)) {
                    size_t pos1 = data.find('|');
                    size_t pos2 = data.find('|', pos1 + 1);
                    if (pos1 != std::string::npos && pos2 != std::string::npos) {
                        cars[i].model_str = data.substr(0, pos1);
                        cars[i].handel = std::stoi(data.substr(pos1 + 1, pos2 - pos1 - 1));
                        cars[i].wait_handel = std::stoi(data.substr(pos2 + 1));
                    }
                }
            }
        }
        close(pipefd[0]);
        int should = 0, had = 0;
        for (int i = 0; i < total_number_of_cars; ++i) {
            Logger::log(LogLevel::INFO, "Car " + std::to_string(cars[i].car_id) + " Should do: " + to_string(cars[i].wait_handel) +
                    " Had do: " + to_string(cars[i].handel) +
                    " Reader: " + cars[i].model_str + ".");
            should += cars[i].wait_handel;
            had += cars[i].handel;
        }
        mail_box->show();
        Logger::log(LogLevel::INFO,"Global Finish Rate: " + to_string(1.0*had/should));

        Logger::log(LogLevel::INFO, "PROCESS FINISH");
    }
}