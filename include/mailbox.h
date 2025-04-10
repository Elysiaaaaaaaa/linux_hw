////
//// Created by elysia on 2025/4/8.
////
//
//#ifndef LINUX_HW_MAILBOX_H
//#define LINUX_HW_MAILBOX_H
//#include "ipc.h"
//#include "mp.h"
//
//using namespace  std;
//
//
//class mailbox {
//private:
//    int semid;
//    int shmid;
//    int total_number_of_mailboxes;
//    int memory_segment_size;
//    char* shared_memory;
//    int* reader_counts; // 每个邮箱的读者计数
//    int reader_count_semid; // 用于保护读者计数的信号量
//
//public:
//    mailbox(int num_mailboxes, int mem_size) : total_number_of_mailboxes(num_mailboxes), memory_segment_size(mem_size) {
//        // 创建信号量,初始化为 1
//        key_t semkey = Ftok(".", 's');
//        semid = sem_get(semkey, num_mailboxes, true, 1);
//
//        // 创建用于保护读者计数的信号量，初始化为 1
//        key_t reader_count_semkey = Ftok(".", 'r');
//        reader_count_semid = sem_get(reader_count_semkey, num_mailboxes, true, 1);
//
//        // 创建共享内存
//        key_t shmkey = Ftok(".", 'm');
//        shmid = shm_init(shmkey, num_mailboxes * mem_size + num_mailboxes * sizeof(int), IPC_CREAT | 0666);
//
//        // 附加共享内存
//        shared_memory = static_cast<char*>(shm_conn(shmid));
//
//        reader_counts = reinterpret_cast<int*>(shared_memory + num_mailboxes * mem_size);
//        memset(reader_counts, 0, num_mailboxes * sizeof(int));
//    }
//
//    ~mailbox() {
//        if (shmdt(shared_memory) == -1) {
//            perror("shmdt");
//        }
//        if (semctl(semid, 0, IPC_RMID) == -1) {
//            perror("semctl IPC_RMID");
//        }
//        if (semctl(reader_count_semid, 0, IPC_RMID) == -1) {
//            perror("semctl reader_count_semid IPC_RMID");
//        }
//        if (shmctl(shmid, IPC_RMID, NULL) == -1) {
//            perror("shmctl IPC_RMID");
//        }
//    }
//
//    void readMailbox(int mailbox_index, char* buffer) {
//        // 保护读者计数的更新
//        Wait(reader_count_semid, mailbox_index);
//        reader_counts[mailbox_index]++;
//        if (reader_counts[mailbox_index] == 1) {
//            // 如果是第一个读者，获取邮箱的信号量
//            Wait(semid, mailbox_index);
//        }
//        Signal(reader_count_semid, mailbox_index);
//
//        memcpy(buffer, shared_memory + mailbox_index * memory_segment_size, memory_segment_size);
//
//        // 保护读者计数的更新
//        Wait(reader_count_semid, mailbox_index);
//        reader_counts[mailbox_index]--;
//        if (reader_counts[mailbox_index] == 0) {
//            // 如果是最后一个读者，释放邮箱的信号量
//            Signal(semid, mailbox_index);
//        }
//        Signal(reader_count_semid, mailbox_index);
//    }
//
//    void writeMailbox(int mailbox_index, const char* data) {
//        size_t data_len = strlen(data);
//        size_t copy_len = (data_len > static_cast<size_t>(memory_segment_size)) ? static_cast<size_t>(memory_segment_size) : data_len;
//        // 获取邮箱的信号量
//        Wait(semid, mailbox_index);
//        memcpy(shared_memory + mailbox_index * memory_segment_size, data, copy_len);
//        // 如果截断了数据，可添加结束符
//        if (copy_len < data_len) {
//            shared_memory[mailbox_index * memory_segment_size + copy_len - 1] = '\0';
//        }
//        // 释放邮箱的信号量
//        Signal(semid, mailbox_index);
//    }
//};
//
//#endif //LINUX_HW_MAILBOX_H
//
//
//
//
//
//
//
//
