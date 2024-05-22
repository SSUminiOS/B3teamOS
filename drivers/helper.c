#include <time.h>
#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <string.h>
#include <dirent.h>
#include <sys/ioctl.h>

#include "helper.h"

int is_wsl() {
    FILE *fp;
    char buffer[256];

    fp = fopen("/proc/version", "r");
    if (fp == NULL) {
        perror("fopen");
        return 0;
    }

    fgets(buffer, sizeof(buffer), fp);
    fclose(fp);

    if (strstr(buffer, "Microsoft") != NULL || strstr(buffer, "WSL") != NULL) {
        return 1;
    }

    return 0;
}

void clear_line() {
    // ANSI escape code to move cursor to the beginning of the line and clear the line
    printf("\r\033[K");
    fflush(stdout);
}

// Return the number of columns (terminal width)
int get_cols() {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) {
        perror("ioctl");
        return -1;
    }
    return ws.ws_col;
}

// 입력 문자열에 공백이 아닌 문자가 있는지 확인
// 공백이 아닌 문자를 찾으면 false 반환
// 문자열 전체가 공백인 경우 true 반환
int is_invalid(char *str) {
    while (*str) {
        if (!isspace((unsigned char)*str)) {
            return 0;
        }
        str++;
    }
    return 1;
}

void system_d(char *fmt, ...) {
    char cmd[BUF_SZ];
    va_list args;

    va_start(args, fmt);
    vsnprintf(cmd, sizeof(cmd), fmt, args);
    va_end(args);

    // printf("%d %s\n", getpid(), cmd);
    system(cmd);
}

void shell(char *cmd, char *fmt, void *ptr) {
    FILE *fp = popen(cmd, "r");
    if (fp == NULL) {
        fprintf(stderr, "Failed to execute command\n");
        exit(EXIT_FAILURE);
    }
    
    fscanf(fp, fmt, ptr);
    pclose(fp);
}

void read_file(const char *path, void (*hdlr)()) {
    FILE *fp = fopen(path, "r");
    char buffer[BUF_SZ];

    if (fp == NULL) {
        fprintf(stderr, "Failed to open file %s\n", path);
        exit(EXIT_FAILURE);
    }

    while (fgets(buffer, sizeof(buffer), fp) != NULL)
        hdlr(buffer);
}

char *get_random_str(const char *prefix) {
    char tpl[1024] = "/tmp/";

    // PREFIX와 임시 파일 이름을 합침
    if (prefix) {
        strcat(tpl, prefix);
        strcat(tpl, ".");
    }
    strcat(tpl, "XXXXXX");

    // 임시 파일 이름 생성 (파일은 생성되지 않음)
    if (mktemp(tpl) == NULL) {
        perror("mktemp");
        exit(EXIT_FAILURE);
    }

    // 랜덤 문자열 부분만 반환
    // 메모리 누수를 방지하기 위해 strdup로 문자열 복사 후 반환
    return strdup(&tpl[5]);
}

void parse_cmd(char *cmd, char **args) {
  while (*cmd != '\0') {
    while (isspace(*cmd)) cmd++;  // 공백 건너뛰기

    if (*cmd == '\"' || *cmd == '\'') {  // 따옴표 시작을 찾으면
      char quote = *cmd++;               // 따옴표 기억
      *args++ = cmd;  // 따옴표 다음 문자를 인자 시작으로 설정
      while (*cmd && *cmd != quote) cmd++;  // 다음 따옴표 찾기
    } else {
      *args++ = cmd;  // 현재 위치를 인자 시작으로 설정
      while (*cmd && !isspace(*cmd) && *cmd != '\"' && *cmd != '\'')
        cmd++;  // 인자 끝 찾기
    }

    if (*cmd) *cmd++ = '\0';  // 현재 인자를 끝내고 다음 인자로
  }
  *args = NULL;  // 인자 리스트 종료
}

long epoch_now() {
    time_t now;
    time(&now);
    return (long)now;
}

void init_timer(void (*hdlr)(), int signo, double second) {
    timer_t timerid;
    struct sigevent sev;
    struct itimerspec its;

    // 시그널 핸들러 등록
    signal(signo, hdlr);

    // 타이머 이벤트 설정
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = signo;
    sev.sigev_value.sival_ptr = &timerid;

    timer_create(CLOCK_REALTIME, &sev, &timerid);

    // 타이머 설정
    long nanoseconds = (second - (long)second) * 1e9;
    long seconds_long = (long)second;

    its.it_value.tv_sec = seconds_long;
    its.it_value.tv_nsec = nanoseconds;
    its.it_interval.tv_sec = seconds_long;
    its.it_interval.tv_nsec = nanoseconds;

    // 타이머 시작
    timer_settime(timerid, 0, &its, NULL);
}
