#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include "primitive.h"
#include "helper.h"
#include "command.h"
#include "system.h"
#include "global.h"

void isolate_proc() {
    pid_t pid = getpid();
    if (setpgid(pid, pid) == -1) {
        perror("setpgid");
        exit(EXIT_FAILURE);
    }
}

// Assure parent start after child signals
void spawn_proc(void (*child)(), void (*parent)(), void *arg) {
    struct shm_info shmem = create_shm(SHM_ORDER, sizeof(int));
    int *order = (int *)shmem.data;
    *order = ORDER_CHILD;
    pid_t pid = fork();

    if (pid < 0) {
        perror("Initialize Error");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        isolate_proc();
        *order = ORDER_PARENT;
        kill(pid, SIGTSTP);

        if (child) child(arg);
    } else {
        while (*order == ORDER_PARENT);
        reptyr_proc(pid);

        if (parent) parent(pid, arg);
    }

    destroy_shm(&shmem);
}

void os_spawn(char *cmd) {
    char *args[BUF_SZ];
    int ret;

    parse_cmd(cmd, args);
    ret = execvp(args[0], args);
    if (ret != 0) {
        printf("'%s' %s\n", cmd, strerror(errno));
    }
}

void manage_proc(pid_t pid, char *cmd) {
    struct task *p;
    sem_wait(ptable_sem);
    p = proc_register(ptable, pid, cmd);
    p = proc_set_tmux_id(p, count_panes() - 1);
    sem_post(ptable_sem);
}

void handle_input() {
    void (*handler)() = os_spawn;
    void *arg = (void *)input_buf;
    if (sem_trywait(input_sem) == -1)
        return;

    printf("\nReceived input: %s\n", input_buf);
    if (EQ(input_buf, "exit"))
      exit(EXIT_SUCCESS);
    else if (EQ(input_buf, "pi")) {
      handler = calc_pi;
      arg = "calc_pi";
    }

    cleanup_disabled = 1;
    spawn_proc(handler, manage_proc, arg);
    cleanup_disabled = 0;

    resize_panes();
}

void check_rq_done(char *line) {
    pid_t pid = atoi(line);
    if (pid != 0)
        proc_deregister(ptable, pid);
}

void loop_handler(int sig) {
    static int iter = 0;
    clear_line();
    printf("\rSched iteration: %d", ++iter);
    fflush(stdout);

    // check if any exit process
    // if RQ_DONE_FILE not exist, skip
    if (access(RQ_DONE_FILE, F_OK) == 0) {
        read_file(RQ_DONE_FILE, check_rq_done);
        unlink(RQ_DONE_FILE);
    }

    // handle pending input
    handle_input();

    sched_handler(ptable);
}

// 시그널 핸들러 함수
void signal_handler(int signum) {
    printf("Received signal: %d (%s)\n", signum, strsignal(signum));
}

void os_core(struct os_args *args) {
    parse_args(args);
    unlink(RQ_DONE_FILE);

    // Initialize ptable
    init_ptable(ptable);
    sem_post(ptable_sem);

    init_timer(loop_handler, SIGRTMIN, CLOCK);
    // Input 이벤트 즉시 처리
    // signal(SIGUSR1, handle_input);
    signal(SIGWINCH, resize_panes);
    signal(SIGINT, cleanup_hdlr);

    printf("Welecome to Mini OS\n");

    while (1)
        sleep(1);
}
