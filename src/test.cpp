//#include <unistd.h>
//#include "../include/ipc.h"
//#include "../include/mp.h"
//#include "../include/logger.h"
//#include "../include/Car.h"
//#include "../include/Tunnel.h"
//#include <iostream>
//#include <cstdio>
//
//int total_number_of_cars=0;//汽车总数
//int maximum_number_of_cars_in_tunnel=0;//隧道最大汽车容量
//int tunnel_travel_time=0;//穿过隧道的时间
//int total_number_of_mailboxes=0;//邮箱数量
//int memory_segment_size=0;//每个邮箱的最大字数
//
//
//// 测试 IPC 相关功能
//void testIPC() {
//    std::cout << "Testing IPC functions..." << std::endl;
//    // 生成键
//    key_t k = Ftok(0, ".");
//    if (k == -1) {
//        std::cerr << "Failed to generate key." << std::endl;
//        return;
//    }
//
//    // 创建信号量
//    int sem_sign = sem_get(k, 1, true, 1);
//    if (sem_sign == -1) {
//        std::cerr << "Failed to get semaphore." << std::endl;
//        return;
//    }
//
//    // 等待信号量
//    Wait(sem_sign, 0);
//    std::cout << "Semaphore acquired." << std::endl;
//
//    // 释放信号量
//    Signal(sem_sign, 0);
//    std::cout << "Semaphore released." << std::endl;
//
//    // 删除信号量
//    sem_del(sem_sign);
//    std::cout << "Semaphore deleted." << std::endl;
//
//    // 创建共享内存
//    int shm_id = shm_get(k, 1024, IPC_CREAT | 0666);
//    if (shm_id == -1) {
//        std::cerr << "Failed to get shared memory." << std::endl;
//        return;
//    }
//
//    // 连接共享内存
//    void* shm_addr = shm_conn(shm_id);
//    if (shm_addr == (void*)-1) {
//        std::cerr << "Failed to connect to shared memory." << std::endl;
//        shm_del(shm_id);
//        return;
//    }
//
//    // 断开共享内存连接
//    shm_disconn(shm_addr);
//    std::cout << "Disconnected from shared memory." << std::endl;
//
//    // 删除共享内存
//    shm_del(shm_id);
//    std::cout << "Shared memory deleted." << std::endl;
//}
//
//// 测试 Car 类
//void testCar() {
//    std::cout << "Testing Car class..." << std::endl;
//    // 创建 Car 对象
//    Car car(1, ".", 1024, 1, Direction::Eastbound);
//
//    // 车辆进入
//    car.enter();
//    std::cout << "Car " << car.getCarId() << " entered the tunnel from " << car.getDirectionStr() << "." << std::endl;
//
//    // 模拟车辆在隧道内的操作
//    sleep(2);
//
//    // 车辆离开
//    car.leave();
//    std::cout << "Car " << car.getCarId() << " left the tunnel." << std::endl;
//}
//
//void testTunnel() {
//    // 生成 IPC 键
//    key_t key = Ftok(1, ".");
//
//    // 创建或获取用于保护内部状态的信号量集
//    int mutex_sid = sem_get(key, 1, true, 1);
//
//    // 创建或获取用于阻塞不符合方向的车的信号量集
//    int block_sid = sem_get(key + 1, 1, true, 0);
//
//    // 初始化 Tunnel 对象
//    Tunnel tunnel(mutex_sid, block_sid);
//
//    // 创建一辆车
//    Car car(1, ".", 1024, 1, Direction::Eastbound);
//
//    Logger::log(LogLevel::INFO,"Testing basic functionality of Tunnel...");
//    // 模拟车辆进入和离开
//}
//// 测试函数
//void test() {
//    // 测试信号量和共享内存操作
//    testIPC();
//
//    // 测试 Car 类
//    testCar();
//
//    // 测试 Tunnel 类
//    testTunnel();
//}
//
//
//
//#include <iostream>
//#include <unistd.h>
//#include "../include/Car.h"
//#include "../include/Tunnel.h"
//#include "../include/logger.h"
//
//// 模拟车辆进入和离开隧道的测试函数
//void testTunnelAndCar() {
//    // 创建信号量
//    key_t key = Ftok(0);
//    int mutex_sem = sem_get(key, 1, true, 1);
//    int block_sem = sem_get(key + 1, 1, true, 1);
//
//    // 创建隧道对象
//    Tunnel tunnel(mutex_sem, block_sem);
//
//    // 创建车辆对象
//
//    Car car1(0, "src/testfiles", 1024, 1, Direction::Eastbound);
//    Car car2(0, "src/testfiles", 1024, 2, Direction::Eastbound);
//    Car car3(0, "src/testfiles", 1024, 3, Direction::Westbound);
//
//    // 车辆1进入隧道
//    tunnel.enter(car1);
//    // 车辆2进入隧道
//    tunnel.enter(car2);
//    // 车辆3尝试进入隧道，应该等待
//    tunnel.enter(car3);
//
//    // 车辆1离开隧道
//    tunnel.leave(car1);
//    // 车辆2离开隧道
//    tunnel.leave(car2);
//
//    // 此时车辆3应该可以进入隧道
//    tunnel.enter(car3);
//    // 车辆3离开隧道
//    tunnel.leave(car3);
//
//    // 删除信号量
//    sem_del(mutex_sem);
//    sem_del(block_sem);
//}
//
//int main() {
//    try {
//        testTunnelAndCar();
//        std::cout << "All tests passed successfully." << std::endl;
//    } catch (const std::exception& e) {
//        std::cerr << "Test failed: " << e.what() << std::endl;
//        return 1;
//    }
//    return 0;
//}
//
////int main() {
////    test();
////    return 0;
////}