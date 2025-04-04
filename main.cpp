#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
int a = 0;

int main() {
    pid_t pid = fork();

    if (pid < 0) {
        // fork失败
        perror("fork");
        return 1;
    } else if (pid == 0) {
        // 子进程
        a++;
        printf("子进程中a的值为：%d\n", a);
    } else {
        // 父进程
        a++;
        printf("父进程中a的值为：%d\n", a);
    }

    return 0;
}