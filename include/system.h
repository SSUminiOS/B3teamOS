#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include "primitive.h"
#include "helper.h"
#include "proc.h"

void os_core(struct os_args *args);
void os_input(struct os_args *args);
void os_status(struct os_args *args);
void spawn_proc(void (*child)(), void (*parent)(), void *arg);
void reptyr_proc(pid_t pid);
void resize_panes();
int count_panes();

#endif