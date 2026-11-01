#include "Strings.h"

// Возвращает длину строки s (не включая завершающий нулевой символ)
size_t my_strlen(const char* s) {
    size_t n = 0;
    while (s[n] != '\0') n++;
    return n;
}

// Возвращает дубликат строки s, выделяя память через malloc. NULL при ошибке памяти
char* my_strdup(const char* s) {
    size_t n = my_strlen(s);
    char* p = malloc(n + 1);
    if (!p) return NULL;
    for (size_t i = 0; i <= n; i++) p[i] = s[i];
    return p;
}

// Сравнение строк как strcmp: <0, 0, >0
int my_strcmp(const char* a, const char* b) {
    while (*a && *a == *b) {
        a++;
        b++;
    }

    unsigned char ca = (unsigned char)*a;
    unsigned char cb = (unsigned char)*b;

    if (ca < cb) return -1;
    if (ca > cb) return 1;
    return 0;
}