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
    mailbox(int num_mailboxes, int mem_size, int proj_id, const char *pathname) : total_number_of_mailboxes(num_mailboxes), memory_segment_size(mem_size) {
        // 创建信号量，初始值为 1
        key_t semkey = ftok(pathname, proj_id + PROJ_SEMKEY_KEY_OFFSET);
        semid = sem_get(semkey, num_mailboxes, true, 1);

        // 创建用于保护读者计数的信号量，初始值为 1
        key_t reader_count_semkey = ftok(pathname, proj_id + PROJ_READER_KEY_OFFSET);
        reader_count_semid = sem_get(reader_count_semkey, num_mailboxes, true, 1);

        // 创建共享内存
        key_t shmkey = ftok(pathname, proj_id + PROJ_MEMORY_KEY_OFFSET);
        shmid = shm_init(shmkey, num_mailboxes * mem_size + num_mailboxes * sizeof(int), IPC_CREAT | 0666);

        // 连接共享内存
        shared_memory = static_cast<char*>(shm_conn(shmid));

        reader_counts = reinterpret_cast<int*>(shared_memory + num_mailboxes * mem_size);
        memset(reader_counts, 0, num_mailboxes * sizeof(int));
    }

    ~mailbox() {
        if (shmdt(shared_memory) == -1) {
            perror("shmdt");
        }
        if (semctl(semid, 0, IPC_RMID) == -1) {
            perror("semctl IPC_RMID");
        }
        if (semctl(reader_count_semid, 0, IPC_RMID) == -1) {
            perror("semctl reader_count_semid IPC_RMID");
        }
        if (shmctl(shmid, IPC_RMID, NULL) == -1) {
            perror("shmctl IPC_RMID");
        }
    }

    void readMailbox(int mailbox_index, char* buffer, int length, int op_time, const std::chrono::time_point<std::chrono::high_resolution_clock>& start_time) {
        // 增加读者计数
        Wait(reader_count_semid, mailbox_index);
        reader_counts[mailbox_index]++;
        if (reader_counts[mailbox_index] == 1) {
            // 如果是第一个读者，获取邮箱的信号量
            Wait(semid, mailbox_index);
        }
        Signal(reader_count_semid, mailbox_index);
        auto current_time = std::chrono::high_resolution_clock::now();
        auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count();
        if (elapsed_time < op_time) {
            usleep((op_time - elapsed_time) * 1000); // 使用 usleep 等待剩余的毫秒数
        }
        // 检查是否还有字符可读
        int read_offset = reader_counts[mailbox_index + total_number_of_mailboxes];
        if (read_offset >= memory_segment_size) {
            std::cout << "End of MailBox" << std::endl;
        } else {
            int copy_len = (length > memory_segment_size - read_offset) ? memory_segment_size - read_offset : length;
            memcpy(buffer, shared_memory + mailbox_index * memory_segment_size + read_offset, copy_len);
            buffer[copy_len] = '\0';
            reader_counts[mailbox_index + total_number_of_mailboxes] += copy_len;
        }

        // 减少读者计数
        Wait(reader_count_semid, mailbox_index);
        reader_counts[mailbox_index]--;
        if (reader_counts[mailbox_index] == 0) {
            // 如果是最后一个读者，释放邮箱的信号量
            Signal(semid, mailbox_index);
        }
        Signal(reader_count_semid, mailbox_index);
    }

    void writeMailbox(int mailbox_index, const char* data, int op_time, const std::chrono::time_point<std::chrono::high_resolution_clock>& start_time) {
        size_t data_len = strlen(data);
        size_t copy_len = (data_len > static_cast<size_t>(memory_segment_size)) ? static_cast<size_t>(memory_segment_size) : data_len;
        // 获取邮箱的信号量
        Wait(semid, mailbox_index);
        auto current_time = std::chrono::high_resolution_clock::now();
        auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count();
        if (elapsed_time < op_time) {
            usleep((op_time - elapsed_time) * 1000); // 使用 usleep 等待剩余的毫秒数
        }
        memcpy(shared_memory + mailbox_index * memory_segment_size, data, copy_len);
        // 截断多余的数据，添加终止符
        if (copy_len < data_len) {
            shared_memory[mailbox_index * memory_segment_size + copy_len - 1] = '\0';
        }
        // 重置读指针
        reader_counts[mailbox_index + total_number_of_mailboxes] = 0;
        // 释放邮箱的信号量
        Signal(semid, mailbox_index);
    }
};
#endif //LINUX_HW_MAILBOX_H








