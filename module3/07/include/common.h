#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

#define PORT 50001
#define NAME_SIZE 64
#define MAX_CLIENTS 64
#define CHUNK_SIZE 4096

typedef struct __attribute__((packed)) msg_hdr
{
    uint8_t type; // J, L, M, F, D, Z
    char name[NAME_SIZE]; // F: sender|filename, иначе sender
    uint32_t size; 
} msg_hdr; // Отключение выравнивания полей, чтобы не было пустых байтов

#define HDR_SIZE ((int)sizeof(msg_hdr))
#define BUF_MAX (HDR_SIZE + CHUNK_SIZE)


#endif