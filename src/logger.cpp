//
// Created by elysia on 2025/4/6.
//

#include "../include/Logger.h"
#include <iostream>
#include <ctime>
#include <sstream>
#include <mutex>
#include <map>

using namespace std;
mutex logMutex;

void Logger::log(LogLevel level, const string& message) {
    lock_guard<mutex> lock(logMutex);  // 保证线程安全

    cout << "[" << getTimestamp() << "] "
              << "[" << levelToString(level) << "] "
              << message << endl;
    cout.flush();
}

string Logger::getTimestamp() {
    time_t now = time(0);
    tm *ltm = localtime(&now);

    ostringstream oss;
    oss << (1900 + ltm->tm_year) << "-"
        << (1 + ltm->tm_mon) << "-"
        << ltm->tm_mday << " "
        << ltm->tm_hour << ":"
        << ltm->tm_min << ":"
        << ltm->tm_sec;

    return oss.str();
}

string Logger::levelToString(LogLevel level) {
    static const map<LogLevel, string> levelToStr = {
            {LogLevel::INFO, "INFO"},
            {LogLevel::WARN, "WARN"},
            {LogLevel::ERROR, "ERROR"}
    };

    auto it = levelToStr.find(level);
    return it != levelToStr.end() ? it->second : "UNKNOWN";
}
