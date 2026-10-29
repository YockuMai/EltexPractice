#include "ui.h"
#include "parsers.h"
#include "getPermissions.h"
#include "setPermissions.h"
#include "perm_bits.h"

#include <stdio.h>
#include <stdlib.h>
#include "Strings.h"

// Состояние сессии
static unsigned current_mode = 0;
static int has_mode = 0;

// Чтение строки 
static int read_line(char *buf, size_t sz) {
    if (!fgets(buf, sz, stdin)) return -1;
    char *nl = my_strchr(buf, '\n');
    if (nl) *nl = '\0';
    return 0;
}

// Печать представлений прав
static void print_all(unsigned m) {
    char *s = mode_to_symbolic(m);
    char *n = mode_to_numeric(m);
    char *b = mode_to_binary(m);
    if (!s || !n || !b) { free(s); free(n); free(b); return; }
    printf("    буквенное: %s\n", s);
    printf("    цифровое:  %s\n", n);
    printf("    битовое:   %s\n", b);
    free(s); free(n); free(b);
}

// Ввод прав (вручную)
static void do_item1(void) {
    printf("Введите права (755 / 4755 / rwxr-xr-x): ");
    char buf[64];
    if (read_line(buf, sizeof buf) < 0 || !*buf) return;

    int m = parse_mode(buf);
    if (m == PARSE_ERROR) { printf("  неверный формат\n"); return; }

    current_mode = (unsigned)m;
    has_mode = 1;
    print_all(current_mode);
}

// Получение прав доступа по имени файла
static void do_item2(void) {
    printf("Имя файла: ");
    char path[4096];
    if (read_line(path, sizeof path) < 0 || !*path) return;

    int st_mode = file_get_st_mode(path);
    if (st_mode < 0) { perror(path); return; }

    unsigned perm = (unsigned)st_mode & 07777;
    current_mode = perm;
    has_mode = 1;

    char *ls_sym = st_mode_to_ls_symbolic((unsigned)st_mode);
    char *ref = ls_l_perm_string(path);

    if (ls_sym) printf("  %s\n", ls_sym);
    print_all(perm);
    if (ref) {
        printf("\n    ls -ld:   %s\n", ref);
        printf("    сравнение: %s\n",
               (ls_sym && my_strcmp(ls_sym, ref) == 0)
                   ? "совпадает" : "РАСХОЖДЕНИЕ");
        free(ref);
    }
    free(ls_sym);
}

// Изменение прав доступа
static void do_item3(void) {
    if (!has_mode) {
        printf("  Сначала введите права (п.1) или файл (п.2).\n");
        return;
    }

    printf("chmod-команда (u+x / go-w / a=r / 755): ");
    char buf[128];
    if (read_line(buf, sizeof buf) < 0 || !*buf) return;

    int new_mode = apply_chmod(current_mode, buf);
    if (new_mode == PARSE_ERROR) { printf("  неверная команда\n"); return; }

    printf("  После модификации (файл на диске не менялся):\n");
    print_all((unsigned)new_mode);
    current_mode = (unsigned)new_mode;
}

// Запуск меню
int run_menu(void) {
    while (1) {
        printf("\nИМИТАЦИЯ РАБОТЫ С ПРАВАМИ ДОСТУПА\n\n");
        printf("1. Ввести права\n");
        printf("2. Ввести имя файла для получения прав\n");
        printf("3. Модифицировать текущие права\n");
        printf("0. Выход\n");

        if (has_mode) {
            puts("\nТекущие права: ");
            print_all(current_mode);
        }
        printf("\nВыбор: ");

        char buf[16];
        if (read_line(buf, sizeof buf) < 0) break;
        if (!my_strcmp(buf, "0")) break;
        else if (!my_strcmp(buf, "1")) do_item1();
        else if (!my_strcmp(buf, "2")) do_item2();
        else if (!my_strcmp(buf, "3")) do_item3();
        else printf("  Нет такого пункта\n");
    }
    return 0;
}