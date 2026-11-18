#include "common.h"
#include "producer.h"

#define MAX_COUNT_NUMBERS 20

#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <semaphore.h>
#include <sys/mman.h>

static void producer_release(sem_t *sem, int fd, char *base, const char* sem_name, const char* shm_name) {
    if (base != NULL && base != MAP_FAILED)
        munmap(base, SHM_SIZE);
    if (fd != -1) {
        close(fd);
        shm_unlink(shm_name);
    }
    if (sem != NULL && sem != SEM_FAILED) {
        sem_close(sem);
        sem_unlink(sem_name);
    }
}

void run_producer(const char *shm_name, const char *sem_name) {
    signal(SIGINT, onsigint);

    // Создание семафора
    sem_t *sem = sem_open(sem_name, O_CREAT | O_EXCL, 0666, 1);
    if (sem == SEM_FAILED) {
        if (errno == EEXIST)
            printf("[Производитель] Ошибка: ресурсы уже созданы. "
                   "Возможно, программа уже запущена.\n");
        else
            perror("sem_open");
        return;
    }

    // Создание разделяемой памяти
    int fd = shm_open(shm_name, O_CREAT | O_EXCL | O_RDWR, 0666);
    if (fd == -1) {
        if (errno == EEXIST)
            printf("[Производитель] Ошибка: разделяемая память уже существует.\n");
        else
            perror("shm_open");
        producer_release(sem, -1, NULL, sem_name, shm_name);
        return;
    }

    // Создание разделяемой памяти заданного размера
    if (ftruncate(fd, SHM_SIZE) == -1) {
        perror("ftruncate");
        producer_release(sem, fd, NULL, sem_name, shm_name);
        return;
    }

    // Связывание разделямой памяти с памятью процесса
    char *base = (char *)mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); // Подключение памяти к процессу
    if (base == MAP_FAILED) {
        perror("mmap");
        producer_release(sem, fd, NULL, sem_name, shm_name);
        return;
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
        sem_wait(sem);

        long long current_off = ctrl->free_off;
        block *b = (block*)(base + current_off);

        b->count = count;
        b->next_off = 0;

        // Заполняем массив
        int* data = (int *)(base + current_off + sizeof(block));
        for (int i = 0; i < count; i++) {
            data[i] = rand() % 100;
        }

        // Связываем список
        if (prev_off == 0)
            ctrl->head_off = current_off;
        else {
            block* prev = (block*)(base + prev_off);
            prev->next_off = current_off;
        }
        prev_off = current_off;
        ctrl->free_off += block_size;

        // Освобождение ресурса
        sem_post(sem);

        sleep(0.5);
    }

    sem_wait(sem);
    ctrl->producer_done = 1;
    sem_post(sem);

    printf("[Производитель] Генерация завершена. Ожидание потребителей...\n");

    // Ожидание потребителей
    while (!stop) {
        sem_wait(sem);
        
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

        sem_post(sem);

        if (all_processed) {
            printf("[Производитель] Все наборы обработаны. Удаляю ресурсы.\n");
            break;
        }

        sleep(0.5);
    }

    if (stop)
        printf("\n[Производитель] Прерван сигналом. Принудительная очистка...\n");

    // Очистка ресурсов
    producer_release(sem, fd, base, sem_name, shm_name);
    printf("[Производитель] Завершил работу.\n");
}