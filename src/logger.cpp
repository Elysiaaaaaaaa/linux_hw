//
// Created by elysia on 2025/4/6.
//

#include "Logger.h"

using namespace std;
mutex logMutex;
chrono::time_point<chrono::high_resolution_clock> Logger::baseTime;
ofstream Logger::logFile;

void Logger::setBaseTime() {
    baseTime = chrono::high_resolution_clock::now();
}

void Logger::initLogFile(const string& filename) {
    lock_guard<mutex> lock(logMutex);
    logFile.open(filename, ios::out | ios::app);
    if (!logFile.is_open()) {
        cerr << "Failed to open log file: " << filename << endl;
    }
}

void Logger::closeLogFile() {
    lock_guard<mutex> lock(logMutex);
    if (logFile.is_open()) {
        logFile.close();
    }
}

void Logger::log(LogLevel level, const string& message) {
    lock_guard<mutex> lock(logMutex);

    string colorCode;
    switch (level) {
        case LogLevel::INFO:
            colorCode = "\033[32m"; // 绿色
            break;
        case LogLevel::WARN:
            colorCode = "\033[33m"; // 黄色
            break;
        case LogLevel::ERROR:
            colorCode = "\033[31m"; // 红色
            break;
        default:
            colorCode = "\033[0m";
            break;
    }
    string resetCode = "\033[0m";

    stringstream formatted;
    formatted << "[" << getTimestamp() << "] "
              << "[" << levelToString(level) << "] "
              << message;

    // 输出到终端（带颜色）
    cout << colorCode << formatted.str() << resetCode << endl;
    cout.flush();

    // 输出到文件（不带颜色）
    if (logFile.is_open()) {
        logFile << formatted.str() << endl;
        logFile.flush();
    }
}

string Logger::getTimestamp() {
    auto now = chrono::high_resolution_clock::now();
    auto elapsed = chrono::duration_cast<chrono::milliseconds>(now - baseTime).count();

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
