#ifndef GET_PERMISSIONS_H
#define GET_PERMISSIONS_H

/* st_mode файла (с битами типа) или -1 при ошибке (errno от stat). */
int file_get_st_mode(const char *path);

/* 'd', '-', 'l', 'c', 'b', 'p', 's' — для первой колонки ls -l. */
char st_mode_type_char(unsigned st_mode);

/* Строка вида "drwxr-xr-x" (malloc; вызывающий free()). */
char *st_mode_to_ls_symbolic(unsigned st_mode);

/* Только для тестов: первые 10 символов `ls -ld -- <path>` (malloc). */
char *ls_l_perm_string(const char *path);

#endif /* GET_PERMISSIONS_H */