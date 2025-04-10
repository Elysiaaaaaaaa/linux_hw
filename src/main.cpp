//
// Created by elysia on 2025/4/8.
//
#include <unistd.h>
#include "../include/ipc.h"
#include "../include/mp.h"
#include "../include/logger.h"
#include "../include/Car.h"
#include "../include/Tunnel.h"
#include "../include/txt_reader.h"
#include <iostream>
#include <cstdio>

int total_number_of_cars=0;//��������
int maximum_number_of_cars_in_tunnel=0;//��������������
int tunnel_travel_time=0;//���������ʱ��
int total_number_of_mailboxes=0;//��������
int memory_segment_size=0;//ÿ��������������



int main(int argc, char** argv){
    txt_reader reader(argv);
    cout << "total_number_of_cars: " << total_number_of_cars << endl;
    cout << "maximum_number_of_cars_in_tunnel: " << maximum_number_of_cars_in_tunnel << endl;
    cout << "tunnel_travel_time: " << tunnel_travel_time << endl;
    cout << "total_number_of_mailboxes: " << total_number_of_mailboxes << endl;
    cout << "memory_segment_size: " << memory_segment_size << endl<<endl;
    int idx;
    int direct;
    char model;//��дģʽ
    std::string data; // д����������
    int time; // ����ʱ��
    int mailbox_idx; // ������
    int length; // �����������ݳ���
    string s;

    for (int i = 0; i < total_number_of_cars; i++){
        reader.input_car();
        cout<<"read:"<<reader.buf.str()<< endl;

        reader.buf >> idx >> direct;
        Car car = Car(0, "/mnt/d/Code/c++/linux_hw/src/testfiles", 1, idx, static_cast<Direction>(direct), reader);
        car.show();
    }
    for (int i = 0; i < total_number_of_cars; i++) {

    }


    return 0;
}

//int main() {
//    test();
//    return 0;
//}