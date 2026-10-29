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