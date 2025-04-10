//
// Created by elysia on 2025/4/5.
//

#include "../include/Car.h"
#include "../include/ipc.h"
#include <cstdlib>
#include <cstring>
#include <iostream>
using namespace std;


// Constructor
Car::Car(int proj_id, const std::string& path, int shm_size, int car_id, Direction dir, txt_reader& reader)
        : car_id_(car_id), direction_(dir) {
    // Generate IPC key
//    char* cwd = std::getenv("PWD");
//    if (cwd == nullptr) {
//        std::cerr << "无法获取当前工作目录" << std::endl;
//        return;
//    }
//    std::string currentPath(cwd);
//    // 计算父目录
//    size_t lastSlash = currentPath.find_last_of('/');
//    if (lastSlash == std::string::npos) {
//        std::cerr << "无法找到父目录" << std::endl;
//        return;
//    }
//    std::string parentPath = currentPath.substr(0, lastSlash);
//    // 拼接目标路径
//    std::string targetPath = parentPath;
//    std::cout << "pwd::" << targetPath << std::endl;

//    cout<<path.c_str()<<endl;
    key_ = Ftok(proj_id, path.c_str());
    // Get or create a semaphore set with 1 semaphore, initialize to 1
    semid_ = sem_get(key_, 1, true, 1);
    // Get or create shared memory
    shmid_ = shm_init(key_, shm_size, IPC_CREAT | 0666);
    time = 0;
    state = State::WAITING;
    model_str = "";
    // Attach to shared memory
    shmaddr_ = shm_conn(shmid_);
//  init p
    m.resize(total_number_of_mailboxes);
    for (int i = 0; i < total_number_of_mailboxes; i++)
        m[i] = 0;
    std::string str;
    cout<<"1"<<endl;
    while (reader.buf >> str) {
        cout<<str<<endl;

        if (str == "w") { // 写入
            std::string st;
            int t;
            int n;
            std::string data;
            int len;
            reader.buf >> st >> t >> n; // 字符串，时间，第几个邮箱
            data = st.substr(1, st.length() - 1); // 去掉引号
//            data = st; // 去掉引号
            len = data.length();
            Operation op;
            op.isWrite = true;
            op.data = data;
            op.time = t;
            op.mailbox = n;
            op.length = len;
            addOperation(op);
        }
        else if (str == "r") { // 读入
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
            cout<<"initfinish"<<endl;
            break;
        }else{
            Logger::log(LogLevel::ERROR, "unsupport model");
            break;
        }
    }
    cout<<"initfinish"<<endl;

}

// Destructor
Car::~Car()
{
    if (shmaddr_) {
        shm_disconn(shmaddr_);
        shmaddr_ = nullptr;
    }
    if (semid_ != -1) {
        sem_del(semid_);
    }
    if (shmid_ != -1) {
        if (shmctl(shmid_, IPC_RMID, nullptr) == -1) {
            Logger::log(LogLevel::ERROR, "~Car.shmctl");
            exit(EXIT_FAILURE);
        }
    }
}

// Request access (P operation)
void Car::enter()
{
    if(state!=State::WAITING){
        Logger::log(LogLevel::ERROR,"car has entered");
        exit(1);
    }
    Logger::log(LogLevel::INFO, "[Car " + std::to_string(car_id_) + " (" + getDirectionStr() + ")] wants to enter.");
    Wait(semid_, 0);
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
    Signal(semid_, 0);
}

// Get pointer to shared memory
void* Car::getSharedMemory()
{
    return shmaddr_;
}

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


// 实现 Car 类的 addOperation 方法
void Car::addOperation(const Operation& op) {
    operations.push_back(op);
}

// 实现 Car 类的 getOperations 方法
const std::vector<Operation>& Car::getOperations() const {
    return operations;
}

void Car::show() const {
    std::cout << "Car ID: " << car_id_ << std::endl;
    std::cout << "Direction: " << getDirectionStr() << std::endl;
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
}