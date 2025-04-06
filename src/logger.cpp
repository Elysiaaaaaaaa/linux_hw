//
// Created by elysia on 2025/4/6.
//

#include "../include/Logger.h"
#include <iostream>
#include <ctime>
#include <sstream>
#include <mutex>
#include <map>
std::mutex logMutex;

void Logger::log(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(logMutex);  // 保证线程安全

    std::cout << "[" << getTimestamp() << "] "
              << "[" << levelToString(level) << "] "
              << message << std::endl;
}

std::string Logger::getTimestamp() {
    time_t now = time(0);
    tm *ltm = localtime(&now);

    std::ostringstream oss;
    oss << (1900 + ltm->tm_year) << "-"
        << (1 + ltm->tm_mon) << "-"
        << ltm->tm_mday << " "
        << ltm->tm_hour << ":"
        << ltm->tm_min << ":"
        << ltm->tm_sec;

    return oss.str();
}

std::string Logger::levelToString(LogLevel level) {
    static const std::map<LogLevel, std::string> levelToStr = {
            {LogLevel::INFO, "INFO"},
            {LogLevel::WARN, "WARN"},
            {LogLevel::ERROR, "ERROR"}
    };

    auto it = levelToStr.find(level);
    return it != levelToStr.end() ? it->second : "UNKNOWN";
}
