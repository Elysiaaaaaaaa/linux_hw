#include "process.h"

#define PROJ_SHM_MAILBOX_OFFSET 8 // ����� mailbox �����ڴ�ƫ����

process::process(int num_mailboxes, int mem_size, int proj_id, const char *pathname) {
    // ��ȡ Tunnel ����Ĺ����ڴ� key
    key_t shm_key_tunnel = ftok(pathname, proj_id + PROJ_SHM_TUNNEL_OFFSET);
    int shmid_tunnel = shmget(shm_key_tunnel, sizeof(Tunnel), IPC_CREAT | 0666);
    if (shmid_tunnel == -1) {
        perror("shmget for tunnel failed");
        exit(EXIT_FAILURE);
    }

    // �����ڴ� attach
    void *shmaddr_tunnel = shmat(shmid_tunnel, nullptr, 0);
    if (shmaddr_tunnel == (void *)-1) {
        perror("shmat for tunnel failed");
        exit(EXIT_FAILURE);
    }

    // ʹ�� placement new ���� Tunnel ʵ���ڹ����ڴ���
    tunnel = new (shmaddr_tunnel) Tunnel(proj_id, pathname);

    // ��ȡ mailbox ����Ĺ����ڴ� key
    key_t shm_key_mailbox = ftok(pathname, proj_id + PROJ_SHM_MAILBOX_OFFSET);
    int shmid_mailbox = shmget(shm_key_mailbox, sizeof(mailbox), IPC_CREAT | 0666);
    if (shmid_mailbox == -1) {
        perror("shmget for mailbox failed");
        exit(EXIT_FAILURE);
    }

    // �����ڴ� attach
    void *shmaddr_mailbox = shmat(shmid_mailbox, nullptr, 0);
    if (shmaddr_mailbox == (void *)-1) {
        perror("shmat for mailbox failed");
        exit(EXIT_FAILURE);
    }

    // ʹ�� placement new ���� mailbox ʵ���ڹ����ڴ���
    mail_box = new (shmaddr_mailbox) mailbox(num_mailboxes, mem_size, proj_id, pathname);

    // ��ʼ���������飨������Ҳ�ɣ�
    cars.reserve(total_number_of_cars);
}

process::~process() {
    // ���� Tunnel ����
    tunnel->~Tunnel();
    if (shmdt(tunnel) == -1) {
        perror("shmdt for tunnel failed");
    }

    // ���� mailbox ����
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
    Wait(tunnel->mutex_, 0); // ��ȡ�ź���
    while (true) {
        if (tunnel->car_count_ == 0) {
            // ���û�г������÷��򲢽���
            tunnel->current_direction_ = car->direction_;
            tunnel->car_count_ += 1;
            car->start_time = std::chrono::high_resolution_clock::now();
            car->state = State::INNER;
            Logger::log(LogLevel::INFO, "Car " + std::to_string(car->car_id) +
                                        " entering tunnel in direction " + std::to_string(static_cast<int>(car->direction_)) +
                                        " (empty tunnel).");
            break;
        } else if (tunnel->current_direction_ == car->direction_ && tunnel->car_count_ < maximum_number_of_cars_in_tunnel) {
            // ͬһ������δ�ﵽ�������������
            (tunnel->car_count_)++;
            car->start_time = std::chrono::high_resolution_clock::now();
            car->state = State::INNER;
            Logger::log(LogLevel::INFO, "Car " + std::to_string(car->car_id) +
                                        " entering tunnel in direction " + std::to_string(static_cast<int>(car->direction_)) +
                                        " (same direction, space available).");
            break;
        } else if (tunnel->current_direction_ != car->direction_) {
            // ����ͬ���ȴ�
            Logger::log(LogLevel::WARN, "Car " + std::to_string(car->car_id) +
                                        " waiting due to opposite direction (direction " + std::to_string(static_cast<int>(car->direction_)) +
                                        "), tunnel occupied by direction " + std::to_string(static_cast<int>(tunnel->current_direction_)) + ".");
        } else {
            // ����������ȴ�
            Logger::log(LogLevel::WARN, "Car " + std::to_string(car->car_id) +
                                        " waiting because tunnel full (direction " + std::to_string(static_cast<int>(car->direction_)) +
                                        ", cars in tunnel: " + std::to_string(tunnel->car_count_) + ").");
        }
        Signal(tunnel->mutex_, 0); // �ͷ��ź���
        Wait(tunnel->block_, 0); // �ȴ������ź���
        Wait(tunnel->mutex_, 0); // �ٴλ�ȡ�ź���
    }
    Signal(tunnel->mutex_, 0); // �ͷ��ź���
}

void process::leave(Car *car){
    Wait(tunnel->mutex_, 0); // ����

    (tunnel->car_count_)--;
    car->state = State::OUT;

    if (tunnel->car_count_ < maximum_number_of_cars_in_tunnel) {
        // ���δ�������Ի���ͬ����ĵȴ���
        Signal(tunnel->block_, 0);
    }

    Signal(tunnel->mutex_, 0); // ����
}

void process::main_process(){
    start_time = std::chrono::high_resolution_clock::now(); // ��¼��ʼʱ��
    Logger::setBaseTime(start_time);
    Logger::log(LogLevel::INFO, "PROCESS BEGAN");

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t id;
    int i = 0;
    // Ϊÿ��������һ���ӽ���
    for (; i < total_number_of_cars; ++i) {
        id = Fork();
        if (id == 0) {
            // �ӽ���
            close(pipefd[0]); // �رն���
            break;
        }
//        sleep(1);
    }

    if (id == 0) {
        // �ӽ����߼�
//        Wait(total_number_of_cars_tunnel, 0);   // �ȴ��ź�����Դ
        enter(&cars[i]);
        tunnel->show();
//        sleep(1);
//        ����ڵ�ÿ���������Է��ʺ��޸�һ������ģ���������ϵͳ�Ĺ����ڴ�Σ����Կ�����һ
//        �����飬���ʲ�������������r��w��������������ڵĳ������ڽ�����󱣳����ֻ�ͨѶ����
//        ���������ֻ��źţ�����������������Ҫ���ʸù����ڴ�Ρ�
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

        // ����Ԥ�ƽ���ʱ��
        auto end_time = cars[i].start_time + cars[i].cost_time;

        // ��ȡ��ǰʱ��
        auto current_time = std::chrono::high_resolution_clock::now();

        // ������Ҫ˯�ߵ�ʱ��
        auto sleep_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - current_time);

        // �����Ҫ˯�ߣ�����ǰʱ�仹δ��Ԥ�ƽ���ʱ�䣩
        if (sleep_duration.count() > 0) {
            // ʹ�� std::this_thread::sleep_for ����������usleep ��֧�� std::chrono ���ͣ�����ʹ�� std::this_thread::sleep_for��
            usleep(sleep_duration.count()*1000);
        }
        leave(&cars[i]);

        // �������ݵ��ܵ�
        const std::string& data = cars[i].model_str;
        write(pipefd[1], data.c_str(), data.size());
        close(pipefd[1]);

//        Logger::log(LogLevel::INFO,cars[i].model_str);
//        Signal(total_number_of_cars_tunnel, 0); // ��ɺ��ͷ��ź���
        exit(0); // �ӽ�����ɺ��˳����������ִ�и����̴���
    } else {
        // �ر�д��
        close(pipefd[1]);

        // �ȴ������ӽ����˳�
        for (int j = 0; j < total_number_of_cars; ++j) {
            wait((int*)0);
        }

        // �ӹܵ���ȡ����
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