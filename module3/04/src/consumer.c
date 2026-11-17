#include "common.h"
#include "consumer.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <unistd.h>

void run_consumer() {
    signal(SIGINT, onsigint);

    key_t shm_key = ftok(KEY_PATH, 'M');
    key_t sem_key = ftok(KEY_PATH, 'S');

    int semid = semget(sem_key, 1, 0);
    int shmid = shmget(shm_key, 0, 0);

    if (semid == -1 || shmid == -1) {
        fprintf(stderr, "[Потребитель %d] Ошибка: Запустите сначала производителя!\n", getpid());
        exit(1);
    }

    char *base = (char *)shmat(shmid, NULL, 0);
    if (base == (char *)-1) {
        perror("shmat"); 
        exit(1);
    }

    shm_control *ctrl = (shm_control *)base;

    printf("[Потребитель %d] Подключился к памяти.\n", getpid());

    while (!stop) {
        // Если производитель удалил семафор
        if (semop(semid, &lock_s, 1) == -1) {
            printf("[Потребитель %d] Производитель завершил свою работу.\n", getpid());
            break; 
        }

        long long off = ctrl->head_off;
        block *target = NULL;

        // Ищем первый необработанный блок
        while (off != 0) {
            block *b = (block *)(base + off);
            if (b->count > 0) {
                target = b;
                break;
            }
            off = b->next_off;
        }

        if (target) {
            int count = target->count;
            int *data = (int *)((char *)target + sizeof(block));
            int max = data[0], min = data[0];
            
            for (int i = 1; i < count; i++) {
                if (data[i] > max) max = data[i];
                if (data[i] < min) min = data[i];
            }

            printf("[Потребитель %d] Блок (смещение %lld): count=%d, max=%d, min=%d\n", 
                   getpid(), off, count, max, min);
            
            target->count = 0;
            if (semop(semid, unlock_s, 2) == -1) {
                perror("semop unlock");
                exit(1);
            }
            sleep(1);
        } 
        else {
            if (ctrl->producer_done) {
                if (semop(semid, unlock_s, 2) == -1) {
                    perror("semop unlock");
                    exit(1);
                }
                printf("[Потребитель %d] Все наборы обработаны. Отключаюсь.\n", getpid());
                break;
            } else {
                if (semop(semid, unlock_s, 2) == -1) {
                    perror("semop unlock");
                    exit(1);
                }
                sleep(1);
            }
        }
    }

    if (stop)
        printf("\n[Потребитель] Прерван сигналом. Принудительная очистка...\n");

    shmdt(base);
}