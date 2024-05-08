#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "system.h"
void print_minios(char* str);

int main() {
    print_minios("[MiniOS SSU] Hello, World!");

    char *input;

    while(1) {
        // readline을 사용하여 입력 받기
        input = readline("커맨드를 입력하세요(종료:exit) : ");

        if (strcmp(input,"exit") == 0) {
            break;
        }

        if (strcmp(input,"minisystem") == 0){
            minisystem();
        }
        else if (strcmp(input,"whoami") == 0){
            whoami();
        }
        else if (strcmp(input,"ipc") == 0){
            ipc_test();
	}
        else if (strncmp(input, "ptrace", strlen("ptrace")) == 0) {
            char* debug_input = input + strlen("ptrace") + 1;
            ptrace_command(debug_input);
        }
        // 명령어를 실행할 때마다 fork를 수행한 후 실행하도록 변경
        else command(input);
    }

    // 메모리 해제
    free(input);
    print_minios("[MiniOS SSU] MiniOS Shutdown........");

    return(1);
}

void print_minios(char* str) {
        printf("%s\n",str);
}
