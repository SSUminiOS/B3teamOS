#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "helper.h"
#include "global.h"
#include "proc.h"

#define PRINT_RQ(header, pos, head, member) { \
    printf("%s", header); \
    list_for_each_entry(pos, head, member) \
        printf(" -> P%2d", pos->id); \
    printf("\n"); \
}

void stat_hdlr() {
    int bar_width = get_cols() - 15;
    double cpu_usage = 70;
    int pos = bar_width * (cpu_usage / 100.0);
    struct task *p;

    system_d("clear");
    printf("CPU : [");
    for (int i = 0; i < bar_width; ++i) {
        if (i < pos) printf("#");
        else printf(" ");
    }
    printf("] %.2f%%\n", cpu_usage);

    sem_wait(ptable_sem);
    PRINT_RQ("[RQ]", p, &(ptable->rq), list);
    PRINT_RQ("[END]", p, &(ptable->rq_done), list_done);

    for_each_until(p, ptable->proc, ptable->proc_cnt)
        printf("P%2d: %d, %s\n", p->id, p->pid, task_stat_str[p->state]);

    sem_post(ptable_sem);
}

void os_status(struct os_args *args) {
    parse_args(args);
    signal(SIGINT, cleanup_hdlr);
    init_timer(stat_hdlr, SIGRTMIN, CLOCK);

    while (1) {
        sleep(1);
    }
}