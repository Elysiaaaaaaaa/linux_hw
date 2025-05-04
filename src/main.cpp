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
int total_number_of_cars=0;//汽车总数
int maximum_number_of_cars_in_tunnel=0;//隧道最大汽车容量
int tunnel_travel_time=0;//穿过隧道的时间
int total_number_of_mailboxes=0;//邮箱数量
int memory_segment_size=0;//每个邮箱的最大字数


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
//  及时分配内存，当 std::vector （假定 cars 是 std::vector 类型）的容量不足时，若要插入新元素，它会重新分配一块更大的内存空间，然后把原来的元素复制（或移动）到新的内存空间，接着释放掉原来的内存。这就可能让之前指向元素的迭代器、指针或者引用失效。
    for (int i = 0; i < total_number_of_cars; i++) {
        reader.input_car();
        p.init_car(reader);
    }

    for (int i = 0; i < total_number_of_cars; i++) {
        p.cars[i].show(); // Add to container
    }

//    执行主进程
    p.main_process();

    return 0;
}

