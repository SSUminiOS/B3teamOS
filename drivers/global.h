#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <stdlib.h>
#include <signal.h>
#include <semaphore.h>

#include "primitive.h"
#include "proc.h"

extern int cleanup_disabled;
extern void (*cleanup_hdlr)();

static struct shm_info *COMMON;
static struct shm_info *PTABLE;
static struct shm_info *INPUT;

static struct common_table *common;
static struct proc_table *ptable;
static char *input_buf;

static sem_t *common_sem;
static sem_t *ptable_sem;
static sem_t *input_sem;

#define OS_INPUT    0
#define OS_CORE     1
#define OS_STATUS   2
#define OS_PROC_END 3

struct common_table {
    int cnt;
    pid_t pids[OS_PROC_END];
};

#define COMMON_SZ sizeof(struct common_table)

static inline struct os_args prepare_args() {
    struct os_args args = {.arg0 = COMMON, .arg1 = PTABLE, .arg2 = INPUT};
    return args;
}

static inline void parse_args(struct os_args *args) {
    COMMON = args->arg0;
    PTABLE = args->arg1;
    INPUT = args->arg2;

    common = COMMON->data;
    ptable = PTABLE->data;
    input_buf = INPUT->data;

    common_sem = COMMON->lock.sem;
    ptable_sem = PTABLE->lock.sem;
    input_sem = INPUT->lock.sem;
}

#endif