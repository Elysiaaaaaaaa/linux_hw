//
// Created by elysia on 2025/4/13.
//

#include "process.h"
process::process(int proj_id, const char *pathname){
    tunnel = new Tunnel(proj_id,pathname);
    std::vector<Car> cars;
    cars.reserve(total_number_of_cars); // Ԥ�����㹻���ڴ�

}

void process::init_car(txt_reader& reader){
    int idx;
    int direct;
    reader.buf >> idx >> direct;
    cars.emplace_back(idx, static_cast<Direction>(direct), reader);
}

void process::enter(Car *car){
    Wait(tunnel->mutex_, 0); // ����
    while (true) {
//        cout<<car_count_<<endl;
        if (tunnel->car_count_ == 0) {
            // �����û�г������÷��򲢽���
            tunnel->current_direction_ = car->direction_;
            tunnel->car_count_ += 1;
            car->start_time = time(0);
            car->state = State::INNER;
            Logger::log(LogLevel::INFO, "Car " + std::to_string(car->car_id) +
                                        " entering tunnel in direction " + std::to_string(static_cast<int>(car->direction_)) +
                                        " (empty tunnel).");
            break;
        }else if (tunnel->current_direction_ == car->direction_ && tunnel->car_count_ < maximum_number_of_cars_in_tunnel) {
            // ����һ����δ�������Խ���
            (tunnel->car_count_)++;
            car->start_time = time(0);
            car->state = State::INNER;
            Logger::log(LogLevel::INFO, "Car " + std::to_string(car->car_id) +
                                        " entering tunnel in direction " + std::to_string(static_cast<int>(car->direction_)) +
                                        " (same direction, space available).");
            break;
        }else if (tunnel->current_direction_ != car->direction_) {
            // ����ͬ���ȴ�
            Logger::log(LogLevel::INFO, "Car " + std::to_string(car->car_id) +
                                        " waiting due to opposite direction (direction " + std::to_string(static_cast<int>(car->direction_)) +
                                        "), tunnel occupied by direction " + std::to_string(static_cast<int>(tunnel->current_direction_)) + ".");
        } else {
            // ����������ȴ�
            Logger::log(LogLevel::INFO, "Car " + std::to_string(car->car_id) +
                                        " waiting because tunnel full (direction " + std::to_string(static_cast<int>(car->direction_)) +
                                        ", cars in tunnel: " + std::to_string(tunnel->car_count_) + ").");
        }
        Signal(tunnel->mutex_,0);
        Wait(tunnel->block_,0);
        Wait(tunnel->mutex_,0);
    }
    Signal(tunnel->mutex_, 0); // ����
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
        sleep(1);
    }

    if (id == 0) {
        // �ӽ����߼�
//        Wait(total_number_of_cars_tunnel, 0);   // �ȴ��ź�����Դ
        enter(&cars[i]);
        sleep(3);
        leave(&cars[i]);
//        cars[i].main_process(
//                tunnel->tunnel_number_of_cars,
//                *tunnel); // ����������

//        Signal(total_number_of_cars_tunnel, 0); // ��ɺ��ͷ��ź���
        exit(0); // �ӽ�����ɺ��˳����������ִ�и����̴���
    } else {
        // �������߼�

        // �ȴ������ӽ����˳�
        int status;
        for (int j = 0; j < total_number_of_cars; ++j) {
            wait((int*)0);
        }

        Logger::log(LogLevel::INFO, "PROCESS FINISH");
    }
}
