#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <unistd.h>
#include <errno.h>

#include "debug.h"

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


int ptrace_command(char *cmd) {
    int result = 0;
    pid_t pid;

    pid = fork();
    if (pid < 0) {
        printf("fork failed");
        return -1;
    }
    else if (pid == 0) {
        // 명령어를 실행할 때마다 fork를 수행한 후 실행하도록 변경
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        result = execlp(cmd, cmd, NULL);
        if (result < 0) {
            printf("command error: %s\n", strerror(errno));
        }
    }
    else {
        struct user_regs_struct regs;
        int status;

        // execve가 성공해서 child가 대기중임
        waitpid(pid, &status, 0);

        // PTRACE는 syscall시마다 SIGTRAP 시그널을 리턴함
        while (!WIFEXITED(status)) {
            // child 동작을 재개(시작)하고 다음 syscall에서 대기하도록 지정
            if (ptrace(PTRACE_SYSCALL, pid, NULL, NULL) == -1) {
                perror("ptrace");
                return 1;
            }

            // child에서 syscall 호출했지만 수행하지는 않고 대기중임
            waitpid(pid, &status, 0);

            if (WIFSTOPPED(status)) {
                // 요청한 syscall 번호 수집
                ptrace(PTRACE_GETREGS, pid, NULL, &regs);
                long syscall_num = regs.orig_rax;
                const char * syscall_name = syscall_names[syscall_num].name;
                int syscall_nargs = syscall_names[syscall_num].nargs;

                if (syscall_num != -1) {
                    printf("%s(", syscall_name);

                    // 요청한 syscall 인자 수집
                    for (int i = 0; i < syscall_nargs; i++) {
                        printf("%llx,", regs.rdi + i * 8);
                    }

                    // child에서 대기중인 syscall 동작 시작
                    if (ptrace(PTRACE_SYSCALL, pid, NULL, NULL) == -1) {
                        perror("ptrace");
                        return 1;
                    }

                    // child에서 syscall 완료했고 대기중임
                    waitpid(pid, &status, 0);
                    if (WIFSTOPPED(status)) {
                        // client의 syscall 리턴 수집
                        ptrace(PTRACE_GETREGS, pid, NULL, &regs);
                        printf(") = %lld\n", regs.rax);
                    }
                }
            }
        }

    }

    return result;
}