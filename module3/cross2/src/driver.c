#include "driver.h"
#include "common.h"

#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <stdio.h>
#include <string.h>

void driver_run(int sock_fd) {
    driver_state_t st = {
        .sock_fd  = sock_fd,
        .timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK),
        .busy = 0,
        .busy_until = 0
    };
    if (st.timer_fd < 0) { perror("timerfd_create"); _exit(1); }

    int epfd = epoll_create1(0);
    if (epfd < 0) { perror("epoll_create1"); _exit(1); }

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = sock_fd; // Сокет для команд от родителя
    epoll_ctl(epfd, EPOLL_CTL_ADD, sock_fd, &ev);

    // Таймер
    ev.events = EPOLLIN;
    ev.data.fd = st.timer_fd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, st.timer_fd, &ev);

    char buf[BUF_SIZE];

    while (1) {
        struct epoll_event events[2];
        int nfds = epoll_wait(epfd, events, 2, -1);
        if (nfds < 0) { perror("epoll_wait"); break; }

        for (int i = 0; i < nfds; i++) {
            int fd = events[i].data.fd;
            if (fd == sock_fd) {
                // команда от родителя
                ssize_t n = read(sock_fd, buf, sizeof(buf) - 1);
                if (n <= 0) {
                    // родитель закрыл соединение — выходим
                    close(epfd);
                    close(st.timer_fd);
                    close(sock_fd);
                    _exit(0);
                }
                buf[n] = '\0';

                if (strncmp(buf, "TASK", 4) == 0) {
                    int secs;
                    if (sscanf(buf, "TASK %d", &secs) != 1 || secs <= 0) {
                        send_line(sock_fd, "ОШИБКА создание таймера\n");
                        continue;
                    }

                    if (st.busy) {
                            // уже занят — сообщаем оставшееся время
                            time_t rem = st.busy_until - now_sec();
                            if (rem < 0) rem = 0;
                            char resp[BUF_SIZE];
                            snprintf(resp, sizeof(resp), "ЗАНЯТ %ld\n", (long)rem);
                            send_line(sock_fd, resp);
                    } else {
                            // запускаем задачу
                            st.busy = 1;
                            st.busy_until = now_sec() + secs;

                            struct itimerspec its = {0};
                            its.it_value.tv_sec = secs;
                            timerfd_settime(st.timer_fd, 0, &its, NULL);

                            send_line(sock_fd, "ОК\n");
                    }
                } else if (strncmp(buf, "STATUS", 6) == 0) {
                    if (st.busy) {
                            time_t rem = st.busy_until - now_sec();
                            if (rem < 0) rem = 0;
                            char resp[BUF_SIZE];
                            snprintf(resp, sizeof(resp), "ЗАНЯТ %ld\n", (long)rem);
                            send_line(sock_fd, resp);
                        } else {
                            send_line(sock_fd, "СВОБОДЕН\n");
                        }
                    } else {
                        send_line(sock_fd, "ОШИБКА неизвестная команда\n");
                    }
            } else if (fd == st.timer_fd) {
                    // таймер сработал — задача завершена
                    uint64_t exp;
                    read(st.timer_fd, &exp, sizeof(exp));  // сброс
                    st.busy = 0;
                    st.busy_until = 0;
                    send_line(sock_fd, "ДОСТУПЕН\n");
            }
        }
    }
}