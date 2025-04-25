//
// Created by elysia on 2025/4/5.
//

#include "../include/Car.h"
#include "../include/ipc.h"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <random>
using namespace std;


// Constructor
//Car::Car(int proj_id, const std::string& path, int shm_size, int car_id, Direction dir, txt_reader& reader)
//        : car_id(car_id), direction_(dir) {
Car::Car(int car_id, Direction dir, txt_reader& reader)
        : direction_(dir) {
//    key_ = Ftok(proj_id, path.c_str());
    // Get or create a semaphore set with 1 semaphore, initialize to 1
//    semid_tunnel_can_enter = sem_get(key_, 1, true, 1);
    // Get or create shared memory
//    shmid_ = shm_init(key_, shm_size, IPC_CREAT | 0666);
    this->car_id = car_id;
    // ������󲨶�ֵ��30%��
    // ������󲨶�ֵ��30%��
    int max_fluctuation = tunnel_travel_time * 30 / 100;

    // ���� -30% ~ +30% ��Χ�������������
    int fluctuation = (rand() % (2 * max_fluctuation + 1)) - max_fluctuation;

    // �õ��������� travel time
    int adjusted_travel_time = tunnel_travel_time + fluctuation;

    // ��ʼ�� cost_time Ϊ��ǰʱ��� + adjusted_travel_time ����
    cost_time = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(adjusted_travel_time);

    state = State::WAITING;
    model_str = "";
    // Attach to shared memory
//    shmaddr_ = shm_conn(shmid_);
//  init p ָ��Ϊ0
    m.resize(total_number_of_mailboxes);
    for (int i = 0; i < total_number_of_mailboxes; i++)
        m[i] = 0;
    string str;
    while (reader.buf >> str) {
        if (str == "w") { // д��
            std::string st;
            int t;
            int n;
            std::string data;
            int len;
            reader.buf >> st >> t >> n; // �ַ�����ʱ�䣬�ڼ�������
            data = st.substr(1, st.length() - 1); // ȥ������
//            data = st; // ȥ������
            len = data.length();
            Operation op;
            op.isWrite = true;
            op.data = data.c_str();
            op.time = t;
            op.mailbox = n;
            op.length = len;
            addOperation(op);
        }
        else if (str == "r") { // ����
            int len;
            int t;
            int n;
            reader.buf >> len >> t >> n;
            Operation op;
            op.isWrite = false;
            op.time = t;
            op.mailbox = n;
            op.length = len;
            addOperation(op);
        }else if (str=="end"){
            break;
        }else{
            Logger::log(LogLevel::ERROR, "unsupport model");
            break;
        }
    }
}

// Destructor
Car::~Car()
{
//    if (shmaddr_) {
//        shm_disconn(shmaddr_);
//        shmaddr_ = nullptr;
//    }
//    if (semid_tunnel_can_enter != -1) {
//        sem_del(semid_tunnel_can_enter);
//    }
//    if (shmid_ != -1) {
//        if (shmctl(shmid_, IPC_RMID, nullptr) == -1) {
//            Logger::log(LogLevel::ERROR, "~Car.shmctl");
//            exit(EXIT_FAILURE);
//        }
//    }
}

// Request access (P operation)
//void Car::enter(int semid_tunnel_can_enter, Tunnel* tunnel)
//{
//    tunnel->enter(this);
//}

// Release access (V operation)
//void Car::leave(int semid_tunnel_can_enter, Tunnel* tunnel)
//{
//    tunnel->leave(this);
//}

// Get pointer to shared memory
//void* Car::getSharedMemory()
//{
//    return shmaddr_;
//}

// Get car id
int Car::getCarId() const
{
    return car_id;
}

// Get direction
Direction Car::getDirection() const
{
    return direction_;
}

// Get direction as string
std::string Car::getDirectionStr() const
{
    return (direction_ == Direction::Eastbound) ? "Eastbound" : "Westbound";
}


// ʵ�� Car ��� addOperation ����
void Car::addOperation(const Operation& op) {
    operations.push_back(op);
}
bool Car::exet_op(){
//    ȷ���������
    if(state!=State::INNER){
        Logger::log(LogLevel::ERROR,"car state uncorrect");
        exit(1);
    }

//    todo

    return true;
}
// ʵ�� Car ��� getOperations ����
const std::vector<Operation>& Car::getOperations() const {
    return operations;
}


void Car::show() const {
    std::cout << "-----------------------" << std::endl;
    std::cout << "Car ID: " << car_id << std::endl;
    std::cout << "Direction: " << getDirectionStr() << std::endl;
    std::cout << "tunnel_travel_time (adjusted): " << adjusted_travel_time << " ms" << std::endl;

    std::cout << "Operations:" << std::endl;
    for (const auto& op : operations) {
        if (op.isWrite) {
            std::cout << "  Write operation: "
                      << "Data: " << string(op.data) << ", "
                      << "Time: " << op.time << ", "
                      << "Mailbox: " << op.mailbox << ", "
                      << "Length: " << op.length << std::endl;
        } else {
            std::cout << "  Read operation: "
                      << "Time: " << op.time << ", "
                      << "Mailbox: " << op.mailbox << ", "
                      << "Length: " << op.length << std::endl;
        }
    }
    std::cout << "-----------------------" << std::endl;
}


//bool Car::main_process(int& semid_tunnel_can_enter, Tunnel* tunnel){
////    ���������̣�����ģ��һ����������еĶ������ź�����tunnel��Ϊ�����ṩ
//    tunnel->enter(this);
//    sleep(2);
//    tunnel->leave(this);
//    return true;
//}