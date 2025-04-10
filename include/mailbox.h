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
//    int* reader_counts; // ÿ������Ķ��߼���
//    int reader_count_semid; // ���ڱ������߼������ź���
//
//public:
//    mailbox(int num_mailboxes, int mem_size) : total_number_of_mailboxes(num_mailboxes), memory_segment_size(mem_size) {
//        // �����ź���,��ʼ��Ϊ 1
//        key_t semkey = Ftok(".", 's');
//        semid = sem_get(semkey, num_mailboxes, true, 1);
//
//        // �������ڱ������߼������ź�������ʼ��Ϊ 1
//        key_t reader_count_semkey = Ftok(".", 'r');
//        reader_count_semid = sem_get(reader_count_semkey, num_mailboxes, true, 1);
//
//        // ���������ڴ�
//        key_t shmkey = Ftok(".", 'm');
//        shmid = shm_init(shmkey, num_mailboxes * mem_size + num_mailboxes * sizeof(int), IPC_CREAT | 0666);
//
//        // ���ӹ����ڴ�
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
//        // �������߼����ĸ���
//        Wait(reader_count_semid, mailbox_index);
//        reader_counts[mailbox_index]++;
//        if (reader_counts[mailbox_index] == 1) {
//            // ����ǵ�һ�����ߣ���ȡ������ź���
//            Wait(semid, mailbox_index);
//        }
//        Signal(reader_count_semid, mailbox_index);
//
//        memcpy(buffer, shared_memory + mailbox_index * memory_segment_size, memory_segment_size);
//
//        // �������߼����ĸ���
//        Wait(reader_count_semid, mailbox_index);
//        reader_counts[mailbox_index]--;
//        if (reader_counts[mailbox_index] == 0) {
//            // ��������һ�����ߣ��ͷ�������ź���
//            Signal(semid, mailbox_index);
//        }
//        Signal(reader_count_semid, mailbox_index);
//    }
//
//    void writeMailbox(int mailbox_index, const char* data) {
//        size_t data_len = strlen(data);
//        size_t copy_len = (data_len > static_cast<size_t>(memory_segment_size)) ? static_cast<size_t>(memory_segment_size) : data_len;
//        // ��ȡ������ź���
//        Wait(semid, mailbox_index);
//        memcpy(shared_memory + mailbox_index * memory_segment_size, data, copy_len);
//        // ����ض������ݣ�����ӽ�����
//        if (copy_len < data_len) {
//            shared_memory[mailbox_index * memory_segment_size + copy_len - 1] = '\0';
//        }
//        // �ͷ�������ź���
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
