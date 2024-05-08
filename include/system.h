#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include <stdbool.h>


// include/linux/sched.h
//SSU struct task_struct {

void minisystem();
void whoami();

// 20150498/command.c
int command(char *cmd);
int ptrace_command(char *cmd);

// 20150498/ipc.c
int ipc_test();

// 20150498/ipc.c
int calc_pi();

#endif