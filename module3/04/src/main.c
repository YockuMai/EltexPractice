#include "producer.h"
#include "consumer.h"

#include <stdio.h>
#include <bits/getopt_core.h>

int main(int argc, char **argv) {
    int opt;
    int is_producer = 0, is_consumer = 0;

    while ((opt = getopt(argc, argv, "pc")) != -1) {
        switch (opt) {
            case 'p': is_producer = 1; break;
            case 'c': is_consumer = 1; break;
            default:
                fprintf(stderr, "Использование: %s -p (производитель) | -c (потребитель)\n", argv[0]);
                return 1;
        }
    }

    if (is_producer && is_consumer) {
        fprintf(stderr, "Выберите что-то одно: -p или -c\n");
        return 1;
    } else if (is_producer) {
        run_producer();
    } else if (is_consumer) {
        run_consumer();
    } else {
        fprintf(stderr, "Укажите роль: -p или -c\n");
        return 1;
    }

    return 0;
}