#ifndef PERM_BITS_H
#define PERM_BITS_H

/* Общие константы для всех модулей прав доступа. */

#define PARSE_ERROR (-1)

#define B_SETUID (1u << 11)
#define B_SETGID (1u << 10)
#define B_STICKY (1u << 9)

#endif