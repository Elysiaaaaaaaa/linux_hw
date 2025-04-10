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
#include <vector>
#include <cstdlib>
#include <ctime>
using namespace std;
int total_number_of_cars=0;//汽车总数
int maximum_number_of_cars_in_tunnel=0;//隧道最大汽车容量
int tunnel_travel_time=0;//穿过隧道的时间
int total_number_of_mailboxes=0;//邮箱数量
int memory_segment_size=0;//每个邮箱的最大字数


int main(int argc, char** argv){
    srand(static_cast<unsigned int>(time(nullptr)));
    Logger::log(LogLevel::INFO,"begin_monitor");
    txt_reader reader(argv);
    cout << "total_number_of_cars: " << total_number_of_cars << endl;
    cout << "maximum_number_of_cars_in_tunnel: " << maximum_number_of_cars_in_tunnel << endl;
    cout << "tunnel_travel_time: " << tunnel_travel_time << endl;
    cout << "total_number_of_mailboxes: " << total_number_of_mailboxes << endl;
    cout << "memory_segment_size: " << memory_segment_size << endl<<endl;
    std::vector<Car> cars; // 创建一个存储 Car 对象的 vector
    cars.reserve(total_number_of_cars); // 预分配足够的内存
//  及时分配内存，当 std::vector （假定 cars 是 std::vector 类型）的容量不足时，若要插入新元素，它会重新分配一块更大的内存空间，然后把原来的元素复制（或移动）到新的内存空间，接着释放掉原来的内存。这就可能让之前指向元素的迭代器、指针或者引用失效。
    int idx;
    int direct;
    char model;//读写模式
    std::string data; // 写操作的数据
    int time; // 操作时间
    int mailbox_idx; // 邮箱编号
    int length; // 读操作的数据长度
    string s;
    std::string conv_str = argv[1]; // 使用 std::string 的构造函数将 char * 转换为 std::string
    Tunnel tunnel(0,argv[1]);

    for (int i = 0; i < total_number_of_cars; i++){
        reader.input_car();

//        cout<<"read:"<<reader.buf.str()<< endl;

        reader.buf >> idx >> direct;
        cars.emplace_back(tunnel.semid_tunnel_car, idx, static_cast<Direction>(direct), reader);
    }
    for (int i = 0; i < total_number_of_cars; i++) {
        cars[i].enter();
//        cars[i].show();
        cars[i].leave();

    }


    return 0;
}

//int main() {
//    test();
//    return 0;
//}