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

    printf("Received input: %s\n", input_buf);
    if (EQ(input_buf, "exit"))
      exit(EXIT_SUCCESS);
    else if (EQ(input_buf, "pi")) {
      handler = calc_pi;
      arg = "calc_pi";
    }
    else if (EQ(input_buf, "rr")) {
      struct proc_table ptable = {};

      // task_register(&ptable, "P1", 10, 10);
      // task_register(&ptable, "P2", 5, 5);
      // task_register(&ptable, "P3", 8, 8);
      // task_register(&ptable, "P4", 3, 3);
      // task_register(&ptable, "P5", 6, 6);
      // task_register(&ptable, "P6", 7, 7);
      // task_register(&ptable, "P7", 4, 4);
      // task_register(&ptable, "P8", 9, 9);

      // rr_sched(&ptable);
    }

    cleanup_disabled = 1;
    spawn_proc(handler, manage_proc, arg);
    cleanup_disabled = 0;

    resize_panes();
}


void loop_handler(int sig) {
    static int iter = 0;
    printf("Sched iteration: %d\n", ++iter);

    // handle pending input
    handle_input();
    sched_handler(ptable);
}


void os_core(struct os_args *args) {
    parse_args(args);

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
