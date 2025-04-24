//
// Created by elysia on 2025/4/8.
//

#include "../include/mailbox.h"
// 获取信号量
void Mailbox::semaphore_p() {
    struct sembuf sb = {0, -1, 0};  // -1 表示 P 操作，锁定信号量
    semop(mutex_, &sb, 1);
}

void Mailbox::semaphore_v() {
    struct sembuf sb = {0, 1, 0};   // 1 表示 V 操作，释放信号量
    semop(mutex_, &sb, 1);
}

Mailbox::Mailbox(key_t shm_key, key_t mutex_key, int total_number_of_mailboxes, bool init)
        : total_number_of_mailboxes(total_number_of_mailboxes) {
    this->shm_key = shm_key;
    this->mutex_key = mutex_key;

    // 创建共享内存
    shmid = shmget(shm_key, sizeof(MailMessage) * total_number_of_mailboxes, IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("Mailbox.shmget failed");
        exit(EXIT_FAILURE);
    }

    // 将共享内存映射到进程的地址空间
    messages = (MailMessage *)shmat(shmid, nullptr, 0);
    if (messages == (void *)-1) {
        perror("Mailbox.shmat failed");
        exit(EXIT_FAILURE);
    }

    // 获取互斥锁信号量
    mutex_ = semget(mutex_key, 1, IPC_CREAT | 0666);
    if (mutex_ == -1) {
        perror("Mailbox.semget failed");
        exit(EXIT_FAILURE);
    }

    if (init) {
        // 初始化信号量为1（表示互斥）
        union semun arg;
        arg.val = 1;
        semctl(mutex_, 0, SETVAL, arg);

        // 初始化共享内存中的每个槽
        for (int i = 0; i < total_number_of_mailboxes; ++i) {
            messages[i].from_car_id = -1;  // 标记为空
        }
    }
}

Mailbox::~Mailbox() {
    shmdt(messages);  // 分离共享内存
}

void Mailbox::write(int car_id, const char *msg) {
    semaphore_p();  // 获取锁

    // 查找空槽（从未使用过的槽）
    for (int i = 0; i < total_number_of_mailboxes; ++i) {
        if (messages[i].from_car_id == -1) {
            messages[i].from_car_id = car_id;
            strncpy(messages[i].msg, msg, MAX_MSG_LEN - 1);
            break;
        }
    }

    semaphore_v();  // 释放锁
}

std::vector<MailMessage> Mailbox::read_all() {
    std::vector<MailMessage> result;
    semaphore_p();  // 获取锁

    // 读取所有有效消息
    for (int i = 0; i < total_number_of_mailboxes; ++i) {
        if (messages[i].from_car_id != -1) {
            result.push_back(messages[i]);
        }
    }

    semaphore_v();  // 释放锁
    return result;
}

// 获取对应车ID的邮件槽
MailMessage* Mailbox::get_message_slot(int car_id) {
    semaphore_p();  // 获取锁

    for (int i = 0; i < total_number_of_mailboxes; ++i) {
        if (messages[i].from_car_id == car_id) {
            semaphore_v();
            return &messages[i];
        }
    }

    semaphore_v();  // 释放锁
    return nullptr;  // 如果未找到，返回空指针
}
