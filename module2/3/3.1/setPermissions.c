#include "setPermissions.h"
#include "perm_bits.h"

#include "Strings.h"

#define WHO_U 04700u
#define WHO_G 02070u
#define WHO_O 01007u
#define WHO_ALL 07777u

// Битовая маска. -1 — недопустимый символ
static int perm_to_bits(unsigned who, char c, unsigned old_mode, unsigned *bits) {
    unsigned r = 0;
    switch (c) {
        case 'r':
            if (who & WHO_U) r |= 0400;
            if (who & WHO_G) r |= 0040;
            if (who & WHO_O) r |= 0004;
            break;
        case 'w':
            if (who & WHO_U) r |= 0200;
            if (who & WHO_G) r |= 0020;
            if (who & WHO_O) r |= 0002;
            break;
        case 'x':
            if (who & WHO_U) r |= 0100;
            if (who & WHO_G) r |= 0010;
            if (who & WHO_O) r |= 0001;
            break;
        case 'X':
            if (old_mode & 0111) {
                if (who & WHO_U) r |= 0100;
                if (who & WHO_G) r |= 0010;
                if (who & WHO_O) r |= 0001;
            }
            break;
        case 's':
            if (who & WHO_U) r |= B_SETUID;
            if (who & WHO_G) r |= B_SETGID;
            break;
        case 't':
            if (who & WHO_O) r |= B_STICKY;
            break;
        default:
            return -1;
    }
    *bits = r;
    return 0;
}

// Применение символьных модификаций прав
static int apply_clause(unsigned old, const char *s, int len) {
    unsigned who = 0;
    int has_who = 0;
    int i = 0;

    while (i < len) {
        char c = s[i];
        unsigned w;
        if (c == 'u') w = WHO_U;
        else if (c == 'g') w = WHO_G;
        else if (c == 'o') w = WHO_O;
        else if (c == 'a') w = WHO_ALL;
        else break;
        who |= w;
        has_who = 1;
        i++;
    }
    if (!has_who) who = WHO_ALL;

    if (i >= len) return PARSE_ERROR;
    char op = s[i++];
    if (op != '+' && op != '-' && op != '=') return PARSE_ERROR;

    unsigned add = 0;
    int nperm = 0;
    while (i < len) {
        char c = s[i++];
        unsigned bits;
        if (c == 'u' || c == 'g' || c == 'o') {
            unsigned src = (c == 'u') ? (old >> 6) & 7
                         : (c == 'g') ? (old >> 3) & 7
                                      :  old & 7;
            if (who & WHO_U) add |= (src & 7) << 6;
            if (who & WHO_G) add |= (src & 7) << 3;
            if (who & WHO_O) add |=  src & 7;
        } else {
            if (perm_to_bits(who, c, old, &bits) < 0) return PARSE_ERROR;
            add |= bits;
        }
        nperm++;
    }
    if (nperm == 0 && op != '=') return PARSE_ERROR;

    unsigned m = old;
    if (op == '=') { m &= ~who; m |= add; }
    else if (op == '+') { m |= add; }
    else { m &= ~add; }
    return (int)m;
}

// Применение прав
int apply_chmod(unsigned old_mode, const char *spec) {
    if (!spec || !*spec) return PARSE_ERROR;

    // Числовой режим
    if (spec[0] >= '0' && spec[0] <= '7') {
        size_t n = my_strlen(spec);
        if (n < 1 || n > 4) return PARSE_ERROR;
        unsigned m = 0;
        for (size_t i = 0; i < n; i++) {
            if (spec[i] < '0' || spec[i] > '7') return PARSE_ERROR;
            m = (m << 3) | (unsigned)(spec[i] - '0');
        }
        return (int)m;
    }

    // Символьный режим
    unsigned cur = old_mode & 07777;
    const char *p = spec;
    while (1) {
        const char *comma = my_strchr(p, ',');
        int len = comma ? (int)(comma - p) : (int)my_strlen(p);
        if (len == 0) return PARSE_ERROR;
        int r = apply_clause(cur, p, len);
        if (r < 0) return PARSE_ERROR;
        cur = (unsigned)r;
        if (!comma) break;
        p = comma + 1;
    }
    return (int)cur;
}