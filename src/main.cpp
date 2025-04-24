//
// Created by elysia on 2025/4/8.
//
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <../include/logger.h>
#include <../include/Car.h>
#include <../include/ipc.h>
#include <../include/mp.h>
#include <../include/process.h>
#include <../include/Tunnel.h>
#include <../include/txt_reader.h>
#include <../include/mailbox.h>
#include <thread>



using namespace std;
int total_number_of_cars=0;//��������
int maximum_number_of_cars_in_tunnel=0;//��������������
int tunnel_travel_time=0;//���������ʱ��
int total_number_of_mailboxes=0;//��������
int memory_segment_size=0;//ÿ��������������


int main(int argc, char** argv){
    srand(static_cast<unsigned int>(time(nullptr)));
    Logger::log(LogLevel::INFO,"begin_monitor");
    txt_reader reader(argv);
    cout << "total_number_of_cars_tunnel: " << total_number_of_cars << endl;
    cout << "maximum_number_of_cars_in_tunnel: " << maximum_number_of_cars_in_tunnel << endl;
    cout << "tunnel_travel_time: " << tunnel_travel_time << endl;
    cout << "total_number_of_mailboxes: " << total_number_of_mailboxes << endl;
    cout << "memory_segment_size: " << memory_segment_size << endl;
    cout << "-----------------------" << endl<<endl;
    process p(0,argv[1]);
//  ��ʱ�����ڴ棬�� std::vector ���ٶ� cars �� std::vector ���ͣ�����������ʱ����Ҫ������Ԫ�أ��������·���һ�������ڴ�ռ䣬Ȼ���ԭ����Ԫ�ظ��ƣ����ƶ������µ��ڴ�ռ䣬�����ͷŵ�ԭ�����ڴ档��Ϳ�����֮ǰָ��Ԫ�صĵ�������ָ���������ʧЧ��
    for (int i = 0; i < total_number_of_cars; i++) {
        reader.input_car();
        p.init_car(reader);
//        tunnel.init_car(reader);
//        reader.buf >> idx >> direct;
//        Car car(idx, static_cast<Direction>(direct), reader); // Create car object
//        cars.push_back(car); // Add to container
    }
    for (int i = 0; i < total_number_of_cars; i++) {
        p.cars[i].show(); // Add to container
    }

//    ִ��������
    p.main_process();

    return 0;
}

//int main() {
//    test();
//    return 0;
//}