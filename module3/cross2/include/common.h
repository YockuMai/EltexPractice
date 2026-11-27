#ifndef COMMON_H
#define COMMON_H

#include <unistd.h>
#include <time.h>

#define MAX_DRIVERS 16
#define BUF_SIZE 256

// Водитель (в родителе)
typedef struct {
    pid_t pid;
    int sock_fd;
} driver_info_t;

// Состояние водителя в дочернем процессе
typedef struct {
    int sock_fd;
    int timer_fd;
    int busy;
    time_t busy_until;
} driver_state_t;

time_t now_sec();
void send_line(int fd, const char *line);

#endif