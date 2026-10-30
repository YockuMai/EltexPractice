#include "ipv4_work.h"
#include <stdio.h>
#include <stdlib.h>

int parse_ipv4(const char* s, uint32_t* out) {
    unsigned a, b, c, d;
    char tail;
    if (sscanf(s, "%u.%u.%u.%u%c", &a, &b, &c, &d, &tail) != 4)
        return -1;
    if (a > 255 || b > 255 || c > 255 || d > 255)
        return -1;
    *out = ((uint32_t)a << 24) | ((uint32_t)b << 16)
         | ((uint32_t)c << 8)  |  (uint32_t)d;
    return 0;
}

void print_ipv4(const uint32_t ip) {
    printf("%u.%u.%u.%u",
           (ip >> 24) & 0xFF, (ip >> 16) & 0xFF,
           (ip >>  8) & 0xFF,  ip & 0xFF);
}

uint32_t rand_u32() {
    uint32_t r = 0;
    for (int i = 0; i < 4; i++)
        r = (r << 8) | ((uint32_t)rand() & 0xFFu);
    return r;
}