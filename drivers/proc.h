#ifndef __PROC_H
#define __PROC_H

#include "primitive.h"

#define TIME_SLICE              3

#define PRIO_DEFAULT            20

#define TASK_RUNNABLE           0
#define TASK_RUNNING            1
#define TASK_EXITED             2
#define TASK_STAT_END           3

extern const char *task_stat_str[];

struct task {
    char cmd[BUF_SZ];
    int tmux_id;
    pid_t pid;
    int id;

    int state;
    int tot_mem;
    int priority;
    long time_start;
    long time_end;
    int time_total;   // 작업에 필요한 시간
    int time_remain;  // 남은 시간
    int time_wait;    // 대기한 시간 
    int time_spent;   // 소모한 시간 
    struct list_head list;       // rq에서 사용할 리스트 정보
	struct list_head list_done;  // rq_done에서 사용할 리스트 정보
};

struct proc_table {
    int proc_cnt;
    struct list_head rq;
    struct list_head rq_done;
    struct task proc[TASK_CNT];
};

struct task* proc_register(struct proc_table *ptable, pid_t pid, char *cmd);
struct task* proc_set_timeout(struct task* p, int total_time);
struct task* proc_set_tmux_id(struct task* p, int tmux_id);
void sched_handler(struct proc_table *ptable);
void init_ptable(struct proc_table *ptable);
void rr_sched(struct proc_table *ptable);

#define PTABLE_SZ sizeof(struct proc_table)

#endif