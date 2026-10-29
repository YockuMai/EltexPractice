#include "Interface.h"

typedef struct {
    const char *name;
    char symbol;
    double (*func)(double, double);
} Command;

void menu_run() {
    const Command commands[] = {
        {"сложение", '+', add},
        {"вычитание", '-', subtract},
        {"умножение", '*', multiply},
        {"деление", '/', divide},
    };
    const int count = (int)(sizeof(commands) / sizeof(commands[0]));

    while (1) {
        puts("\nКАЛЬКУЛЯТОР С ДИНАМИЧЕСКИМ МЕНЮ");
        puts(" 0) выход");
        for (int i = 0; i < count; ++i) {
            printf(" %d) %-10s (%c)\n",
                   i + 1, commands[i].name, commands[i].symbol);
        }

        int choice;
        printf("Выбор: ");
        if (scanf("%d", &choice) != 1) {
            int c; while ((c = getchar()) != '\n' && c != EOF) {}
            puts("Некорректный ввод.");
            continue;
        }
        if (choice == 0) break;
        if (choice < 1 || choice > count) {
            puts("Такой команды нет.");
            continue;
        }

        double a, b;
        printf("a = ");
        if (scanf("%lf", &a) != 1) {
            int c; while ((c = getchar()) != '\n' && c != EOF) {}
            puts("Некорректное число.");
            continue;
        }
        printf("b = ");
        if (scanf("%lf", &b) != 1) {
            int c; while ((c = getchar()) != '\n' && c != EOF) {}
            puts("Некорректное число.");
            continue;
        }

        double result = commands[choice - 1].func(a, b);

        if (isnan(result)) {
            puts("Ошибка: деление на ноль! ");
        } else if (isinf(result)) {
            printf("Результат: %sбесконечность\n",
                   result < 0 ? "-" : "+");
        } else {
            printf("Результат: %g\n", result);
        }

        getchar();
        getchar();
        system("clear");
    }

}