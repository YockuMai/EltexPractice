#include "common.h"
#include "publisher.h"
#include "my_string.h"
#include <stdio.h>
#include <sys/msg.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>

static volatile sig_atomic_t stop = 0;
static void on_sigint(int s) { (void)s; stop = 1; }

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

int publisher_run(const char *topic) {
    size_t topic_len = my_strlen(topic);
    if (topic_len == 0 || topic_len >= MAX_TOPIC) {
        fprintf(stderr, "[Издатель] Ошибка: недопустимая длина темы\n");
        return 1;
    }

    int qid = msgget(MSG_KEY, 0);
    if (qid == -1) {
        fprintf(stderr, "[Издатель] Ошибка: запустите брокера\n");
        return 1;
    }

    // Установка обработчика сигнала SIGINT
    struct sigaction sa = {0};
    sa.sa_handler = on_sigint;
    sigaction(SIGINT, &sa, NULL);

    pid_t me = getpid();
    printf("[Издатель %d] PID=%d, Тема=%s\n", (int)me, (int)me, topic);

    char line[MAX_TEXT];
    while (!stop) {
        errno = 0;
        if (!fgets(line, sizeof(line), stdin))
        {
            if (stop) break;
            if (errno == EINTR) continue;
            break;
        }

        size_t len = my_strlen(line);
        if (len > 0 && line[len - 1] == '\n') line[--len] = 0;
        if (len == 0) continue;

        struct msgbuf msg;
        msg.mtype = 1; // Тип сообщения для брокера
        size_t pos = 0;

        int fits = 1;
        fits &= append(msg.mtext, MAX_TEXT, &pos, "send,");
        fits &= append_uint(msg.mtext, MAX_TEXT, &pos, (unsigned)me);
        fits &= append(msg.mtext, MAX_TEXT, &pos, ",");
        fits &= append(msg.mtext, MAX_TEXT, &pos, topic);
        fits &= append(msg.mtext, MAX_TEXT, &pos, ",");
        fits &= append(msg.mtext, MAX_TEXT, &pos, line);

        if (!fits) {
            fprintf(stderr, "[Издатель %d] Ошибка: сообщение слишком длинное\n", (int)me);
        }

        if (msgsnd(qid, &msg, sizeof(msg.mtext), 0) == -1) {
            if (errno == EIDRM || errno == EINVAL)
            {
                fprintf(stderr, "[Издатель %d] Очередь удалена брокером\n", (int)me);
                return 1;
            }
            else if (errno == EINTR)
                continue;
            perror("[Издатель] Ошибка при отправке сообщения");
            return 1;
        }
    }

    printf("[Издатель %d] Завершение работы...\n", (int)me);
    return 0;
}