//
// Created by elysia on 2025/4/6.
//

#include "Logger.h"


using namespace std;
mutex logMutex;
std::chrono::time_point<std::chrono::high_resolution_clock> Logger::baseTime;

void Logger::setBaseTime(){
    baseTime = std::chrono::high_resolution_clock::now();
}

void Logger::log(LogLevel level, const string& message) {
    lock_guard<mutex> lock(logMutex);  // ��֤�̰߳�ȫ

    string colorCode;
    switch (level) {
        case LogLevel::INFO:
            colorCode = "\033[32m"; // ��ɫ
            break;
        case LogLevel::WARN:
            colorCode = "\033[33m"; // ��ɫ
            break;
        case LogLevel::ERROR:
            colorCode = "\033[31m"; // ��ɫ
            break;
        default:
            colorCode = "\033[0m";  // Ĭ��
            break;
    }

    string resetCode = "\033[0m";

    cout << colorCode
         << "[" << getTimestamp() << "] "
         << "[" << levelToString(level) << "] "
         << message
         << resetCode << endl;
    cout.flush();
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