#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <arpa/inet.h>

#define BUF_SIZE 4096

int send_udp(int sock, const char* msg, struct sockaddr_in target, unsigned short src_port);

#endif