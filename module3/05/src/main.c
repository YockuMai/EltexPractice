#include "producer.h"
#include "consumer.h"

#include <stdio.h>
#include <bits/getopt_core.h>
#include <unistd.h>
#include <string.h>

#define NAME_MAX_LEN 96

int main(int argc, char **argv) {
    int opt;
    int is_producer = 0, is_consumer = 0;
    const char *session = "prod_cons";

    while ((opt = getopt(argc, argv, "pcn:")) != -1) {
        switch (opt) {
            case 'p': is_producer = 1; break;
            case 'c': is_consumer = 1; break;
            case 'n': session = optarg; break;
            default:
                fprintf(stderr, "Использование: %s -p (производитель) | -c (потребитель)\n", argv[0]);
                return 1;
        }
    }

    if (is_producer == is_consumer) {
        fprintf(stderr, "Укажите ровно одно: -p или -c\n");
        return 1;
    }

    if (strchr(session, '/')) {
        fprintf(stderr, "SESSION не должен содержать '/'\n");
        return 1;
    }

    char shm_name[NAME_MAX_LEN];
    char sem_name[NAME_MAX_LEN];

    if (snprintf(shm_name, sizeof shm_name, "/%s_%u.shm",
                 session, getuid()) >= (int)sizeof(shm_name) ||
        snprintf(sem_name, sizeof sem_name, "/%s_%u.sem",
                 session, getuid()) >= (int)sizeof(sem_name)) {
        fprintf(stderr, "SESSION слишком длинный\n");
        return 1;
    }

    if (is_producer) run_producer(shm_name, sem_name);
    else run_consumer(shm_name, sem_name);
    return 0;
}