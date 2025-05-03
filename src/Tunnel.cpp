//
// Created by elysia on 2025/4/5.
//

#include "Tunnel.h"

extern int maximum_number_of_cars_in_tunnel;
Tunnel::Tunnel(int proj_id, const char *pathname){
    // ���ɹ�����ź��� key
    mutex_key = ftok(pathname, proj_id + PROJ_MUTEX_KEY_OFFSET);
    block_key = ftok(pathname, proj_id + PROJ_BLOCK_KEY_OFFSET);
    car_count_key = ftok(pathname, proj_id + PROJ_CARCOUNT_KEY_OFFSET);
    key_t maxcars_key = ftok(pathname, proj_id + PROJ_MAXCARS_KEY_OFFSET);
    key_t totalcars_key = ftok(pathname, proj_id + PROJ_TOTALCARS_KEY_OFFSET);

    // ��ȡ�򴴽��ź���
    mutex_ = sem_get(mutex_key, 1, true, 1);
    block_ = sem_get(block_key, 1, true, 0);
    tunnel_number_of_cars = sem_get(maxcars_key, 1, true, maximum_number_of_cars_in_tunnel);
    total_number_of_cars_tunnel = sem_get(totalcars_key, 1, true, total_number_of_cars);

    // ��ͨ��������Ϊ Tunnel ���ڹ����ڴ��У������Ա������������
    car_count_ = 0;
}

//void Tunnel::enter(Car *car) {
//    Wait(mutex_, 0); // ����
//    while (true) {
//        cout<<car_count_<<endl;
//        if (car_count_ == 0) {
//            // �����û�г������÷��򲢽���
//            current_direction_ = car->direction_;
//            this->car_count_++;
//            car->start_time = time(0);
//            car->state = State::INNER;
//            Logger::log(LogLevel::INFO, "Car " + std::to_string(car->car_id) +
//                                        " entering tunnel in direction " + std::to_string(static_cast<int>(car->direction_)) +
//                                        " (empty tunnel).");
//            break;
//        }else if (current_direction_ == car->direction_ && car_count_ < maximum_number_of_cars_in_tunnel) {
//            // ����һ����δ�������Խ���
//            car_count_++;
//            car->start_time = time(0);
//            car->state = State::INNER;
//            Logger::log(LogLevel::INFO, "Car " + std::to_string(car->car_id) +
//                                        " entering tunnel in direction " + std::to_string(static_cast<int>(car->direction_)) +
//                                        " (same direction, space available).");
//            break;
//        }else if (current_direction_ != car->direction_) {
//            // ����ͬ���ȴ�
//            Logger::log(LogLevel::INFO, "Car " + std::to_string(car->car_id) +
//                                        " waiting due to opposite direction (direction " + std::to_string(static_cast<int>(car->direction_)) +
//                                        "), tunnel occupied by direction " + std::to_string(static_cast<int>(current_direction_)) + ".");
//        } else {
//            // ����������ȴ�
//            Logger::log(LogLevel::INFO, "Car " + std::to_string(car->car_id) +
//                                        " waiting because tunnel full (direction " + std::to_string(static_cast<int>(car->direction_)) +
//                                        ", cars in tunnel: " + std::to_string(car_count_) + ").");
//        }
//        Signal(mutex_,0);
//        Wait(block_,0);
////        wait(block_, mutex_);
//    }
//    Signal(mutex_, 0); // ����
//}
//
//
//void Tunnel::leave(Car *car) {
//    Wait(mutex_, 0); // ����
//
//    car_count_--;
//    car->state = State::OUT;
//
//    Logger::log(LogLevel::INFO, "Car " + std::to_string(car->car_id) +
//                                " leaving tunnel, remaining cars: " + std::to_string(car_count_) + ".");
//
//    if (car_count_ < maximum_number_of_cars_in_tunnel) {
//        // ���δ�������Ի���ͬ����ĵȴ���
//        Signal(block_, 0);
//    }
//
//    Signal(mutex_, 0); // ����
//}


void Tunnel::show() {
    std::string directionStr = (current_direction_ == Direction::Eastbound) ? "Eastbound" : "Westbound";
    std::string logMessage = "Tunnel Status: Cars count: " + std::to_string(car_count_) +
                             ", Current direction: " + directionStr;
    Logger::log(LogLevel::INFO, logMessage);
}

Tunnel::~Tunnel() {

}

//
//void Tunnel::main_process() {
//    Logger::log(LogLevel::INFO, "TUNNEL BEGAN");
//
//    pid_t id;
//    int i = 0;
//
//    // Ϊÿ��������һ���ӽ���
//    for (; i < total_number_of_cars; ++i) {
//        id = Fork();
//        if (id == 0) {
//            // �ӽ���
//            break;
//        }
//    }
//
//    if (id == 0) {
//        // �ӽ����߼�
////        Wait(total_number_of_cars_tunnel, 0);   // �ȴ��ź�����Դ
//
//        cars[i].main_process(
//                tunnel_number_of_cars,
//                this); // ����������
//
////        Signal(total_number_of_cars_tunnel, 0); // ��ɺ��ͷ��ź���
//        exit(0); // �ӽ�����ɺ��˳����������ִ�и����̴���
//    } else {
//        // �������߼�
//
//        // �ȴ������ӽ����˳�
//        int status;
//        for (int j = 0; j < total_number_of_cars; ++j) {
//            wait((int*)0);
//        }
//
//        Logger::log(LogLevel::INFO, "TUNNEL FINISH");
//    }
//}