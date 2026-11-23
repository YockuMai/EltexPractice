#ifndef COMMON_H
#define COMMON_H

#define PORT 50000
#define BROADCAST_ADDR "255.255.255.255"
#define BUF_SIZE 512
#define ID_SIZE 64
#define NAME_SIZE 64

typedef struct chat_ctx {
    int sock_fd;
    int running;
    char my_id[ID_SIZE];
    char my_name[NAME_SIZE];
} chat_ctx;

#endif