#ifndef STRINGS_H
#define STRINGS_H

#include <stddef.h>   /* size_t */

size_t my_strlen(const char *s);
char  *my_strdup(const char *s);
int    my_strcmp(const char *a, const char *b);
char  *my_strchr(const char *s, int c);
void  *my_memcpy(void *dst, const void *src, size_t n);

#endif /* STRINGS_H */