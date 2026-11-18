#include "common.h"
#include "consumer.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <fcntl.h>

void run_consumer(const char *shm_name, const char *sem_name) {
    signal(SIGINT, onsigint);

    sem_t *sem = sem_open(sem_name, 0);
    int fd = shm_open(shm_name, O_RDWR, 0);

    if (sem == SEM_FAILED || fd == -1) {
        fprintf(stderr,
                "[Потребитель %d] Ошибка: запустите сначала производителя!\n",
                getpid());
        if (sem != SEM_FAILED) sem_close(sem);
        if (fd != -1) close(fd);
        exit(1);
    }

    // Связывание разделямой памяти с памятью процесса
    char *base = (char *)mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (base == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    shm_control *ctrl = (shm_control *)base;

    printf("[Потребитель %d] Подключился к памяти.\n", getpid());

    while (!stop) {
        sem_wait(sem);

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
            sem_post(sem);
            sleep(1);
        } 
        else {
            if (ctrl->producer_done) {
                sem_post(sem);
                printf("[Потребитель %d] Все наборы обработаны. Отключаюсь.\n", getpid());
                break;
            } else {
                sem_post(sem);
                sleep(1);
            }
        }
    }

    if (stop)
        printf("\n[Потребитель] Прерван сигналом. Принудительная очистка...\n");

    munmap(base, SHM_SIZE);
    close(fd);
    sem_close(sem);
}