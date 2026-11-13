#include "broker.h"
#include "my_string.h"
#include "publisher.h"
#include "subscriber.h"
#include <stdio.h>

int main(int argc, const char **argv) {
    if (argc == 2 && my_strcmp(argv[1], "-b") == 0) {
        return broker_run();
    } else if (argc == 3 && my_strcmp(argv[1], "-p") == 0) {
        return publisher_run(argv[2]);
    } else if (argc >= 3 && my_strcmp(argv[1], "-s") == 0) {
        return subscriber_run(argc - 2, (argv + 2));
    } else {
        fprintf(stderr, "Использование:\n");
        fprintf(stderr, "  %s -b Запуск брокера\n", argv[0]);
        fprintf(stderr, "  %s -p <тема> Запуск издателя\n", argv[0]);
        fprintf(stderr, "  %s -s <тема1> ... Запуск подписчика\n", argv[0]);
        return 1;
    }
}