#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include "helper.h"
#include "system.h"
#include "global.h"

struct shm_info common_shm;
struct shm_info ptable_shm;
struct shm_info input_shm;

void cleanup();
int cleanup_disabled = 0;
void (*cleanup_hdlr)() = cleanup;

void cleanup() {
    printf("%d stat: %d\n", getpid(), cleanup_disabled);
    if (cleanup_disabled)
        return;

    pid_t target_proc = getpid();
    pid_t *common_proc;
    struct task *p;

    for (int i = EXIT_TMOUT; i > 0; i--) {
        system_d("tmux display-message -t MiniOS:0 'Shutting Down... %d'", i);
        sleep(1);
    }

    // Kill all subprocess
    for_each_until(p, ptable->proc, ptable->proc_cnt)
        kill(p->pid, SIGTERM);

    // Kill common os process
    for_each(common_proc, common->pids)
        if (*common_proc != target_proc) 
            kill(*common_proc, SIGTERM);
            
    system_d("tmux kill-session -t MiniOS 2>/dev/null");
}

void init_global() {
    common_shm = create_shm("COMMON", COMMON_SZ);
    ptable_shm = create_shm("PTABLE", PTABLE_SZ);
    input_shm = create_shm("INPUT", BUF_SZ);
    struct os_args args;
    
    COMMON = &common_shm;
    PTABLE = &ptable_shm;
    INPUT = &input_shm;

    args = prepare_args();
    parse_args(&args);
}

void register_system(pid_t pid, void *arg) {
    sem_wait(common_sem);
    common->pids[common->cnt++] = pid;
    sem_post(common_sem);
}

void init_tui() {
    // Parse args and Register Exit Trap
    struct os_args args = prepare_args();
    atexit(cleanup);

    system_d("tmux kill-session -t MiniOS 2>/dev/null");
    system_d("tmux new-session -d -s MiniOS");
    system_d("tmux set status-position top");
    system_d("tmux set mouse on");

    system_d("tmux split-window -h -t MiniOS:0.0");
    spawn_proc(os_input, register_system, &args);
    system_d("tmux kill-pane -t MiniOS:0.0");
    spawn_proc(os_core, register_system, &args);
    
    spawn_proc(os_status, register_system, &args);
    system_d("tmux kill-pane -t MiniOS:0.2");
    resize_panes();

    // Disable cleanup for root proc
    cleanup_disabled = 1;
}

int main() {
    /*
    int ptrace_scope;
    shell("cat /proc/sys/kernel/yama/ptrace_scope", "%d", &ptrace_scope);

    if (ptrace_scope != 0) {
        printf("PTRACE NOT READY. Please run following with sudo.\n");
        printf("echo 0 > /proc/sys/kernel/yama/ptrace_scope\n");
        exit(EXIT_FAILURE);
    }
    */

    init_global();
    // set common_sem to 1
    sem_post(common_sem);
    init_tui();

    system_d("echo 'Mini OS' | pv -qL 15");
    system_d("echo 'Booting up:' | pv -qL 15");
    system_d("echo '###################' | pv -qL 15");
    system_d("tmux attach");
    return 0;
}

