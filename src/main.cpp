//
// Created by elysia on 2025/4/8.
//
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <logger.h>
#include <Car.h>
#include <ipc.h>
#include <mp.h>
#include <process.h>
#include <tunnel.h>
#include <txt_reader.h>
#include <mailbox.h>


using namespace std;
int total_number_of_cars=0;//��������
int maximum_number_of_cars_in_tunnel=0;//��������������
int tunnel_travel_time=0;//���������ʱ��
int total_number_of_mailboxes=0;//��������
int memory_segment_size=0;//ÿ��������������


int main(int argc, char** argv){
    std::srand(42);
//    srand(static_cast<unsigned int>(time(nullptr)));
    txt_reader reader(argv);
    cout << "total_number_of_cars_tunnel: " << total_number_of_cars << endl;
    cout << "maximum_number_of_cars_in_tunnel: " << maximum_number_of_cars_in_tunnel << endl;
    cout << "tunnel_travel_time: " << tunnel_travel_time << endl;
    cout << "total_number_of_mailboxes: " << total_number_of_mailboxes << endl;
    cout << "memory_segment_size: " << memory_segment_size << endl;
    cout << "-----------------------" << endl<<endl;
    process p(tunnel_travel_time, total_number_of_mailboxes, memory_segment_size, 0,argv[1]);
//  ��ʱ�����ڴ棬�� std::vector ���ٶ� cars �� std::vector ���ͣ�����������ʱ����Ҫ������Ԫ�أ��������·���һ�������ڴ�ռ䣬Ȼ���ԭ����Ԫ�ظ��ƣ����ƶ������µ��ڴ�ռ䣬�����ͷŵ�ԭ�����ڴ档��Ϳ�����֮ǰָ��Ԫ�صĵ�������ָ���������ʧЧ��
    for (int i = 0; i < total_number_of_cars; i++) {
        reader.input_car();
        p.init_car(reader);
    }

    for (int i = 0; i < total_number_of_cars; i++) {
        p.cars[i].show(); // Add to container
    }

//    ִ��������
    p.main_process();

    return 0;
}

