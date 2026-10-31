#include "ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// payload -> prio, чтобы печатать приоритет извлечённого
#define MAX_PAYLOAD 65536
static int prio_of[MAX_PAYLOAD];
static int next_payload = 0;

static int gen_payload(void) {
    int p = next_payload++;
    if (next_payload >= MAX_PAYLOAD) next_payload = 0;
    return p;
}

// ввод целого
static int read_int(const char *prompt, int *out) {
    printf("%s", prompt);
    fflush(stdout);

    if (scanf("%d", out) != 1) {
        int c;
        while ((c = getchar()) != '\n' && c != EOF) {}
        return 0;
    }
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
    return 1;
}

static void print_result(const char *op, int ok, int payload) {
    if (ok)
        printf("  [%s] -> данные=%d приоритет=%d\n",
               op, payload, prio_of[payload]);
    else
        printf("  [%s] -> ПУСТО / не найдено\n", op);
}

// операции меню

static void do_push(PQueue *pq) {
    int payload, prio;
    if (!read_int("  данные (0..65535): ", &payload) || payload < 0 || payload >= MAX_PAYLOAD) {
        puts("  некорректные данные"); return;
    }
    if (!read_int("  приоритет (0..255): ", &prio)) {
        puts("  некорректный приоритет"); return;
    }
    if (!pq_push(pq, payload, prio)) {
        puts("  добавление не удалось (неверный приоритет)"); return;
    }
    prio_of[payload] = prio;
    printf("  добавлено: данные=%d приоритет=%d\n", payload, prio);
}

static void do_generate(PQueue *pq) {
    int n;
    if (!read_int("  сколько сообщений сгенерировать: ", &n) || n <= 0) {
        puts("  некорректное количество"); return;
    }
    for (int i = 0; i < n; ++i) {
        int prio = rand() % N_PRIO;
        int payload = gen_payload();
        if (!pq_push(pq, payload, prio)) {
            puts("  добавление не удалось"); return;
        }
        prio_of[payload] = prio;
        printf("  #%d: данные=%d приоритет=%d\n", i + 1, payload, prio);
    }
}

static void do_pop_first(PQueue *pq) {
    int v;
    int ok = pq_pop_first(pq, &v);
    print_result("первый в очереди", ok, v);
}

static void do_pop_prio(PQueue *pq) {
    int prio;
    if (!read_int("  приоритет (0..255): ", &prio)) {
        puts("  некорректный ввод"); return;
    }
    int v;
    int ok = pq_pop_prio(pq, prio, &v);
    print_result("с указанным приоритетом", ok, v);
}

static void do_pop_at_least(PQueue *pq) {
    int prio;
    if (!read_int("  граница приоритета (0..256): ", &prio)) {
        puts("  некорректный ввод"); return;
    }
    int v;
    int ok = pq_pop_at_least(pq, prio, &v);
    print_result("приоритет лучше заданного", ok, v);
}

static void do_drain(PQueue *pq) {
    int v, n = 0;
    while (pq_pop_first(pq, &v)) {
        printf("  извлечено #%d: данные=%d приоритет=%d\n",
               ++n, v, prio_of[v]);
    }
    printf("  всего извлечено: %d\n", n);
}

static void do_dump(const PQueue *pq) {
    int total = 0;
    for (int p = 0; p < N_PRIO; ++p) {
        if (!pq->q[p].head) continue;
        printf("  приоритет %3d: ", p);
        for (const Node *n = pq->q[p].head; n; n = n->next) {
            printf("%d ", n->payload);
            ++total;
        }
        putchar('\n');
    }
    if (total == 0) puts("  (очередь пуста)");
    else printf("  всего сообщений: %d\n", total);
}

// меню

static void print_menu(void) {
    puts("\nОЧЕРЕДЬ С ПРИОРИТЕТОМ");
    puts(" 1. Добавить сообщение вручную");
    puts(" 2. Сгенерировать случайные сообщения");
    puts(" 3. Извлечь первый в очереди");
    puts(" 4. Извлечь с указанным приоритетом");
    puts(" 5. Извлечь с приоритетом выше заданного");
    puts(" 6. Показать очередь");
    puts(" 7. Извлечь всё");
    puts(" 0. Выход");
    printf("> ");
    fflush(stdout);
}

void ui_run(PQueue *pq) {
    srand((unsigned)time(NULL));

    while (1) {
        print_menu();
        int choice;
        if (!read_int("", &choice)) {
            puts("\n(конец ввода / ошибка) -- выход");
            return;
        }
        switch (choice) {
            case 1: do_push(pq); break;
            case 2: do_generate(pq); break;
            case 3: do_pop_first(pq); break;
            case 4: do_pop_prio(pq); break;
            case 5: do_pop_at_least(pq); break;
            case 6: do_dump(pq); break;
            case 7: do_drain(pq); break;
            case 0: return;
            default: puts("  неизвестный пункт меню");
        }
    }
}