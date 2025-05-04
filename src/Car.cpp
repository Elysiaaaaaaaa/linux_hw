//
// Created by elysia on 2025/4/5.
//

#include "Car.h"


Car::Car(int car_id, Direction dir, txt_reader& reader)
        :m(total_number_of_mailboxes, 0),
          car_id(car_id),
          direction_(dir),
          cost_time(calculateTravelTime()),
          state(State::WAITING),
          model_str(""),
          handel(0),
          wait_handel(0){
    parseOperations(reader);
    std::sort(operations.begin(), operations.end(), [](const Operation& a, const Operation& b) {
        return a.time < b.time;
    });
}

std::chrono::milliseconds Car::calculateTravelTime() {
    int fluctuation = std::rand() % 61 - 30;
    int fluctuated_time = static_cast<int>(tunnel_travel_time * (1 + (fluctuation / 100.0)));
    return std::chrono::milliseconds(fluctuated_time);
}
void Car::parseOperations(txt_reader& reader) {
    std::string str;
    while (reader.buf >> str) {
        if (str == "w") {
            wait_handel++;
            std::string data;
            int t, n;
            reader.buf >> data >> t >> n;
            data = data.substr(1, data.length() - 2);
            operations.push_back({true, data, t, n, static_cast<int>(data.length())});
        } else if (str == "r") {
            wait_handel++;
            int len, t, n;
            reader.buf >> len >> t >> n;
            operations.push_back({false, "", t, n, len});
        } else if (str == "end") {
            break;
        } else {
            Logger::log(LogLevel::ERROR, "unsupported model");
            break;
        }
    }
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