#include "common.h"
#include "strongpoint.h"
#include "driver.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>

static driver_info_t drivers[MAX_DRIVERS];
static int driver_count = 0;

static void create_driver(int epfd) {
    if (driver_count >= MAX_DRIVERS) {
        printf("Достигнут лимит водителей\n");
        return;
    }

    int sv[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) < 0) {
        perror("socketpair");
        return;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        close(sv[0]);
        close(sv[1]);
        return;
    }

    if (pid == 0) {
        // дочерний процесс — водитель
        close(sv[0]);
        driver_run(sv[1]);
        _exit(0);
    }

    close(sv[1]);
    drivers[driver_count].pid = pid;
    drivers[driver_count].sock_fd = sv[0];

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = sv[0];
    epoll_ctl(epfd, EPOLL_CTL_ADD, sv[0], &ev);

    printf("Создан водитель с PID %d\n", pid);
    driver_count++;
}

static int find_driver(pid_t pid) {
    for (int i = 0; i < driver_count; i++) {
        if (drivers[i].pid == pid) return i;
    }
    return -1;
}

static void send_task(pid_t pid, int timer_sec) {
    int idx = find_driver(pid);
    if (idx < 0) {
        printf("Водитель с PID %d не найден\n", pid);
        return;
    }
    char cmd[BUF_SIZE];
    snprintf(cmd, sizeof(cmd), "TASK %d\n", timer_sec);
    send_line(drivers[idx].sock_fd, cmd);
}

static void get_status(pid_t pid) {
    int idx = find_driver(pid);
    if (idx < 0) {
        printf("Водитель с PID %d не найден\n", pid);
        return;
    }
    send_line(drivers[idx].sock_fd, "STATUS\n");
}

static void get_drivers(void) {
    if (driver_count == 0) {
        printf("Нет запущенных водителей\n");
        return;
    }
    for (int i = 0; i < driver_count; i++) {
        send_line(drivers[i].sock_fd, "STATUS\n");
    }
}

void strongpoint_run() {
    int epfd = epoll_create1(0);
    if (epfd < 0) { perror("epoll_create1"); return; }

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = STDIN_FILENO;
    epoll_ctl(epfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev);

    printf("CLI такси. Команды:\n");
    printf("  create_driver\n");
    printf("  send_task <pid> <task_timer>\n");
    printf("  get_status <pid>\n");
    printf("  get_drivers\n");

    char line[BUF_SIZE];

    while (1) {
        struct epoll_event events[MAX_DRIVERS + 1];
        int nfds = epoll_wait(epfd, events, MAX_DRIVERS + 1, -1);
        if (nfds < 0) { perror("epoll_wait"); break; }
        for (int i = 0; i < nfds; i++) {
            int fd = events[i].data.fd;
            if (fd == STDIN_FILENO) {
                if (!fgets(line, sizeof(line), stdin)) {
                    // EOF — корректно завершаемся
                    for (int j = 0; j < driver_count; j++) {
                        close(drivers[j].sock_fd);
                    }
                    close(epfd);
                    return;
                }
                line[strcspn(line, "\n")] = 0;
                if (strcmp(line, "create_driver") == 0) {
                    create_driver(epfd);
                } else if (strncmp(line, "send_task ", 10) == 0) {
                    pid_t pid;
                    int timer;
                    if (sscanf(line + 10, "%d %d", &pid, &timer) == 2) {
                        send_task(pid, timer);
                    } else {
                        printf("Использование: send_task <pid> <task_timer>\n");
                    }
                } else if (strncmp(line, "get_status ", 11) == 0) {
                    pid_t pid;
                    if (sscanf(line + 11, "%d", &pid) == 1) {
                        get_status(pid);
                    } else {
                        printf("Использование: get_status <pid>\n");
                    }
                } else if (strcmp(line, "get_drivers") == 0) {
                    get_drivers();
                } else if (strlen(line) > 0) {
                    printf("Неизвестная команда: %s\n", line);
                }
            } else {
                // событие от сокета водителя — ответ
                char buf[BUF_SIZE];
                ssize_t n = read(fd, buf, sizeof(buf) - 1);
                if (n <= 0) {
                    printf("Водитель (fd %d) отсоединился\n", fd);
                    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                    close(fd);
                } else {
                    buf[n] = '\0';
                    pid_t pid = -1;
                    for (int j = 0; j < driver_count; j++) {
                        if (drivers[j].sock_fd == fd) {
                            pid = drivers[j].pid;
                            break;
                        }
                    }
                    if (pid != -1)
                        printf("Водитель %d: %s", pid, buf);
                    else
                        printf("Ответ: %s", buf);
                }
            }
        }
    }
    close(epfd);
}