#include "Strings.h"
#include <stdlib.h>

size_t my_strlen(const char *s) {
    size_t n = 0;
    while (s[n] != '\0') n++;
    return n;
}

char *my_strdup(const char *s) {
    size_t n = my_strlen(s);
    char *p = malloc(n + 1);
    if (!p) return NULL;
    for (size_t i = 0; i <= n; i++) p[i] = s[i];
    return p;
}

int my_strcmp(const char *a, const char *b) {
    while (*a && *a == *b) { a++; b++; }
    return (int)(unsigned char)*a - (int)(unsigned char)*b;
}

char *my_strchr(const char *s, int c) {
    char ch = (char)c;
    while (*s != ch) {
        if (*s == '\0') return NULL;
        s++;
    }
    return (char *)s;
}

void *my_memcpy(void *dst, const void *src, size_t n) {
    unsigned char       *d = dst;
    const unsigned char *s = src;
    for (size_t i = 0; i < n; i++) d[i] = s[i];
    return dst;
}