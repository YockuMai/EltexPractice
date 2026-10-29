#ifndef SET_PERMISSIONS_H
#define SET_PERMISSIONS_H

/* Применить chmod-спеку к old_mode. Файл НЕ трогается.
 * Возвращает новый mode (0..07777) или PARSE_ERROR. */
int apply_chmod(unsigned old_mode, const char *spec);

#endif /* SET_PERMISSIONS_H */