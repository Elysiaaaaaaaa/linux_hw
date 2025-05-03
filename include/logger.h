//
// Created by elysia on 2025/4/6.
//

#ifndef LINUX_HW_LOGGER_H
#define LINUX_HW_LOGGER_H
#include <string>
#include <chrono>
#include <iostream>
#include <ctime>
#include <sstream>
#include <mutex>
#include <map>

enum class LogLevel {
    INFO,
    WARN,
    ERROR
};

class Logger {
public:
    static void setBaseTime();
    static void log(LogLevel level, const std::string& message);

private:
    static std::chrono::time_point<std::chrono::high_resolution_clock> baseTime;
    static std::string getTimestamp();
    static std::string levelToString(LogLevel level);

};
#endif //LINUX_HW_LOGGER_H