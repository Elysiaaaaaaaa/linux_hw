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
//        : car_id_(car_id), direction_(dir) {
Car::Car(int semid_tunnel_car, int car_id, Direction dir, txt_reader& reader)
        : semid_tunnel_can_enter(semid_tunnel_car), car_id_(car_id), direction_(dir) {
//    key_ = Ftok(proj_id, path.c_str());
    // Get or create a semaphore set with 1 semaphore, initialize to 1
//    semid_tunnel_can_enter = sem_get(key_, 1, true, 1);
    // Get or create shared memory
//    shmid_ = shm_init(key_, shm_size, IPC_CREAT | 0666);
    cost_time = int(tunnel_travel_time * (0.7 + (static_cast<double>(rand()) / RAND_MAX) * 0.6));
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
            op.data = data;
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
void Car::enter()
{
    cout<<"e1"<<endl;
    if(this->state!=State::WAITING){
        cout<<"e2"<<endl;
        Logger::log(LogLevel::ERROR,"car has entered");
        exit(1);
    }
    cout<<"e3"<<endl;
    if(sem_get_val(semid_tunnel_can_enter)<=0){
        Logger::log(LogLevel::INFO, "[Car " + std::to_string(car_id_) + " (" + getDirectionStr() + ")] wants to enter.");
//    �ȴ������
    }
    Wait(semid_tunnel_can_enter, 0);
    start_time = time(0);
    state = State::INNER;
    Logger::log(LogLevel::INFO, "[Car " + to_string(car_id_) + " (" + getDirectionStr() + ")] entered.");
}

// Release access (V operation)
void Car::leave()
{
    if(state!=State::INNER){
        Logger::log(LogLevel::ERROR,"car hasn't enter");
        exit(1);
    }
    Logger::log(LogLevel::INFO, "[Car " + to_string(car_id_) + " (" + getDirectionStr() + ")] is leaving.");
    state = State::OUT;
//    �����--
    Signal(semid_tunnel_can_enter, 0);
}

// Get pointer to shared memory
//void* Car::getSharedMemory()
//{
//    return shmaddr_;
//}

// Get car id
int Car::getCarId() const
{
    return car_id_;
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
bool Car::overtime(time_t ct){
    if(ct==-1){
        return (time(0) - start_time) > cost_time;
    }else{
        return (time(0) - start_time) > ct;
    }
}

void Car::show() const {
    std::cout << "-----------------------" << std::endl;
    std::cout << "Car ID: " << car_id_ << std::endl;
    std::cout << "Direction: " << getDirectionStr() << std::endl;
    std::cout << "tunnel_travel_time:" << cost_time <<std::endl;
    std::cout << "Operations:" << std::endl;
    for (const auto& op : operations) {
        if (op.isWrite) {
            std::cout << "  Write operation: "
                      << "Data: " << op.data << ", "
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


bool Car::main_process(){
//    ���������̣�����ģ��һ����������еĶ������ź�����tunnel��Ϊ�����ṩ
    cout<<"enter"<<endl;
    enter();

    for (const auto& op : operations) {
//        ��������
        if (op.isWrite) {
            std::cout << "  Write operation: "
                      << "Data: " << op.data << ", "
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
    cout<<"leave"<<endl;
    leave();

    return true;
}