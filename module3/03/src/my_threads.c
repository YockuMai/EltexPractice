#include "my_threads.h"
#include "queue.h"
#include <signal.h>
#include <mqueue.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define TIMEOUT 200

static volatile sig_atomic_t stop = 0;
static volatile sig_atomic_t need_notify = 0;
static void on_sigint(int sig) {
    (void)sig; // подавление предупреждения о неиспользуемом параметре
    stop = 1;
    need_notify = 1;
}

void threads_install_sigint(void) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = on_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
}

/* 
Функция потока-отправителя сообщений. Она считывает сообщения с stdin и отправляет их в очередь сообщений.
Если поток получает сигнал SIGINT, он завершает свою работу и отправляет сообщение с приоритетом EXIT_PRIORITY, 
чтобы уведомить поток-получатель о завершении работы.
*/
void *sender_thread(void* arg) {
    mqd_t queue = *(mqd_t*)arg;
    char buf[MSG_SIZE];
    
    while (!stop) {
        fd_set rfds; // Набор файловых дескрипторов, за которыми необходимо следить
        FD_ZERO(&rfds); // Очистка набора
        FD_SET(STDIN_FILENO, &rfds); // Добавляем поток ввода в набор "жду чтения"
        struct timeval tv = {0, TIMEOUT * 1000}; // Таймаут

        /*  1 аргумент - наибольший номер файлового дескриптора среди всех проверяемых наборов, плюс единица
            2 аргумент - проверка на чтение
            3 аргумент - проверка на запись
            4 аргумент - проверка на исключительные ситуации
            5 аргумент - как долго ждать события
            Возвращает:
            -1 - ошибка
            0 - время ожидания истекло
            >0 - общее количество готовых файловых дескрипторов
        */
        int r = select(STDIN_FILENO + 1, &rfds, NULL, NULL, &tv);
        if (r < 0) {
            if (errno == EINTR) continue;
            perror("select");
            break;
        }
        if (r == 0) continue;

        // Чтение сообщения
        if (fgets(buf, sizeof(buf), stdin) == NULL) {
            if (errno == EINTR) {
                continue; // если прервано сигналом, продолжаем
            }
            break; // ошибка или конец файла
        }

        size_t len = strlen(buf);
        if (len > 0 && buf[len - 1] == '\n') buf[--len] = '\0';
        if (len == 0) continue;

        if (mq_send(queue, buf, len + 1, NORMAL_PRIORITY) < 0)
            perror("mq_send");
    }

    if (!stop) {
        /* Вышли по EOF — receiver ещё работает, собеседник не в курсе. */
        stop = 1;
        need_notify = 1;
    }

    if (need_notify) {
        const char exit_msg[] = "exit";
        if (mq_send(queue, exit_msg, sizeof(exit_msg), EXIT_PRIORITY) < 0)
            perror("mq_send (EXIT)");
    }
    return NULL;
}

/*
Функция потока-получателя сообщений. Она получает сообщения из очереди сообщений и выводит их на экран. 
Если получено сообщение с приоритетом EXIT_PRIORITY, поток завершает свою работу.
*/
void *receiver_thread(void* arg) {
    mqd_t queue = *(mqd_t*)arg;
    char buf[MSG_SIZE];
    unsigned int priority;

    while (!stop) {
        struct timespec ts; // время: tv_sec (секунды) + tv_nsec (наносекунды)
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_nsec += TIMEOUT * 1000000L; // Перевод мс в нс
        ts.tv_sec += ts.tv_nsec / 1000000000L; // Сколько целых секунд накопилось в tv_nsec прибавляем к секундам
        ts.tv_nsec %= 1000000000L; // Остаток должен быть в диапазоне [0, 999999999]
        // Читаем сообщение из очереди с таймаутом
        ssize_t bytes_read = mq_timedreceive(queue, buf, MSG_SIZE, &priority, &ts);
        
        // Если при чтении сообщения ошибка
        if (bytes_read < 0) {
            // Если истекло время таймаута
            if (errno == ETIMEDOUT || errno == EINTR)
                continue;
            // Ошибка чтения
            perror("mq_timedreceive");
            break;
        }

        // Приоритет полученного сообещния равен приоритету выхода
        if (priority == EXIT_PRIORITY) {
            printf("Получено сообщение о выходе. Завершение работы.\n");
            stop = 1;
            break;
        }

        printf("[Собеседник]> %s\n", buf);
        fflush(stdout);
    }
    return NULL;
}