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
    int from_car_id;   // 发信车ID
    char msg[MAX_MSG_LEN];  // 消息内容
};

class Mailbox {
private:
    key_t shm_key;                   // 共享内存键值
    int shmid;                        // 共享内存ID
    MailMessage *messages;            // 指向共享内存的指针
    key_t mutex_key;                  // 锁的键值
    int mutex_;                       // 锁的信号量
    int total_number_of_mailboxes;    // 邮箱数量

    // 信号量操作
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








