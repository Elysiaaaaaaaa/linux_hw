//
// Created by elysia on 2025/4/5.
//

#include "../include/Car.h"
#include <cstdlib>
#include <cstring>
#include <iostream>

// Constructor
Car::Car(int proj_id, const std::string& path, int shm_size, int car_id, Direction dir)
        : key_(0), semid_(-1), shmid_(-1), shmaddr_(nullptr), car_id_(car_id), direction_(dir)
{
    // Generate IPC key
    key_ = Ftok(proj_id, path.c_str());

    // Get or create a semaphore set with 1 semaphore, initialize to 1
    semid_ = sem_get(key_, 1, true, 1);

    // Get or create shared memory
    shmid_ = shm_get(key_, shm_size, IPC_CREAT | 0666);

    // Attach to shared memory
    shmaddr_ = shm_conn(shmid_);
}

// Destructor
Car::~Car()
{
    if (shmaddr_) {
        shm_disconn(shmaddr_);
        shmaddr_ = nullptr;
    }
    if (semid_ != -1) {
        sem_del(semid_);
    }
    if (shmid_ != -1) {
        if (shmctl(shmid_, IPC_RMID, nullptr) == -1) {
            Logger::log(LogLevel::ERROR, "~Car.shmctl");
            exit(EXIT_FAILURE);
        }
    }
}

// Request access (P operation)
void Car::enter()
{
    Logger::log(LogLevel::INFO, "[Car " + std::to_string(car_id_) + " (" + getDirectionStr() + ")] wants to enter.");
    Wait(semid_, 0);
    Logger::log(LogLevel::INFO, "[Car " + to_string(car_id_) + " (" + getDirectionStr() + ")] entered.");
}

// Release access (V operation)
void Car::leave()
{
    Logger::log(LogLevel::INFO, "[Car " + to_string(car_id_) + " (" + getDirectionStr() + ")] is leaving.");
    Signal(semid_, 0);
}

// Get pointer to shared memory
void* Car::getSharedMemory()
{
    return shmaddr_;
}

// Get car id
int Car::getCarId() const
{
    return car_id_;
}

// Get direction
Direction Car::getDirection() const
{
    return direction_;
}

// Get direction as string
std::string Car::getDirectionStr() const
{
    return (direction_ == Direction::Eastbound) ? "Eastbound" : "Westbound";
}
