#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>


int command(char *cmd) {
    int result;
    pid_t pid;

    pid = fork();
    if (pid < 0) {
        printf("fork failed");
        return -1;
    }
    else if (pid == 0) {
        // 명령어를 실행할 때마다 fork를 수행한 후 실행하도록 변경
        result = execlp(cmd, cmd, NULL);
        if (result < 0) {
            printf("command error: %s\n", strerror(errno));
        }
    }
    else {
        result = wait(NULL);
    }

    return result;
}