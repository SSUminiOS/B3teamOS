#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>

#define BUF_SZ 1024
#define READ_END 0
#define WRITE_END 1

int ipc_test()
{
    char write_buf[BUF_SZ] = "IPC SUCCESS";
    char read_buf[BUF_SZ];
    int status;
    pid_t pid;
    int fd[2];

    if (pipe(fd) == -1) {
        printf("fork failed");
        return -1;
    }

    pid = fork();
    if (pid < 0) {
        printf("fork failed");
        return -1;
    }
    else if (pid == 0) {
        close(fd[WRITE_END]);
        read(fd[READ_END], read_buf, BUF_SZ);
        printf("IPC TEST: %s\n", read_buf);
        close(fd[READ_END]);
        exit(0);
    }
    else {
        close(fd[READ_END]);
        write(fd[WRITE_END], write_buf, strlen(write_buf) + 1);
        close(fd[WRITE_END]);
        wait(&status);
    }
    return 0;
}