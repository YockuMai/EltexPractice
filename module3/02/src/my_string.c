#include "my_string.h"

size_t my_strlen(const char *s) {
    size_t n = 0;
    while (s[n]) n++;
    return n;
}

void my_strncpy(char *dst, const char *src, size_t n) {
    size_t i = 0;
    if (!n) return;
    while (i + 1 < n && src[i]) { dst[i] = src[i]; i++; }
    dst[i] = 0;
}

int my_strcmp(const char *a, const char *b) {
    while (*a && *a == *b) { a++; b++; }
    return (int)(unsigned char)*a - (int)(unsigned char)*b;
}