#include "parsers.h"
#include "perm_bits.h"

#include <stdio.h>
#include <stdlib.h>
#include "Strings.h"

// ---------- парсеры ---------- 

int parse_numeric(const char *s) {
    size_t n = my_strlen(s);
    if (n != 3 && n != 4) return PARSE_ERROR;
    unsigned m = 0;
    for (size_t i = 0; i < n; i++) {
        if (s[i] < '0' || s[i] > '7') return PARSE_ERROR;
        m = (m << 3) | (unsigned)(s[i] - '0');
    }
    return (int)m;
}

int parse_symbolic(const char *s) {
    if (my_strlen(s) != 9) return PARSE_ERROR;
    unsigned m = 0;
    for (int i = 0; i < 9; i++) {
        int bit = 8 - i;
        char c = s[i];
        if (i % 3 == 0) {
            if      (c == 'r') m |= 1u << bit;
            else if (c != '-') return PARSE_ERROR;
        } else if (i % 3 == 1) {
            if      (c == 'w') m |= 1u << bit;
            else if (c != '-') return PARSE_ERROR;
        } else {
            if      (c == 'x')            m |= 1u << bit;
            else if (c == '-')            { /* пусто */ }
            else if (c == 's' && i == 2)  m |= (1u << bit) | B_SETUID;
            else if (c == 'S' && i == 2)  m |= B_SETUID;
            else if (c == 's' && i == 5)  m |= (1u << bit) | B_SETGID;
            else if (c == 'S' && i == 5)  m |= B_SETGID;
            else if (c == 't' && i == 8)  m |= (1u << bit) | B_STICKY;
            else if (c == 'T' && i == 8)  m |= B_STICKY;
            else return PARSE_ERROR;
        }
    }
    return (int)m;
}

/* Решение «какой парсер» — это логика, не UI. */
int parse_mode(const char *s) {
    if (!s || !*s) return PARSE_ERROR;
    if (s[0] >= '0' && s[0] <= '7') return parse_numeric(s);
    return parse_symbolic(s);
}

/* ---------- форматтеры ---------- */

char *mode_to_symbolic(unsigned m) {
    char *out = malloc(10);
    if (!out) return NULL;
    for (int i = 0; i < 9; i++) {
        int bit = 8 - i;
        int set = (m >> bit) & 1;
        if (i % 3 == 0)      out[i] = set ? 'r' : '-';
        else if (i % 3 == 1) out[i] = set ? 'w' : '-';
        else {
            int sp = (i == 2 && (m & B_SETUID)) ||
                     (i == 5 && (m & B_SETGID)) ||
                     (i == 8 && (m & B_STICKY));
            if (sp) out[i] = set ? (i == 8 ? 't' : 's')
                                 : (i == 8 ? 'T' : 'S');
            else    out[i] = set ? 'x' : '-';
        }
    }
    out[9] = '\0';
    return out;
}

char *mode_to_numeric(unsigned m) {
    char *out = malloc(5);
    if (!out) return NULL;
    if (m & 07000u) sprintf(out, "%04o", m);
    else            sprintf(out, "%03o", m);
    return out;
}

char *mode_to_binary(unsigned m) {
    char *out = malloc(16);
    if (!out) return NULL;
    int p = 0;
    for (int g = 3; g >= 0; g--) {
        for (int b = 2; b >= 0; b--)
            out[p++] = ((m >> (g * 3 + b)) & 1) ? '1' : '0';
        if (g) out[p++] = ' ';
    }
    out[p] = '\0';
    return out;
}