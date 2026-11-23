#ifndef CHAT_H
#define CHAT_H

#include <stddef.h>

int send_msg(int sock_fd, char type, const char* text,
             const char* my_id, const char* my_name);

int read_msg(int sock_fd,
             char* id, size_t id_sz,
             char* name, size_t name_sz,
             char* type,
             char* text, size_t text_sz);

#endif