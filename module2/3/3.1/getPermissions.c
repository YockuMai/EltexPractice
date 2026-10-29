#include "getPermissions.h"
#include "parsers.h"

#include <stdio.h>
#include <stdlib.h>
#include "Strings.h"
#include <sys/stat.h>
#include <sys/types.h>

// Получение st_mode файла 
int file_get_st_mode(const char *path) {
    struct stat st;
    if (stat(path, &st) != 0) return -1;
    return (int)st.st_mode;
}

// Определение типа файла
char st_mode_type_char(unsigned st_mode) {
    switch (st_mode & S_IFMT) {
        case S_IFREG: return '-';
        case S_IFDIR: return 'd';
        case S_IFLNK: return 'l';
        case S_IFCHR: return 'c';
        case S_IFBLK: return 'b';
        case S_IFIFO: return 'p';
        case S_IFSOCK: return 's';
        default: return '?';
    }
}

// Перевод битов в символьный вид
char *st_mode_to_ls_symbolic(unsigned st_mode) {
    char *perm = mode_to_symbolic(st_mode & 07777); // Передаём только биты прав
    if (!perm) return NULL;
    char *out = malloc(11);
    if (!out) { free(perm); return NULL; }
    out[0] = st_mode_type_char(st_mode);
    my_memcpy(out + 1, perm, 10);
    free(perm);
    return out;
}

// Выделение прав доступа из команды ls -ld
char *ls_l_perm_string(const char *path) {
    size_t n = my_strlen(path);
    char *cmd = malloc(n * 4 + 32);
    if (!cmd) return NULL;

    char *p = cmd;
    p += sprintf(p, "ls -ld -- '");
    for (size_t i = 0; i < n; i++) {
        if (path[i] == '\'') { my_memcpy(p, "'\\''", 4); p += 4; }
        else *p++ = path[i];
    }
    *p++ = '\'';
    *p = '\0';

    FILE *fp = popen(cmd, "r");
    free(cmd);
    if (!fp) return NULL;

    char line[4096];
    char *result = NULL;
    if (fgets(line, sizeof line, fp)) {
        result = malloc(11);
        if (result) { my_memcpy(result, line, 10); result[10] = '\0'; }
    }
    pclose(fp);
    return result;
}