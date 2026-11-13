#include "common.h"
#include "broker.h"
#include "my_string.h"
#include <signal.h>
#include <sys/msg.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct sub_entry {
    pid_t pid;
    char topic[MAX_TOPIC];
};

static struct sub_entry subs[MAX_SUBS];
static int sub_count = 0;

static pid_t pubs[MAX_PUBS];
static int pub_count = 0;

static volatile sig_atomic_t stop = 0;

static void add_sub(pid_t pid, const char *topic) {
    for (int i = 0; i < sub_count; i++) {
        if (subs[i].pid == pid && my_strcmp(subs[i].topic, topic) == 0)
            return;
    }
    if (sub_count < MAX_SUBS) {
        subs[sub_count].pid = pid;
        my_strncpy(subs[sub_count].topic, topic, MAX_TOPIC);
        sub_count++;
    }
}

static void add_pub(pid_t pid) {
    for (int i = 0; i < pub_count; i++) {
        if (pubs[i] == pid)
            return;
    }
    if (pub_count < MAX_PUBS) {
        pubs[pub_count] = pid;
        pub_count++;
    }
}

static void del_sub(pid_t pid, const char *topic) {
    for (int i = 0; i < sub_count; i++) {
        if (subs[i].pid == pid && my_strcmp(subs[i].topic, topic) == 0) {
            subs[i] = subs[sub_count - 1];
            sub_count--;
            return;
        }
    }
}

static void on_sigint(int s) { (void)s; stop = 1; }

static int next_field(const char **p, char *dst, size_t maxlen) {
    const char *s;
    size_t i = 0;
    if (!p || !(s = *p) || !*s) return -1;

    while (*s && *s != ',') {
        if (i + 1 < maxlen) dst[i++] = *s;
        s++;
    }
    dst[i] = 0;
    if (*s == ',') s++;
    *p = s;
    return 0;
}

int broker_run() {
    signal(SIGINT, on_sigint);

    // Создание очереди сообщений
    int qid = msgget(MSG_KEY, IPC_CREAT | IPC_EXCL | 0666);
    if (qid == -1) {
        if (errno == EEXIST)
            fprintf(stderr, "[Брокер %d] Другое приложеняи брокера уже запущено\n", (int)getpid());
        else
            perror("msgget");
        
        return 1;
    }
    printf("[Брокер %d] Запущен. Ожидание сообщений...\n", (int)getpid());

    struct msgbuf msg;
    char cmd[32], pidbuf[16], topic[MAX_TOPIC];
    const char *payload;

    while (!stop) {
        // IPC_NOWAIT - не блокировать процесс, если сообщений нет
        // 1 - тип сообщения, который мы хотим получить (приоритет)
        ssize_t n = msgrcv(qid, &msg, sizeof(msg.mtext), 1, IPC_NOWAIT);

        // При ошибке msgrcv, проверяем errno
        if (n == -1) {
            // Если нет сообщений, продолжаем цикл
            if (errno == ENOMSG) {
                usleep(100000);
                continue;
            }
            
            // Если произошла ошибка, продолжаем цикл
            if (errno == EINTR) {
                continue;
            }

            // Если очередь сообщений была удалена, завершаем работу
            if (errno == EIDRM || errno == EINVAL) {
                fprintf(stderr, "[Брокер %d] Очередь сообщений была удалена\n", (int)getpid());
                break;
            }

            perror("msgrcv");
            break;
        }

        // Разбор сообщения в формате "cmd,pid,topic,payload"
        const char *p = msg.mtext;
        if (next_field(&p, cmd, sizeof(cmd)) != 0) continue;
        if (next_field(&p, pidbuf, sizeof(pidbuf)) != 0) continue;
        if (next_field(&p, topic, sizeof(topic)) != 0) continue;
        // payload может содержать запятые
        payload = p;

        pid_t pid = (pid_t)atoi(pidbuf);
        if (pid <= 0) continue;

        if (my_strcmp(cmd, "subscribe") == 0) {
            add_sub(pid, topic);
            printf("[Брокер %d] Подписка: PID=%d, Тема=%s\n", (int)getpid(), pid, topic);
        } else if (my_strcmp(cmd, "unsubscribe") == 0) {
            del_sub(pid, topic);
            printf("[Брокер %d] Отписка: PID=%d, Тема=%s\n", (int)getpid(), pid, topic);
        } else if (my_strcmp(cmd, "send") == 0) {
            add_pub(pid);
            printf("[Брокер %d] Сообщение от PID=%d, Тема=%s, Сообщение=%s\n", (int)getpid(), pid, topic, payload);

            // Отправка сообщения всем подписчикам на эту тему
            for (int i = 0; i < sub_count; i++) {
                if (my_strcmp(subs[i].topic, topic) != 0) continue;

                struct msgbuf out_msg;
                out_msg.mtype = subs[i].pid;

                size_t pos = 0;
                if (pos + 1 < MAX_TEXT) out_msg.mtext[pos++] = '[';
                size_t tl = my_strlen(topic);
                for (size_t k = 0; k < tl && pos + 1 < MAX_TEXT; k++)
                    out_msg.mtext[pos++] = topic[k];
                if (pos + 2 < MAX_TEXT) {
                    out_msg.mtext[pos++] = ']';
                    out_msg.mtext[pos++] = ' ';
                }
                size_t pl = my_strlen(payload);
                for (size_t k = 0; k < pl && pos + 1 < MAX_TEXT; k++)
                    out_msg.mtext[pos++] = payload[k];
                out_msg.mtext[pos] = 0;

                if (msgsnd(qid, &out_msg, sizeof(out_msg.mtext), 0) == -1) {
                    if (errno == EAGAIN)
                        fprintf(stderr, "[Брокер %d] Очреедь сообщений переполнена для PID=%d\n", (int)getpid(), (int)subs[i].pid);
                    else if (errno == EIDRM || errno == EINVAL)
                        stop = 1; // Очередь сообщений была удалена, завершаем работу
                }
            }
        }
        else fprintf(stderr, "[Брокер %d] Неизвестная команда: %s\n", (int)getpid(), cmd);
    }

    // Завершение работы брокера
    printf("[Брокер %d] Завершение работы...\n", (int)getpid());

    // Удаление издателей
    for (int i = 0; i < pub_count; i++) {
        kill(pubs[i], SIGINT);
        printf("[Брокер %d] Завершение издателя PID=%d\n", (int)getpid(), (int)pubs[i]);
    }

    // Удаление подписчиков
    for (int i = 0; i < sub_count; i++) {
        int dup = 0;
        for (int j = 0; j < i; j++) {
            if (subs[i].pid == subs[j].pid) {
                dup = 1;
                break;
            }
        }
        if (!dup) {
            kill(subs[i].pid, SIGINT);
            printf("[Брокер %d] Завершение подписчика PID=%d\n", (int)getpid(), (int)subs[i].pid);
        }
    }

    // Удаление очереди сообщений
    // IPC_RMID - немедленно удаляет очередь сообщений, даже если процессы все еще используют ее
    msgctl(qid, IPC_RMID, NULL);
    printf("[Брокер %d] Очередь сообщений удалена\n", (int)getpid());
    return 0;
}