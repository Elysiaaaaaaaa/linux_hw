//
// Created by elysia on 2025/4/8.
//

#include "../include/mailbox.h"
// ��ȡ�ź���
void Mailbox::semaphore_p() {
    struct sembuf sb = {0, -1, 0};  // -1 ��ʾ P �����������ź���
    semop(mutex_, &sb, 1);
}

void Mailbox::semaphore_v() {
    struct sembuf sb = {0, 1, 0};   // 1 ��ʾ V �������ͷ��ź���
    semop(mutex_, &sb, 1);
}

Mailbox::Mailbox(key_t shm_key, key_t mutex_key, int total_number_of_mailboxes, bool init)
        : total_number_of_mailboxes(total_number_of_mailboxes) {
    this->shm_key = shm_key;
    this->mutex_key = mutex_key;

    // ���������ڴ�
    shmid = shmget(shm_key, sizeof(MailMessage) * total_number_of_mailboxes, IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("Mailbox.shmget failed");
        exit(EXIT_FAILURE);
    }

    // �������ڴ�ӳ�䵽���̵ĵ�ַ�ռ�
    messages = (MailMessage *)shmat(shmid, nullptr, 0);
    if (messages == (void *)-1) {
        perror("Mailbox.shmat failed");
        exit(EXIT_FAILURE);
    }

    // ��ȡ�������ź���
    mutex_ = semget(mutex_key, 1, IPC_CREAT | 0666);
    if (mutex_ == -1) {
        perror("Mailbox.semget failed");
        exit(EXIT_FAILURE);
    }

    if (init) {
        // ��ʼ���ź���Ϊ1����ʾ���⣩
        union semun arg;
        arg.val = 1;
        semctl(mutex_, 0, SETVAL, arg);

        // ��ʼ�������ڴ��е�ÿ����
        for (int i = 0; i < total_number_of_mailboxes; ++i) {
            messages[i].from_car_id = -1;  // ���Ϊ��
        }
    }
}

Mailbox::~Mailbox() {
    shmdt(messages);  // ���빲���ڴ�
}

void Mailbox::write(int car_id, const char *msg) {
    semaphore_p();  // ��ȡ��

    // ���ҿղۣ���δʹ�ù��Ĳۣ�
    for (int i = 0; i < total_number_of_mailboxes; ++i) {
        if (messages[i].from_car_id == -1) {
            messages[i].from_car_id = car_id;
            strncpy(messages[i].msg, msg, MAX_MSG_LEN - 1);
            break;
        }
    }

    semaphore_v();  // �ͷ���
}

std::vector<MailMessage> Mailbox::read_all() {
    std::vector<MailMessage> result;
    semaphore_p();  // ��ȡ��

    // ��ȡ������Ч��Ϣ
    for (int i = 0; i < total_number_of_mailboxes; ++i) {
        if (messages[i].from_car_id != -1) {
            result.push_back(messages[i]);
        }
    }

    semaphore_v();  // �ͷ���
    return result;
}

// ��ȡ��Ӧ��ID���ʼ���
MailMessage* Mailbox::get_message_slot(int car_id) {
    semaphore_p();  // ��ȡ��

    for (int i = 0; i < total_number_of_mailboxes; ++i) {
        if (messages[i].from_car_id == car_id) {
            semaphore_v();
            return &messages[i];
        }
    }

    semaphore_v();  // �ͷ���
    return nullptr;  // ���δ�ҵ������ؿ�ָ��
}
