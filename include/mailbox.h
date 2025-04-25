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
    int* reader_counts; // ÿ������Ķ��߼���
    int reader_count_semid; // ���ڱ������߼������ź���

public:
    mailbox(int num_mailboxes, int mem_size, int proj_id, const char *pathname) : total_number_of_mailboxes(num_mailboxes), memory_segment_size(mem_size) {
        // �����ź�������ʼֵΪ 1
        key_t semkey = ftok(pathname, proj_id + PROJ_SEMKEY_KEY_OFFSET);
        semid = sem_get(semkey, num_mailboxes, true, 1);

        // �������ڱ������߼������ź�������ʼֵΪ 1
        key_t reader_count_semkey = ftok(pathname, proj_id + PROJ_READER_KEY_OFFSET);
        reader_count_semid = sem_get(reader_count_semkey, num_mailboxes, true, 1);

        // ���������ڴ�
        key_t shmkey = ftok(pathname, proj_id + PROJ_MEMORY_KEY_OFFSET);
        shmid = shm_init(shmkey, num_mailboxes * mem_size + num_mailboxes * sizeof(int), IPC_CREAT | 0666);

        // ���ӹ����ڴ�
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
        // ���Ӷ��߼���
        Wait(reader_count_semid, mailbox_index);
        reader_counts[mailbox_index]++;
        if (reader_counts[mailbox_index] == 1) {
            // ����ǵ�һ�����ߣ���ȡ������ź���
            Wait(semid, mailbox_index);
        }
        Signal(reader_count_semid, mailbox_index);
        auto current_time = std::chrono::high_resolution_clock::now();
        auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count();
        if (elapsed_time < op_time) {
            usleep((op_time - elapsed_time) * 1000); // ʹ�� usleep �ȴ�ʣ��ĺ�����
        }
        // ����Ƿ����ַ��ɶ�
        int read_offset = reader_counts[mailbox_index + total_number_of_mailboxes];
        if (read_offset >= memory_segment_size) {
            std::cout << "End of MailBox" << std::endl;
        } else {
            int copy_len = (length > memory_segment_size - read_offset) ? memory_segment_size - read_offset : length;
            memcpy(buffer, shared_memory + mailbox_index * memory_segment_size + read_offset, copy_len);
            buffer[copy_len] = '\0';
            reader_counts[mailbox_index + total_number_of_mailboxes] += copy_len;
        }

        // ���ٶ��߼���
        Wait(reader_count_semid, mailbox_index);
        reader_counts[mailbox_index]--;
        if (reader_counts[mailbox_index] == 0) {
            // ��������һ�����ߣ��ͷ�������ź���
            Signal(semid, mailbox_index);
        }
        Signal(reader_count_semid, mailbox_index);
    }

    void writeMailbox(int mailbox_index, const char* data, int op_time, const std::chrono::time_point<std::chrono::high_resolution_clock>& start_time) {
        size_t data_len = strlen(data);
        size_t copy_len = (data_len > static_cast<size_t>(memory_segment_size)) ? static_cast<size_t>(memory_segment_size) : data_len;
        // ��ȡ������ź���
        Wait(semid, mailbox_index);
        auto current_time = std::chrono::high_resolution_clock::now();
        auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count();
        if (elapsed_time < op_time) {
            usleep((op_time - elapsed_time) * 1000); // ʹ�� usleep �ȴ�ʣ��ĺ�����
        }
        memcpy(shared_memory + mailbox_index * memory_segment_size, data, copy_len);
        // �ض϶�������ݣ������ֹ��
        if (copy_len < data_len) {
            shared_memory[mailbox_index * memory_segment_size + copy_len - 1] = '\0';
        }
        // ���ö�ָ��
        reader_counts[mailbox_index + total_number_of_mailboxes] = 0;
        // �ͷ�������ź���
        Signal(semid, mailbox_index);
    }
};
#endif //LINUX_HW_MAILBOX_H








