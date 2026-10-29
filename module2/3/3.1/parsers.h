#ifndef PARSERS_H
#define PARSERS_H

/* Единая точка входа: определяет формат и парсит.
 * Возвращает mode (0..07777) или PARSE_ERROR. */
int parse_mode(const char *s);

/* Явные парсеры (нужны для тестов и для chmod). */
int parse_numeric(const char *s);   /* "755" / "4755"         */
int parse_symbolic(const char *s);  /* "rwxr-xr-x" / "rwsr-t" */

/* Форматтеры: возвращают malloc-строку, вызывающий free().
 * NULL — ошибка выделения памяти. */
char *mode_to_symbolic(unsigned mode);  /* "rwxr-xr-x"    */
char *mode_to_numeric(unsigned mode);   /* "755" / "4755" */
char *mode_to_binary(unsigned mode);    /* "111 101 101"  */

#endif