#include "protocol.h"

#include <netinet/udp.h>
#include <string.h>

int send_udp(int sock, const char* msg, struct sockaddr_in target, unsigned short src_port) {
    int msg_len = strlen(msg);
    if (msg_len <= 0 || msg_len >= BUF_SIZE) return -1;

    unsigned char packet[sizeof(struct udphdr) + BUF_SIZE];
    struct udphdr *udp = (struct udphdr*)packet;
    unsigned char* payload = packet + sizeof(struct udphdr);

    udp->source = htons(src_port);
    udp->dest = target.sin_port;
    udp->len = htons(sizeof(struct udphdr) + msg_len);
    udp->check = 0;

    memcpy(payload, msg, msg_len);

    return sendto(sock, packet, sizeof(struct udphdr) + msg_len, 0, (struct sockaddr*) &target, sizeof(target));
}