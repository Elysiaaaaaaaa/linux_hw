//
// Created by elysia on 2025/4/5.
//

#include "../include/Tunnel.h"

extern int maximum_number_of_cars_in_tunnel;
Tunnel::Tunnel(int proj_id, const char *pathname){
    // ���� IPC ��
    cars.reserve(total_number_of_cars); // Ԥ�����㹻���ڴ�
    mutex_key = Ftok(proj_id, pathname);
    block_key = Ftok(proj_id + 1, pathname);
    // ʹ�ò�ͬ�� proj_id ��ȷ����ͬ�ļ�

    // �������ȡ����tunnel�����ڲ�״̬���ź���������ʼֵΪ 1
    mutex_ = sem_get(mutex_key, 1, true, 1);

    // �������ȡ�������������Ϸ���ĳ����ź���������ʼֵΪ 0
    block_ = sem_get(block_key, 1, true, 0);
//    ���ÿ��Ƴ��������ź���������Car�ڲ�
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
    Wait(mutex_, 0); // ����
    while (true) {
        cout<<car_count_<<endl;
        if (car_count_ == 0) {
            // �����û�г������÷��򲢽���
            current_direction_ = car->direction_;
            this->car_count_++;
            car->start_time = time(0);
            car->state = State::INNER;
            Logger::log(LogLevel::INFO, "Car " + std::to_string(car->car_id) +
                                        " entering tunnel in direction " + std::to_string(static_cast<int>(car->direction_)) +
                                        " (empty tunnel).");
            Signal(mutex_, 0); // ����
            return;
        }else if (current_direction_ == car->direction_ && car_count_ < maximum_number_of_cars_in_tunnel) {
            // ����һ����δ�������Խ���
            car_count_++;
            car->start_time = time(0);
            car->state = State::INNER;
            Logger::log(LogLevel::INFO, "Car " + std::to_string(car->car_id) +
                                        " entering tunnel in direction " + std::to_string(static_cast<int>(car->direction_)) +
                                        " (same direction, space available).");
            Signal(mutex_, 0); // ����
            return;
        }else if (current_direction_ != car->direction_) {
            // ����ͬ���ȴ�
            Logger::log(LogLevel::INFO, "Car " + std::to_string(car->car_id) +
                                        " waiting due to opposite direction (direction " + std::to_string(static_cast<int>(car->direction_)) +
                                        "), tunnel occupied by direction " + std::to_string(static_cast<int>(current_direction_)) + ".");
        } else {
            // ����������ȴ�
            Logger::log(LogLevel::INFO, "Car " + std::to_string(car->car_id) +
                                        " waiting because tunnel full (direction " + std::to_string(static_cast<int>(car->direction_)) +
                                        ", cars in tunnel: " + std::to_string(car_count_) + ").");
        }

        Wait(block_, mutex_);
    }
}


void Tunnel::leave(Car *car) {
    Wait(mutex_, 0); // ����

    car_count_--;
    car->state = State::OUT;

    Logger::log(LogLevel::INFO, "Car " + std::to_string(car->car_id) +
                                " leaving tunnel, remaining cars: " + std::to_string(car_count_) + ".");

    if (car_count_ < maximum_number_of_cars_in_tunnel) {
        // ���δ�������Ի���ͬ����ĵȴ���
        Signal(block_, 0);
    }

    Signal(mutex_, 0); // ����
}

void Tunnel::main_process() {
    Logger::log(LogLevel::INFO, "TUNNEL BEGAN");

    pid_t id;
    int i = 0;

    // Ϊÿ��������һ���ӽ���
    for (; i < total_number_of_cars; ++i) {
        usleep(1000000);
        id = Fork();
        if (id == 0) {
            // �ӽ���
            break;
        }
    }

    if (id == 0) {
        // �ӽ����߼�
//        Wait(total_number_of_cars_tunnel, 0);   // �ȴ��ź�����Դ

        cars[i].main_process(
                tunnel_number_of_cars,
                this); // ����������

//        Signal(total_number_of_cars_tunnel, 0); // ��ɺ��ͷ��ź���
        exit(0); // �ӽ�����ɺ��˳����������ִ�и����̴���
    } else {
        // �������߼�

        // �ȴ������ӽ����˳�
        int status;
        for (int j = 0; j < total_number_of_cars; ++j) {
            wait((int*)0);
        }

        Logger::log(LogLevel::INFO, "TUNNEL FINISH");
    }
}
