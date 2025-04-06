//
// Created by elysia on 2025/4/4.
//

#ifndef LINUX_HW_MP_H
#define LINUX_HW_MP_H

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include "logger.h"
using namespace std;


pid_t Fork();

pid_t Wait(int * stat_loc = (int*)0);

void Exit(int statues);

#endif //LINUX_HW_MP_H
