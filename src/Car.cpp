//
// Created by elysia on 2025/4/5.
//

#include "Car.h"


Car::Car(int car_id, Direction dir, txt_reader& reader)
        : direction_(dir) {
    this->car_id = car_id;


    // 计算最大波动值（30%）
    int fluctuation = std::rand() % 61 - 30;
    int fluctuated_time = static_cast<int>(tunnel_travel_time * (1 + (fluctuation / 100.0)));
    cost_time = std::chrono::milliseconds(fluctuated_time);
    state = State::WAITING;
    model_str = "";
    m.resize(total_number_of_mailboxes);
    for (int i = 0; i < total_number_of_mailboxes; i++)
        m[i] = 0;
    string str;
    while (reader.buf >> str) {
        if (str == "w") { // 写入
            std::string st;
            int t;
            int n;
            std::string data;
            int len;
            reader.buf >> st >> t >> n; // 字符串，时间，第几个邮箱
            data = st.substr(1, st.length() - 2); // 去掉引号
            len = data.length();
            Operation op;
            op.isWrite = true;
            op.data = data; // 直接赋值
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
            break;
        }else{
            Logger::log(LogLevel::ERROR, "unsupport model");
            break;
        }
    }
    std::sort(operations.begin(), operations.end(), [](const Operation& a, const Operation& b) {
        return a.time < b.time;
    });
}

// Destructor
Car::~Car()
{
}

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


// 实现 Car 类的 addOperation 方法
void Car::addOperation(const Operation& op) {
    operations.push_back(op);
}

void Car::show() const {
    std::cout << "-----------------------" << std::endl;
    std::cout << "Car ID: " << car_id << std::endl;
    std::cout << "Direction: " << getDirectionStr() << std::endl;
    std::cout << "tunnel_travel_time: " << cost_time.count() << " ms" << std::endl;

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