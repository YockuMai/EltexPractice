#ifndef IPV4_WORK_H
#define IPV4_WORK_H

#include <stdint.h>

int      parse_ipv4(const char *s, uint32_t *out);
void     print_ipv4(uint32_t ip);
uint32_t rand_u32(void);

#endif