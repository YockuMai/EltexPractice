#include <common.h>

#include <string.h>

time_t now_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec;
}

void send_line(int fd, const char *line) {
    size_t len = strlen(line);
    ssize_t n = write(fd, line, len);
    (void)n;
}