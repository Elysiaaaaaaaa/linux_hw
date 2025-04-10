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
extern int total_number_of_cars;//��������
extern int maximum_number_of_cars_in_tunnel;//��������������
extern int tunnel_travel_time;//���������ʱ��
extern int total_number_of_mailboxes;//��������
extern int memory_segment_size;//ÿ��������������

class txt_reader {
private:
    std::ifstream file;
public:
    std::stringstream buf;
    txt_reader(char** path) {
        file.open(path[1]);

//        // ����ļ���ȫ������
//        std::string line;
//        std::cout << "File content:" << std::endl;
//        while (std::getline(file, line)) {
//            std::cout << line << std::endl;
//        }
//        // ���ļ�ָ�����õ��ļ���ͷ
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
        buf.str("");//��ʼ��������
        while (file >> s) {
            // cout<<"in:"<<s<<endl;
// ����
            if (s == "end") {
                buf << s << " ";
                return true;
            }
// ����
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
// ��д
            else {
// ����
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
