#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

void minisystem();
void whoami();

// extra/command.c
int command(char *cmd);
int ptrace_command(char *cmd);

// extra/ipc.c
int ipc_test();

// extra/ipc.c
int calc_pi();

// extra/rr.c
#define TASK_CNT 32
struct task {
    char id[10];
    int time_arrival; // 도착한 시간
    int time_total;   // 작업에 필요한 시간
    int time_remain;  // 남은 시간
    int time_wait;    // 대기한 시간 
    int time_spent;   // 소모한 시간 
};

struct proc_table {
    int proc_cnt;
    int execution_order[TASK_CNT];
    struct task proc[TASK_CNT];
};

void register_proc(struct proc_table *ptable, char *id, int arrival_time, int total_time);
void iterate_ptable(struct proc_table *ptable, void (*func)(), void *arg);
void rr_sched(struct proc_table *ptable);

#endif
