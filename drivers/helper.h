#ifndef __HELPER_H__
#define __HELPER_H__

#include <sys/types.h> 

#define RQ_DONE_FILE  "RQ_DONE"
#define SEM_NAME  "/sem"
#define SHM_ORDER "/order"
#define ORDER_PARENT 1
#define ORDER_CHILD 0

#define PANE_LEFT   0
#define PANE_RIGHT  2

#define BUF_SZ      256
#define TASK_CNT    32
#define CLOCK       1.0
#define EXIT_TMOUT  3

int is_wsl();
int get_cols();
long epoch_now();
void clear_line();
int is_invalid(char *str);
void system_d(char *fmt, ...);
void parse_cmd(char *cmd, char **args);
char *get_random_str(const char *prefix);
void shell(char *cmd, char *fmt, void *ptr);
void read_file(const char *path, void (*hdlr)());
void init_timer(void (*hdlr)(), int signo, double second);

#define EQ(buf, str) (strncmp(buf, str, strlen(str)) == 0)

#endif