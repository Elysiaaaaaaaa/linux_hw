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
std::chrono::time_point<std::chrono::high_resolution_clock> Logger::baseTime;

void Logger::setBaseTime(const std::chrono::time_point<std::chrono::high_resolution_clock>& startTime){
    baseTime = std::chrono::high_resolution_clock::now();
}

void Logger::log(LogLevel level, const string& message) {
    // lock_guard<mutex> lock(logMutex);  // 保证线程安全

    cout << "[" << getTimestamp() << "] "
         << "[" << levelToString(level) << "] "
         << message << endl;
    // cout.flush();
}

string Logger::getTimestamp() {
    auto now = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - baseTime).count();

    ostringstream oss;
    oss << elapsed;

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