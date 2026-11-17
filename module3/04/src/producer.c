#include "common.h"
#include "producer.h"

#define MAX_COUNT_NUMBERS 20

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

void run_producer() {
    signal(SIGINT, onsigint);

    key_t shm_key = ftok(KEY_PATH, 'M'); // Для разделяемой памяти
    key_t sem_key = ftok(KEY_PATH, 'S'); // Для семафора

    int semid = semget(sem_key, 1, IPC_CREAT | IPC_EXCL | 0666); // Создание нового набора семафоров
    if (semid < 0) {
        if (errno == EEXIST) {
            printf("[Производитель] Ошибка: Ресурсы уже созданы. Возможно, программа уже запущена.\n");
            exit(1);
        }
        perror("semget"); exit(1);
    }

    semctl(semid, 0, SETVAL, 1); // Инициализация семафора в 1

    int shmid = shmget(shm_key, SHM_SIZE, IPC_CREAT | IPC_EXCL | 0666); // Создание разделяемой памяти
    if (shmid < 0) {
        perror("shmget");
        exit(1);
    }

    char* base = (char*)shmat(shmid, NULL, 0); // Подключение памяти к процессу
    if (base == (char*)-1) {
        perror("shmat");
        exit(1);
    }

    shm_control* ctrl = (shm_control *) base;
    ctrl->head_off = 0;
    ctrl->free_off = sizeof(shm_control);
    ctrl->producer_done = 0;

    srand(time(NULL));
    long long prev_off = 0;

    // Генерация данных
    while (!stop) {
        int count = rand() % MAX_COUNT_NUMBERS + 1;
        long long block_size = sizeof(block) + count * sizeof(int);

        if (ctrl->free_off + block_size > SHM_SIZE) {
            printf("[Производитель] Память исчерпана.\n");
            break;
        }

        // Захват ресурса
        if (semop(semid, &lock_s, 1) == -1) {
            perror("semop lock");
            exit(1);
        }

        long long current_off = ctrl->free_off;
        block *b = (block*)(base + current_off);

        b->count = count;
        b->next_off = 0;

        // Заполняем массив
        int* data = (int *)(base + current_off + sizeof(block));
        for (int i = 0; i < count; i++) {
            data[i] = rand() % 100;
        }

        // Связываем "список"
        if (prev_off == 0)
            ctrl->head_off = current_off;
        else {
            block* prev = (block*)(base + prev_off);
            prev->next_off = current_off;
        }
        prev_off = current_off;
        ctrl->free_off += block_size;

        if (semop(semid, unlock_s, 2) == -1) {
            perror("semop unlock");
            exit(1);
        }

        sleep(1);
    }

    if (semop(semid, &lock_s, 1) == -1) {
        perror("semop lock");
        exit(1);
    }
    ctrl->producer_done = 1;
    if (semop(semid, unlock_s, 2) == -1) {
        perror("semop unlock");
        exit(1);
    }

    printf("[Производитель] Генерация завершена. Ожидание потребителей...\n");

    // Ожидание потребителей
    while (!stop) {
        if (semop(semid, &lock_s, 1) == -1) {
            perror("semop lock");
            exit(1);
        }
        
        int all_processed = 1;
        long long off = ctrl->head_off;

        while(off != 0) {
            block* b = (block *)(base + off);
            if (b->count != 0) {
                all_processed = 0;
                break;
            }
            off = b->next_off;
        }

        if (semop(semid, unlock_s, 2) == -1) {
            perror("semop unlock");
            exit(1);
        }

        if (all_processed) {
            printf("[Производитель] Все наборы обработаны. Удаляю ресурсы.\n");
            break;
        }

        sleep(1);
    }

    if (stop)
        printf("\n[Производитель] Прерван сигналом. Принудительная очистка...\n");

    // Очистка ресурсов
    shmdt(base);
    shmctl(shmid, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID);
    printf("[Производитель] Завершил работу.\n");
}