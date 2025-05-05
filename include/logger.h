//
// Created by elysia on 2025/4/6.
//

#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <mutex>
#include <chrono>
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
    static void initLogFile(const std::string& filename);
    static void closeLogFile();

private:
    static std::string getTimestamp();
    static std::string levelToString(LogLevel level);

    static std::chrono::time_point<std::chrono::high_resolution_clock> baseTime;
    static std::ofstream logFile;
};

#endif // LOGGER_H
