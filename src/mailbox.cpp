//
// Created by elysia on 2025/4/8.
//

#include "mailbox.h"
// ��ȡ�ź���

mailbox::mailbox(int num_mailboxes, int mem_size, int proj_id, const char *pathname) : total_number_of_mailboxes(num_mailboxes), memory_segment_size(mem_size) {
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

mailbox::~mailbox() {
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

void mailbox::readMailbox(int mailbox_index, std::string& result, int op_time, const std::chrono::time_point<std::chrono::high_resolution_clock>& start_time) {
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
        result.clear();
    } else {
        int copy_len = memory_segment_size - read_offset;
        // �ҵ�ʵ�ʵ��ַ�������
        int actual_len = 0;
        while (actual_len < copy_len && shared_memory[mailbox_index * memory_segment_size + read_offset + actual_len] != '\0') {
            actual_len++;
        }
        result.assign(shared_memory + mailbox_index * memory_segment_size + read_offset, actual_len);
        reader_counts[mailbox_index + total_number_of_mailboxes] += actual_len;
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

void mailbox::writeMailbox(int mailbox_index, string data, int op_time, const std::chrono::time_point<std::chrono::high_resolution_clock>& start_time) {
    size_t data_len = data.length();
    size_t copy_len = (data_len > static_cast<size_t>(memory_segment_size)) ? static_cast<size_t>(memory_segment_size) : data_len;
    // ��ȡ������ź���
    Wait(semid, mailbox_index);
    auto current_time = std::chrono::high_resolution_clock::now();
    auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count();
    if (elapsed_time < op_time) {
        usleep((op_time - elapsed_time) * 1000); // ʹ�� usleep �ȴ�ʣ��ĺ�����
    }
    memcpy(shared_memory + mailbox_index * memory_segment_size, data.c_str(), copy_len);
    // �ض϶�������ݣ������ֹ��
    if (copy_len < data_len) {
        shared_memory[mailbox_index * memory_segment_size + copy_len - 1] = '\0';
    }
    // ���ö�ָ��
    reader_counts[mailbox_index + total_number_of_mailboxes] = 0;
    // �ͷ�������ź���
    Signal(semid, mailbox_index);
}

void mailbox::show() {
    for (int i = 0; i < total_number_of_mailboxes; i++) {
        std::string mailboxData;
        // ����Ϊ�˻�ȡ�������ݣ����� readMailbox ����������ֻ��չʾ�������ʱ���������Ϊ 0
        std::chrono::time_point<std::chrono::high_resolution_clock> dummyStartTime = std::chrono::high_resolution_clock::now();
        readMailbox(i, mailboxData, 0, dummyStartTime);

        std::string message = "Mailbox: " + std::to_string(1+i) + " Info: Data: \"" + mailboxData + "\", Reader Count: " + std::to_string(reader_counts[i]);
        Logger::log(LogLevel::INFO, message);
    }
}
