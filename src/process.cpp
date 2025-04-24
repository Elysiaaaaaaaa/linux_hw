//
// Created by elysia on 2025/4/13.
//

#include "process.h"
process::process(int proj_id, const char *pathname) {
    // ��ȡ Tunnel ����Ĺ����ڴ� key
    key_t shm_key = ftok(pathname, proj_id + PROJ_SHM_TUNNEL_OFFSET);
    int shmid = shmget(shm_key, sizeof(Tunnel), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget failed");
        exit(EXIT_FAILURE);
    }

    // �����ڴ� attach
    void *shmaddr = shmat(shmid, nullptr, 0);
    if (shmaddr == (void *)-1) {
        perror("shmat failed");
        exit(EXIT_FAILURE);
    }

    // ʹ�� placement new ���� Tunnel ʵ���ڹ����ڴ���
    tunnel = new (shmaddr) Tunnel(proj_id, pathname);

    // ��ʼ���������飨������Ҳ�ɣ�
    std::vector<Car> cars;
    cars.reserve(total_number_of_cars);
}



process::~process() {
    // ���ٻ�����
    delete tunnel;
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
            car->start_time = time(0);
            car->state = State::INNER;
            Logger::log(LogLevel::INFO, "Car " + std::to_string(car->car_id) +
                                        " entering tunnel in direction " + std::to_string(static_cast<int>(car->direction_)) +
                                        " (empty tunnel).");
            break;
        } else if (tunnel->current_direction_ == car->direction_ && tunnel->car_count_ < maximum_number_of_cars_in_tunnel) {
            // ͬһ������δ�ﵽ�������������
            (tunnel->car_count_)++;
            car->start_time = time(0);
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

    Logger::log(LogLevel::INFO, "Car " + std::to_string(car->car_id) +
                                " leaving tunnel, remaining cars: " + std::to_string(tunnel->car_count_) + ".");

    if (tunnel->car_count_ < maximum_number_of_cars_in_tunnel) {
        // ���δ�������Ի���ͬ����ĵȴ���
        Signal(tunnel->block_, 0);
    }

    Signal(tunnel->mutex_, 0); // ����
}
void process::main_process(){
    Logger::log(LogLevel::INFO, "PROCESS BEGAN");

    pid_t id;
    int i = 0;

    // Ϊÿ��������һ���ӽ���
    for (; i < total_number_of_cars; ++i) {
        id = Fork();
        if (id == 0) {
            // �ӽ���
            break;
        }
//        sleep(1);
    }

    if (id == 0) {
        // �ӽ����߼�
//        Wait(total_number_of_cars_tunnel, 0);   // �ȴ��ź�����Դ
        enter(&cars[i]);
        tunnel->show();
//        ����ڵ�ÿ���������Է��ʺ��޸�һ������ģ���������ϵͳ�Ĺ����ڴ�Σ����Կ�����һ
//        �����飬���ʲ�������������r��w��������������ڵĳ������ڽ�����󱣳����ֻ�ͨѶ����
//        ���������ֻ��źţ�����������������Ҫ���ʸù����ڴ�Ρ�

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


//        Signal(total_number_of_cars_tunnel, 0); // ��ɺ��ͷ��ź���
        exit(0); // �ӽ�����ɺ��˳����������ִ�и����̴���
    } else {
        // �������߼�

        // �ȴ������ӽ����˳�
        for (int j = 0; j < total_number_of_cars; ++j) {
            wait((int*)0);
        }

        Logger::log(LogLevel::INFO, "PROCESS FINISH");
    }
}
