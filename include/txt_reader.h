//
// Created by elysia on 2025/4/9.
//

#ifndef LINUX_HW_TXT_READER_H
#define LINUX_HW_TXT_READER_H
#include <iostream>
#include <stdlib.h>
#include <sstream>
#include <fstream>
#include "logger.h"

using namespace std;
extern int total_number_of_cars;//汽车总数
extern int maximum_number_of_cars_in_tunnel;//隧道最大汽车容量
extern int tunnel_travel_time;//穿过隧道的时间
extern int total_number_of_mailboxes;//邮箱数量
extern int memory_segment_size;//每个邮箱的最大字数

class txt_reader {
private:
    std::ifstream file;
public:
    std::stringstream buf;
    txt_reader(char** path) {
        file.open(path[1]);

//        // 输出文件的全部内容
//        std::string line;
//        std::cout << "File content:" << std::endl;
//        while (std::getline(file, line)) {
//            std::cout << line << std::endl;
//        }
//        // 将文件指针重置到文件开头
//        file.clear();
//        file.seekg(0, std::ios::beg);


        if (!file.is_open()) {
            Logger::log(LogLevel::WARN, "File open error!");
            return;
        }
        if (!(file >> total_number_of_cars >> maximum_number_of_cars_in_tunnel >> tunnel_travel_time >> total_number_of_mailboxes >> memory_segment_size)) {
            Logger::log(LogLevel::WARN, "Failed to read initial values from file!");
            file.close();
            return;
        }
        buf << total_number_of_cars << " " << maximum_number_of_cars_in_tunnel << " " << tunnel_travel_time << " " << total_number_of_mailboxes << " " << memory_segment_size << " ";

    }
    ~txt_reader() {
        if (file.is_open()) {
            file.close();
        }
    }
    bool input_car () {
        std::string s;
        buf.str("");//初始化缓冲区
        while (file >> s) {
            // cout<<"in:"<<s<<endl;
// 结束
            if (s == "end") {
                buf << s << " ";
                return true;
            }
// 来车
            else if (s == "car") {
                for(int i=0; i<2;i++) {
                    if (file >> s) {
                        // cout<<"car_in:"<<s<<endl;
    //                    idx
                        buf << s << " ";
                    } else {
                        Logger::log(LogLevel::WARN, "Unexpected end of file while reading car information!");
                        return false;
                    }
                }
            }
// 读写
            else {
// 操作
                buf << s << " ";

                for (int i = 0; i < 3; ++i) {
                    if (file >> s) {
                        buf << s << " ";
                        // cout<<"handle_in:"<<s<<endl;

                    } else {
                        Logger::log (LogLevel::WARN, "Unexpected end of file while reading read/write information!");
                        return false;
                    }
                }
            }
        }
        return true;
    }


};
#endif //LINUX_HW_TXT_READER_H
