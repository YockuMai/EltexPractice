#include "subscriber.h"
#include "common.h"
#include "my_string.h"
#include <stdio.h>
#include <sys/msg.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>

static int append(char *buf, size_t limit, size_t *pos, const char *src) {
    int ok = 1;
    size_t i = 0;
    while (src[i]) {
        if (*pos + 1 >= limit) { ok = 0; break; }
        buf[(*pos)++] = src[i++];
    }
    buf[*pos] = 0;
    return ok;
}

static int append_uint(char *buf, size_t limit, size_t *pos, unsigned v) {
    char tmp[16];
    int n = 0;
    if (v == 0) {
        tmp[n++] = '0';
    } else {
        while (v) { tmp[n++] = (char)('0' + (v % 10)); v /= 10; }
    }
    int ok = 1;
    while (n > 0) {
        if (*pos + 1 >= limit) { ok = 0; break; }
        buf[(*pos)++] = tmp[--n];
    }
    buf[*pos] = 0;
    return ok;
}

static volatile sig_atomic_t stop = 0;
static void on_sigint(int s) { (void)s; stop = 1; }

int subscriber_run(int n, const char **topics) {
    for (int i = 0; i < n; i++) {
        size_t tl = my_strlen(topics[i]);
        if (tl == 0 || tl >= MAX_TOPIC) {
            fprintf(stderr, "[Подписчик] Неверная длина темы для \"%s\"\n", topics[i]);
            return 1;
        }
    }

    int qid = msgget(MSG_KEY, 0);
    if (qid == -1) {
        perror("[Подписчик] Запустите брокера");
        return 1;
    }

    // Установка обработчика сигнала SIGINT
    struct sigaction sa = {0};
    sa.sa_handler = on_sigint;
    sigaction(SIGINT, &sa, NULL);

    pid_t me = getpid();

    for (int i = 0; i < n; i++) {
        struct msgbuf cmd;
        cmd.mtype = 1;
        size_t pos = 0;
        cmd.mtext[0] = 0;

        append(cmd.mtext, sizeof(cmd), &pos, "subscribe,");
        append_uint(cmd.mtext, sizeof(cmd), &pos, (unsigned)me);
        append(cmd.mtext, sizeof(cmd), &pos, ",");
        append(cmd.mtext, sizeof(cmd), &pos, topics[i]);
        if (msgsnd(qid, &cmd, my_strlen(cmd.mtext) + 1, 0) == -1) {
            perror("[Подписчик] Ошибка отправки команды подписки");
            return 1;
        }
    }
    printf("[Подписчик %d] Подписка на %d тем(ы) выполнена\n", me, n);
    for (int i = 0; i < n; i++)
        printf("[Подписчик %d]  %s\n", me, topics[i]);

    struct msgbuf msg;
    while (!stop) {
        ssize_t n = msgrcv(qid, &msg, sizeof(msg.mtext), (long)me, IPC_NOWAIT);
        if (n == -1) {
            if (errno == ENOMSG) { usleep(100000); continue; }
            if (errno == EINTR) {
                continue;
            }
            if (errno == EIDRM || errno == EINVAL) {
                fprintf(stderr, "[Подписчик] Очередь сообщений была удалена\n");
                return 1;
            }
            perror("msgrcv");
            return 1;
        }
        printf("[Подписчик %d] Получено сообщение: %s\n", me, msg.mtext);
    }

    for (int i = 0; i < n; i++) {
        struct msgbuf cmd;
        cmd.mtype = 1;
        size_t pos = 0;
        cmd.mtext[0] = 0;
        append(cmd.mtext, sizeof(cmd), &pos, "unsubscribe,");
        append_uint(cmd.mtext, sizeof(cmd), &pos, (unsigned)me);
        append(cmd.mtext, sizeof(cmd), &pos, ",");
        append(cmd.mtext, sizeof(cmd), &pos, topics[i]);
        if (msgsnd(qid, &cmd, my_strlen(cmd.mtext) + 1, 0) == -1) {
            if (errno == EIDRM || errno == EINVAL) {
                fprintf(stderr, "[Подписчик] Очередь сообщений была удалена\n");
                printf ("[Подписчик %d] Завершение работы...\n", me);
                return 1;
            }
            if (errno == EINTR) {
                continue;
            }
            perror("[Подписчик] Ошибка отправки команды отписки");
            return 1;
        }
    }
    printf("[Подписчик %d] Отписка от %d тем(ы) выполнена\n", me, n);
    for (int i = 0; i < n; i++)
        printf("[Подписчик %d]  %s\n", me, topics[i]);

    printf ("[Подписчик %d] Завершение работы...\n", me);
    return 0;
}