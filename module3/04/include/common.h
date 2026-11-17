#ifndef COMMON_H
#define COMMON_H

#include <sys/sem.h>
#include <signal.h>

#define SHM_SIZE 512
#define KEY_PATH "."

typedef struct shm_control {
    long long head_off; // Смещение первого блока (0 - список пуст)
    long long free_off; // Смещение для следующей записи
    int producer_done; // 1 - генерация завершена
} shm_control;

typedef struct block {
    long long count; // Количество элементов (0 - обработан)
    long long next_off; // Смещение следующего блока (0 - последний)
} block;

// Семафорные структуры
static struct sembuf lock_s = {0, -1 ,0};
static struct sembuf unlock_s[2] = {{0, 0, 0}, {0, 1, 0}};

// Для обработки сигнала
static volatile sig_atomic_t stop = 0;

static void onsigint(int sig) {
    (void)sig; // Подавляем предупреждение о неиспользуемом параметре
    stop = 1;
}

#endif