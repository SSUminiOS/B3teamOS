#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "helper.h"
#include "global.h"

void handle_sigint(int sig) {
    printf("\nCtrl-C pressed. Press 'Ctrl-D' to quit.\n");
    rl_on_new_line();
    rl_replace_line("", 0);
    rl_redisplay();
}

void os_input(struct os_args *args) {
    char *input, *token;
    parse_args(args);

    signal(SIGINT, handle_sigint);

    while (1) {
        input = readline("Enter command('exit' to quit) : ");
        if (input == NULL) {
            printf("Ctrl-D pressed. Exiting.\n");
            break;
        }
        if (is_invalid(input))
            continue;

        // 빈 입력이 아닐 경우에만
        if (*input)
            add_history(input);

        strcpy(input_buf, input);
        sem_post(input_sem);
        // Core가 바로 응답하게 하기 위해서 signal 전송
        // kill(common->pids[OS_CORE], SIGUSR1);
    }
}