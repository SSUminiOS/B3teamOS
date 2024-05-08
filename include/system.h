#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include <stdbool.h>


// include/linux/sched.h
//SSU struct task_struct {

void minisystem();
void whoami();

// extra/command.c
int command(char *cmd);
int ptrace_command(char *cmd);

// extra/ipc.c
int ipc_test();

// extra/ipc.c
int calc_pi();

#endif
