#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <time.h>

#include "primitive.h"
#include "helper.h"
#include "proc.h"

struct task *curr = NULL;

const char *task_stat_str[] = {
    [TASK_RUNNABLE] = "RUNNABLE",
    [TASK_RUNNING] = "RUNNING",
    [TASK_EXITED] = "EXITED",
};

struct task* proc_register(struct proc_table *ptable, pid_t pid, char *cmd) {
    struct task *next;
    if (ptable->proc_cnt > TASK_CNT) {
        perror("max task exceed");
        exit(EXIT_FAILURE);
    }

    next = &(ptable->proc[ptable->proc_cnt]);    
    if (cmd)
        strcpy(next->cmd, cmd);

    next->priority = PRIO_DEFAULT;
    next->state = TASK_RUNNABLE;
    next->tot_mem = 0;
    next->pid = pid;

    next->time_start = epoch_now();
    next->time_spent = 0; 
    next->time_wait = 0;

    INIT_LIST_HEAD(&(next->list));
    INIT_LIST_HEAD(&(next->list_done));
    // 프로세스를 rq에 삽입
    list_reinsert_tail(&(next->list), &(ptable->rq));

    ptable->proc_cnt++;
    return next;
}

struct task* proc_deregister(struct proc_table *ptable, pid_t pid) {
    struct list_head *rq = &(ptable->rq);
    struct task *p, *target = NULL;

    for_each(p, ptable->proc) {
        if (p->pid == pid)
            target = p;
        else if (p->pid > pid)
            p->tmux_id --;
    }

    target->state = TASK_EXITED;
    list_reinsert(&(target->list_done), &(ptable->rq_done));
    list_del(&(target->list));

    return target;
}

struct task* proc_set_timeout(struct task* p, int total_time) {
    p->time_remain = total_time;
    p->time_total = total_time;
}

struct task* proc_set_tmux_id(struct task* p, int tmux_id) {
    p->tmux_id = tmux_id;
}

void init_ptable(struct proc_table *ptable) {
    INIT_LIST_HEAD(&(ptable->rq));
    INIT_LIST_HEAD(&(ptable->rq_done));

    struct task *p;
    for_each(p, ptable->proc) {
        p->state = TASK_EXITED;
        p->id = i;
    }
}

void send_signal(pid_t pid, int signo) {
    char cmd[BUF_SZ];
    int child_pid = 0;
    sprintf(cmd, "pgrep -P %d", pid);

    shell(cmd, "%d", &child_pid);
    kill(pid, signo);

    if (child_pid != 0)
        kill(child_pid, signo);
}

// 미리 사용시간 집계하고 수행 시작
void sched_next(struct proc_table *ptable, struct task* next, int ts) {
    struct list_head *rq = &(ptable->rq);
    struct task* p;

    list_for_each_entry(p, rq, list) {
        send_signal(p->pid, SIGSTOP);
        if (p != next) {
            // p->time_spent += ts;
            p->time_wait += ts;
        }
    }

    next->time_spent += ts;
    send_signal(next->pid, SIGCONT);
    system_d("tmux select-pane -t MiniOS:0.%d -m", next->tmux_id);
}

void sched_handler(struct proc_table *ptable) {
    struct list_head *rq = &(ptable->rq);
    
    if (curr)
        if (curr->state != TASK_EXITED)
            list_reinsert_tail(&(curr->list), rq);
        else if (curr->state == TASK_EXITED)
            list_del(&(curr->list));

    // check rq is not empty
    if (!list_empty(rq)) {
        curr = list_first_entry(rq, struct task, list);
        sched_next(ptable, curr, CLOCK);
    } else {
        curr = NULL;
    }
}
