//
// Created by elysia on 2025/4/4.
//

#include "../include/mp.h"

//
// Created by elysia on 2025/4/4.
//


pid_t Fork(){
    pid_t pid = fork();
    if(pid<0){
        Logger::log(LogLevel::ERROR,"ERROR:Fork.fork");
        exit(EXIT_FAILURE);
    }
    return pid;
}

pid_t Wait(int * stat_loc){
//    stat_loc:·µ»ØÐÅÏ¢
    pid_t pid = wait(stat_loc);
    return pid;
}

