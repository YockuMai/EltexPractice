#include "Interface.h"
#include "Operation.h"

#include <dirent.h>
#include <dlfcn.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#define MAX_COMMANDS 4

typedef struct {
    const char *name;
    char symbol;
    double (*func)(double, double);
    void *handle;
} Command;

static void clear_input(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }
}

static void load_commands(const char *dir, Command *cmds, int *count) {
    *count = 0;
    DIR *d = opendir(dir);
    if (!d) { perror(dir); return; }

    struct dirent *ent;
    while ((ent = readdir(d)) && *count < MAX_COMMANDS) {

        char path[4096];
        snprintf(path, sizeof path, "%s/%s", dir, ent->d_name);

        struct stat st;
        if (stat(path, &st) != 0 || !S_ISREG(st.st_mode))
            continue;

        void *h = dlopen(path, RTLD_NOW);
        if (!h) { fprintf(stderr, "%s\n", dlerror()); continue; }

        Command *c = &cmds[*count];
        const char **n = dlsym(h, "calc_name");
        const char *s = dlsym(h, "calc_symbol");
        void *f = dlsym(h, "calc_func");

        // При ошибке
        if (!n || !s || !f) {
            dlclose(h);
            continue;
        }

        c->name = *n;
        c->symbol = *s;
        c->func = (double (*)(double, double))f;
        c->handle = h;
        (*count)++;
    }
    closedir(d);
}

void menu_run(const char *lib_dir) {
    Command cmds[MAX_COMMANDS];
    int count = 0;
    load_commands(lib_dir, cmds, &count);

    if (count == 0) {
        fprintf(stderr, "Не найдено библиотек в %s\n", lib_dir);
        return;
    }

    while (1) {
        puts("\nКАЛЬКУЛЯТОР");
        puts(" 0) выход");
        for (int i = 0; i < count; i++)
            printf(" %d) %s (%c)\n", i + 1, cmds[i].name, cmds[i].symbol);

        int choice;
        printf("Выбор: ");
        if (scanf("%d", &choice) != 1) { clear_input(); puts("Ошибка ввода."); continue; }
        clear_input();

        if (choice == 0) break;
        if (choice < 1 || choice > count) { puts("Нет такой команды."); continue; }

        double a, b;
        printf("a = ");
        if (scanf("%lf", &a) != 1) { clear_input(); puts("Ошибка."); continue; }
        printf("b = ");
        if (scanf("%lf", &b) != 1) { clear_input(); puts("Ошибка."); continue; }
        clear_input();

        double r = cmds[choice - 1].func(a, b);
        if (isnan(r))        puts("Деление на ноль!");
        else if (isinf(r))   printf("Результат: %sбесконечность\n", r < 0 ? "-" : "+");
        else                 printf("Результат: %g\n", r);

        puts("Enter...");
        getchar();
        system("clear");
    }

    for (int i = 0; i < count; i++) dlclose(cmds[i].handle);
}