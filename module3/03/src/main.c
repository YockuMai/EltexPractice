#include "queue.h"
#include "my_threads.h"
#define MAX_LENGTH_QUEUE_NAME 64

#include <stdio.h>
#include <pthread.h>
#include <mqueue.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Использование: %s имя_очереди\n", argv[0]);
        return 1;
    }

    char name1[MAX_LENGTH_QUEUE_NAME], name2[MAX_LENGTH_QUEUE_NAME];
    snprintf(name1, sizeof(name1), "/%s_1", argv[1]);
    snprintf(name2, sizeof(name2), "/%s_2", argv[1]);

    mqd_t q1, q2;
    int r1 = create_queue(&q1, name1, 0666, MAX_MSGS, MSG_SIZE);
    if (r1 < 0) return 1;
    int is_creator = (r1 == 0);

    if (create_queue(&q2, name2, 0666, MAX_MSGS, MSG_SIZE) < 0) {
        mq_close(q1);
        if (is_creator) mq_unlink(name1);
        return 1;
    }

    mqd_t recv_q = is_creator ? q1 : q2;
    mqd_t send_q = is_creator ? q2 : q1;

    printf("[%s] приём: %s, отправка: %s\n",
           is_creator ? "Создатель" : "Клиент",
           is_creator ? name1 : name2,
           is_creator ? name2 : name1);

    // Обработчик сигнала SIGINT
    threads_install_sigint();

    // Два потока: отправитель и получатель
    pthread_t st, rt;
    // Создаёт отдельный поток
    pthread_create(&st, NULL, sender_thread,   &send_q);
    pthread_create(&rt, NULL, receiver_thread, &recv_q);
    
    // Ожидание завершения потоков
    pthread_join(st, NULL);
    pthread_join(rt, NULL);

    // Закрываем и (если создатель) удаляем
    mq_close(q1);
    mq_close(q2);
    if (is_creator) {
        mq_unlink(name1);
        mq_unlink(name2);
        printf("Удаление очередей\n");
    }
    return 0;
}