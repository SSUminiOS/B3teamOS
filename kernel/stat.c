#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <semaphore.h>

#include "helper.h"
#include "global.h"
#include "proc.h"

#define PRINT_RQ(header, pos, head, member) { \
    printf("%s", header); \
    list_for_each_entry(pos, head, member) \
        printf(" -> P%2d", pos->id); \
    printf("\n"); \
}


// 계산된 CPU 사용량과 메모리 사용량을 저장하기 위한 전역 변수
static double cpu_usage = 0.0;
static double mem_usage = 0.0;
static struct proc_table *ptable;
static sem_t *ptable_sem;

// CPU 사용량 계산 함수
double calculate_cpu_usage() {
    long double a[4], b[4], loadavg;
    FILE *fp;
    static long double prev[4] = {0.0, 0.0, 0.0, 0.0};

    fp = fopen("/proc/stat","r");
    if (fp == NULL) {
        perror("Error opening file");
        return -1;
    }
    if (fscanf(fp, "%*s %Lf %Lf %Lf %Lf", &a[0], &a[1], &a[2], &a[3]) < 4) {
        perror("Error reading file");
        fclose(fp);
        return -1;
    }
    fclose(fp);

    if (prev[0] == 0 && prev[1] == 0 && prev[2] == 0 && prev[3] == 0) {
        prev[0] = a[0];
        prev[1] = a[1];
        prev[2] = a[2];
        prev[3] = a[3];
        return -1;
    }

    long double total_prev = prev[0] + prev[1] + prev[2] + prev[3];
    long double total_now = a[0] + a[1] + a[2] + a[3];
    long double total_diff = total_now - total_prev;
    long double idle_diff = a[3] - prev[3];

    loadavg = (total_diff - idle_diff) / total_diff;

    prev[0] = a[0];
    prev[1] = a[1];
    prev[2] = a[2];
    prev[3] = a[3];

    return loadavg * 100;
}

// 메모리 사용량 계산 함수
double calculate_memory_usage() {
    long long total_memory = 0;
    long long free_memory = 0;
    char line[256];
    FILE *fp = fopen("/proc/meminfo", "r");
    if (fp == NULL) {
        perror("Error opening file");
        return -1;
    }
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "MemTotal:", 9) == 0) {
            sscanf(line, "MemTotal: %lld kB", &total_memory);
        }
        if (strncmp(line, "MemFree:", 8) == 0) {
            sscanf(line, "MemFree: %lld kB", &free_memory);
            break;
        }
    }
    fclose(fp);
    if (total_memory == 0) {
        return -1;
    }

    double used_memory = (double)(total_memory - free_memory) / total_memory;
    return used_memory * 100;
}

void print_proc_info(int pid) {
    char path[40], line[256], state;
    long virtualMem;
    sprintf(path, "/proc/%d/stat", pid);

    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        perror("Error opening process stat file");
        return;
    }
    // /proc/[pid]/stat 파일에서 상태(state)와 가상 메모리 크기(VmSize)를 읽습니다.
    // 이 예에서는 필드 3(state)과 필드 23(vsize)를 읽습니다.
    if (fscanf(fp, "%*d %*s %c %*d %*d %*d %*d %*d %*u %*lu %*lu %*lu %*lu %*lu %*lu %*ld %*ld %*ld %*ld %*ld %*ld %*llu %ld", &state, &virtualMem) != 2) {
        perror("Error reading process stat file");
    } else {
        printf(" - PID: %d, State: %c, VmSize: %ld KB\n", pid, state, virtualMem / 1024);
    }
    fclose(fp);
}

// CPU, 메모리 사용량 보여주는 함수
void stat_hdlr() {
    cpu_usage = calculate_cpu_usage();
    mem_usage = calculate_memory_usage();
    struct task *p;

    system_d("clear");
    printf("CPU : [");
    int bar_width = get_cols() - 15;
    int cpu_pos = bar_width * (cpu_usage / 100.0);
    for (int i = 0; i < bar_width; ++i) {
        if (i < cpu_pos) printf("#");
        else printf(" ");
    }
    printf("] %.2f%%\n", cpu_usage);

    printf("MEM : [");
    int mem_bar_width = get_cols() - 15;
    int mem_pos = mem_bar_width * (mem_usage / 100.0);
    for (int i = 0; i < mem_bar_width; ++i) {
        if (i < mem_pos) printf("#");
        else printf(" ");
    }
    printf("] %.2f%%\n", mem_usage);

    sem_wait(ptable_sem);
    PRINT_RQ("[RQ]", p, &(ptable->rq), list);
    list_for_each_entry(p, &(ptable->rq), list) {
        print_proc_info(p->pid); // 각 프로세스의 정보를 출력
    }
    PRINT_RQ("[END]", p, &(ptable->rq_done), list_done);
    list_for_each_entry(p, &(ptable->rq_done), list_done) {
        print_proc_info(p->pid); // 각 프로세스의 정보를 출력
    }

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
