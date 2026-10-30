#include "ui.h"
#include "ipv4_work.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

// Число единичных битов в маске = длина префикса k (для 255.255.255.0 → 24)
static int popcount_u32(uint32_t x)
{
    int c = 0;
    while (x) { c += (int)(x & 1u); x >>= 1; }
    return c;
}

/* Читает строку в buf (не длиннее size-1). Возвращает 1 при успехе,
   0 при EOF/ошибке. Завершающий '\n' срезается. */
static int read_line(char *buf, size_t size)
{
    if (!fgets(buf, (int)size, stdin))
        return 0;
    size_t len = strlen(buf);
    if (len && buf[len - 1] == '\n')
        buf[len - 1] = '\0';
    return 1;
}

// Спрашивает IP, пока не введут корректный. Возвращает 0 при EOF
static int ask_ipv4(const char *prompt, const char *hint, uint32_t *out)
{
    char line[128];
    for (;;) {
        printf("%s", prompt);
        if (hint && *hint)
            printf(" (%s)", hint);
        printf(": ");
        fflush(stdout);

        if (!read_line(line, sizeof line)) {
            putchar('\n');
            return 0;
        }
        if (parse_ipv4(line, out) == 0)
            return 1;
        printf("  Ошибка: \"%s\" — это не IPv4-адрес вида a.b.c.d.\n\n", line);
    }
}

// Корректная маска — непрерывные единицы слева
static int mask_is_valid(uint32_t mask)
{
    uint32_t inv = ~mask;
    return (inv & (inv + 1u)) == 0u;
}

/* Принимает маску в любом из форматов:
     "255.255.255.0" - точечная запись
     "24"  или "/24" - длина префикса (0..32)
   Возвращает 0 при успехе, -1 при ошибке. */
static int parse_mask(const char *s, uint32_t *out)
{
    const char *p = s;
    if (*p == '/')
        p++;

    char *end = NULL;
    long  v   = strtol(p, &end, 10);
    if (end != p && *end == '\0') {
        if (v < 0 || v > 32)
            return -1;
        *out = (v == 0) ? 0u
                        : (uint32_t)(0xFFFFFFFFu << (32 - (int)v));
        return 0;
    }

    return parse_ipv4(s, out);
}

// Спрашивает маску, пока не введут корректную. Возвращает 0 при EOF
static int ask_mask(const char *prompt, uint32_t *out)
{
    char line[128];
    for (;;) {
        printf("%s (255.255.255.0 или /24): ", prompt);
        fflush(stdout);

        if (!read_line(line, sizeof line)) {
            putchar('\n');
            return 0;
        }
        uint32_t m;
        if (parse_mask(line, &m) != 0) {
            printf("  Ошибка: \"%s\" — нужен адрес вида a.b.c.d "
                   "или префикс /0../32.\n\n", line);
            continue;
        }
        if (!mask_is_valid(m)) {
            printf("  Ошибка: \"%s\" — это не маска подсети "
                   "(единичные биты должны идти подряд слева).\n\n", line);
            continue;
        }
        *out = m;
        return 1;
    }
}

// Спрашивает N, пока не введут положительное целое. 0 при EOF.
static int ask_count(const char *prompt, long *out)
{
    char line[64];
    for (;;) {
        printf("%s: ", prompt);
        fflush(stdout);

        if (!read_line(line, sizeof line)) {
            putchar('\n');
            return 0;
        }
        char *end = NULL;
        long  n   = strtol(line, &end, 10);
        if (end == line || *end != '\0' || n <= 0) {
            printf("  Ошибка: \"%s\" — нужно целое положительное число.\n\n",
                   line);
            continue;
        }
        *out = n;
        return 1;
    }
}

int run_program(void)
{
    uint32_t gw, mask;
    long n;

    printf("ИМИТАЦИЯ ОТПРАВКИ ПАКЕТОВ IPV4\n\n");

    if (!ask_ipv4("Введите IP-адрес шлюза", "например, 192.168.1.1", &gw))
        return EXIT_FAILURE;
    if (!ask_mask("Введите маску подсети", &mask))
        return EXIT_FAILURE;
    if (!ask_count("Введите количество пакетов N", &n))
        return EXIT_FAILURE;

    uint32_t net = gw & mask;

    int k = popcount_u32(mask);            // длина префикса
    uint64_t subnet_size = (uint64_t)1u << (32 - k);      // адресов в подсети
    uint64_t total_v4 = (uint64_t)1u << 32;            // всего IPv4-адресов
    double p_own = 1.0 / (double)((uint64_t)1u << k);
    double p_other = 1.0 - p_own;
    double exp_own = (double)n * p_own;
    double exp_other = (double)n * p_other;

    printf("\nТЕОРИЯ\n");
    printf("Длина префикса: /%d\n", k);

    printf("Всего адресов IPv4: %llu шт.\n",
           (unsigned long long)total_v4);

    printf("Адресов в подсети: %llu шт.\n", (unsigned long long)subnet_size);

    printf("Вероятность «своей»: %.6g%%\n", 100.0 * p_own);
    printf("Вероятность «чужой»: %.6g%%\n", 100.0 * p_other);

    printf("Ожидаемое количество \"своих\": %.0f шт.\n", exp_own);
    printf("Ожидаемое количество \"чужих\": %.0f шт.\n\n", exp_other);

    srand((unsigned)time(NULL));

    long own = 0;
    long foreign = 0;

    for (long i = 0; i < n; i++) {
        uint32_t dst = rand_u32();
        int is_own = ((dst & mask) == net);

        if (is_own) own++;
        else foreign++;
    }

    printf("ПРАКТИКА\n");
    printf("Вероятность «своей»: %.6g%%\n", 100 * (double)own / (double)n);
    printf("Вероятность «чужой»: %.6g%%\n", 100 * (double)foreign / (double)n);
    printf("Количество \"своих\": %ld шт.\n", own);
    printf("Количество \"чужих\": %ld шт.\n", foreign);

    return EXIT_SUCCESS;
}