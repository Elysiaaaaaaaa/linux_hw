//
// Created by elysia on 2025/4/8.
//

#ifndef LINUX_HW_MAILBOX_H
#define LINUX_HW_MAILBOX_H
#include "ipc.h"
#include "mp.h"
#include <iostream>
#include <cstring>
#include <vector>
#include <chrono>
#define PROJ_SEMKEY_KEY_OFFSET 5
#define PROJ_READER_KEY_OFFSET 6
#define PROJ_MEMORY_KEY_OFFSET 7
using namespace std;
class mailbox {
private:
    int semid;
    int shmid;
    int total_number_of_mailboxes;
    int memory_segment_size;
    char* shared_memory;
    int* reader_counts; // 每个邮箱的读者计数
    int reader_count_semid; // 用于保护读者计数的信号量

public:
    mailbox(int num_mailboxes, int mem_size, int proj_id, const char *pathname);
    ~mailbox();
    void readMailbox(int mailbox_index, std::string& result, int op_time, const std::chrono::time_point<std::chrono::high_resolution_clock>& start_time);
    void writeMailbox(int mailbox_index, string data, int op_time, const std::chrono::time_point<std::chrono::high_resolution_clock>& start_time);
    void show();
    void read_one(int i);
};
#endif //LINUX_HW_MAILBOX_H








