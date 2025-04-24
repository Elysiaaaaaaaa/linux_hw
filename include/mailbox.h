//
// Created by elysia on 2025/4/8.
//

#ifndef LINUX_HW_MAILBOX_H
#define LINUX_HW_MAILBOX_H
#include "ipc.h"
#include "mp.h"
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <vector>
#include <cstring>
#include <iostream>

const int MAX_MSG_LEN = 256;

struct MailMessage {
    int from_car_id;   // ���ų�ID
    char msg[MAX_MSG_LEN];  // ��Ϣ����
};

class Mailbox {
private:
    key_t shm_key;                   // �����ڴ��ֵ
    int shmid;                        // �����ڴ�ID
    MailMessage *messages;            // ָ�����ڴ��ָ��
    key_t mutex_key;                  // ���ļ�ֵ
    int mutex_;                       // �����ź���
    int total_number_of_mailboxes;    // ��������

    // �ź�������
    void semaphore_p();
    void semaphore_v();

public:
    Mailbox(key_t shm_key, key_t mutex_key, int total_number_of_mailboxes, bool init);
    ~Mailbox();

    void write(int car_id, const char *msg);
    std::vector<MailMessage> read_all();
    MailMessage* get_message_slot(int car_id);
};


#endif //LINUX_HW_MAILBOX_H








