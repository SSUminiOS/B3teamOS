#include <stdio.h>
#include "system.h"
#define TIME_SLICE 3

void register_proc(struct proc_table *ptable, char *id, 
                   int arrival_time, int total_time) {
    struct task *next;
    if (ptable->proc_cnt > TASK_CNT) {
        perror("max task exceed");
        exit(EXIT_FAILURE);
    }

    next = &(ptable->proc[ptable->proc_cnt]);    
    next->time_arrival = arrival_time;
    next->time_remain = total_time;
    next->time_total = total_time;
    next->time_spent = 0;
    next->time_wait = 0;
    strcpy(next->id, id);
    ptable->proc_cnt++;
}

void proc_status(struct task *p, void *arg) {
    printf("[%s]\t%2d\t%2d\t%2d\t%2d\n", p->id, p->time_total, p->time_remain,
                                         p->time_wait, p->time_spent);
}

void iterate_ptable(struct proc_table *ptable, void (*func)(), void *arg) {
    if (ptable->proc_cnt > 0) {
        for (int i = 0; i < ptable->proc_cnt; i++) {
            struct task *p = &(ptable->proc[i]);
            func(p, arg);
        }
    }
}

void evict_proc(struct task *p, struct task *target_p) {
    if (p >= target_p)
        *p = *(p + 1);
}

void rr_update_wait(struct task *p, int *ts) {
    if (p->time_remain > 0) {
        p->time_spent += *ts;
        p->time_wait += *ts;
    }
}

void rr_update_wait_undo(struct task *p, int *ts) {
    if (p->time_remain > 0) {
        p->time_spent -= *ts;
        p->time_wait -= *ts;
    }
}

void rr_sched(struct proc_table *ptable) {
    int ts = TIME_SLICE;
    printf(" ID\ttotal\tremain\twait\tspent\n");

    while (ptable->proc_cnt > 0) {
        struct task *evict_list[TASK_CNT];
        int evict_cnt = 0;
        printf("===== Round Robin Cycle START =====\n");
        for (int i = 0; i < ptable->proc_cnt; i++) {
            struct task *p = &(ptable->proc[i]);
            if (p->time_remain > ts) {
                iterate_ptable(ptable, rr_update_wait, (void *) &ts);
                rr_update_wait_undo(p, &ts);
                p->time_spent += ts;
                p->time_remain -= ts;
                proc_status(p, NULL);
            } else if (p->time_remain > 0) {
                iterate_ptable(ptable, rr_update_wait, (void *) &(p->time_remain));
                rr_update_wait_undo(p, &(p->time_remain));
                p->time_spent += p->time_remain;
                p->time_remain -= p->time_remain;
                proc_status(p, NULL);

                evict_list[evict_cnt++] = p;
                strcat(p->id, "*");
            } 
        }

        if (evict_cnt > 0) {
            printf("===== Round Robin Cycle END =====\n");
            iterate_ptable(ptable, proc_status, NULL);
            for (int i = evict_cnt - 1; i >= 0; i--) {
                iterate_ptable(ptable, evict_proc, (void *) evict_list[i]);
                ptable->proc_cnt --;
            }
        }
    }    
}