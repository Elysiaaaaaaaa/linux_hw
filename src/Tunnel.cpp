//
// Created by elysia on 2025/4/5.
//

#include "../include/Tunnel.h"

extern int maximum_number_of_cars_in_tunnel;
Tunnel::Tunnel(int proj_id, const char *pathname, std::vector<Car>& cars):cars(cars) {
    // ���� IPC ��
    key_t mutex_key = Ftok(proj_id, pathname);
    key_t block_key = Ftok(proj_id + 1, pathname);
    // ʹ�ò�ͬ�� proj_id ��ȷ����ͬ�ļ�

    // �������ȡ����tunnel�����ڲ�״̬���ź���������ʼֵΪ 1
    mutex_ = sem_get(mutex_key, 1, true, 1);

    // �������ȡ�������������Ϸ���ĳ����ź���������ʼֵΪ 0
    block_ = sem_get(block_key, 1, true, 0);
//    ���ÿ��Ƴ��������ź���������Car�ڲ�
    semid_tunnel_car = sem_get(IPC_PRIVATE, 1, true, maximum_number_of_cars_in_tunnel);
    car_count_ = 0;
}




void Tunnel::enter(Car &car) {
    Wait(mutex_, 0); // ����

    while (true) {
        if (car_count_ == 0) {
            // �����û�г������÷��򲢽���
            current_direction_ = car.direction_;
            car_count_++;
            Logger::log(LogLevel::INFO, "Car " + std::to_string(car.car_id_) +
                                        " entering tunnel in direction " + std::to_string(static_cast<int>(car.direction_)) +
                                        " (empty tunnel).");
            Signal(mutex_, 0); // ����
            return;
        }else if (current_direction_ == car.direction_ && car_count_ < maximum_number_of_cars_in_tunnel) {
            // ����һ����δ�������Խ���
            car_count_++;
            Logger::log(LogLevel::INFO, "Car " + std::to_string(car.car_id_) +
                                        " entering tunnel in direction " + std::to_string(static_cast<int>(car.direction_)) +
                                        " (same direction, space available).");
            Signal(mutex_, 0); // ����
            return;
        }else if (current_direction_ != car.direction_) {
            // ����ͬ���ȴ�
            Logger::log(LogLevel::INFO,  "Car " + std::to_string(car.car_id_) +
                                         " waiting due to opposite direction (direction " + std::to_string(static_cast<int>(car.direction_)) +
                                         "), tunnel occupied by direction " + std::to_string(static_cast<int>(current_direction_)) + ".");
        } else {
            // ����������ȴ�
            Logger::log(LogLevel::INFO,  "Car " + std::to_string(car.car_id_) +
                                         " waiting because tunnel full (direction " + std::to_string(static_cast<int>(car.direction_)) +
                                         ", cars in tunnel: " + std::to_string(car_count_) + ").");
        }

        Signal(mutex_, 0); // ������������
        Wait(block_, 0);   // �������ȴ������仯
        Wait(mutex_, 0);   // �ٴμ��������¼������
    }
}


void Tunnel::leave(Car &car) {
    Wait(mutex_, 0); // ����

    car_count_--;
    Logger::log(LogLevel::INFO, "Car " + std::to_string(car.car_id_) +
                                " leaving tunnel, remaining cars: " + std::to_string(car_count_) + ".");

    if (car_count_ < maximum_number_of_cars_in_tunnel) {
        // ���δ�������Ի���ͬ����ĵȴ���
        Signal(block_, 0);
    }

    Signal(mutex_, 0); // ����
}

void Tunnel::main_process(){
    Logger::log(LogLevel::INFO,"TUNNEL BEGAN");
//    ��������̣�������������
    pid_t id;
    int i = 0;
    for(; i<total_number_of_cars; i++){
//        Ϊÿ������������
        id = fork();
        cout<<id<<"|"<<endl;
        usleep(100);
        if(int(id)==0){
            cout<<"11111111111"<<endl;
//            �ӽ��̣����� ����
            cars[i].main_process();
            break;
        }
    }
    usleep(100);
    sleep(2);
    cout<<sem_get_val(semid_tunnel_car)<<endl;
    while (sem_get_val(semid_tunnel_car)<maximum_number_of_cars_in_tunnel){
//        ������û��ȫ����
        usleep(100);
    }
    usleep(100);
    Logger::log(LogLevel::INFO,"TUNNEL FINISH");

}