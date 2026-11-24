#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stddef.h>
#include <stdint.h>

int write_all(int fd, const void* buf, size_t len);
int read_all(int fd, void* buf, size_t len);
int send_msg(int sock, uint8_t type, const char* name, const void* buf, uint32_t size);
int send_file(int sock, const char* sender, const char* path);

#endif