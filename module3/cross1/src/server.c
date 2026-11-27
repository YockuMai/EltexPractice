#include "protocol.h"
#include "server.h"

#define MAX_CLIENTS 64

#include <arpa/inet.h>
#include <stddef.h>
#include <stdio.h>
#include <errno.h>
#include <netinet/udp.h>
#include <netinet/ip.h>
#include <string.h>
#include <unistd.h>

typedef struct client_counter {
    struct sockaddr_in addr;
    int count;
} client_counter;

static client_counter clients[MAX_CLIENTS];
static int client_count = 0; 

static client_counter* get_client(const struct sockaddr_in* addr) {
    for (int i = 0; i < client_count; i++) {
        if (clients[i].addr.sin_addr.s_addr == addr->sin_addr.s_addr && clients[i].addr.sin_port == addr->sin_port)
            return &clients[i];
    }
    if (client_count >= MAX_CLIENTS) return NULL;
    clients[client_count].addr = *addr;
    clients[client_count].count = 0;
    return &clients[client_count++];
}

static void remove_client(const struct sockaddr_in* addr) {
    for (int i = 0; i < client_count; i++) {
        if (clients[i].addr.sin_addr.s_addr == addr->sin_addr.s_addr && clients[i].addr.sin_port == addr->sin_port) {
            for (int j = i; j < client_count - 1; j++)
                clients[j] = clients[j + 1];
            client_count--;
            return;
        }
    }
}

int run_server(unsigned short port) {
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (sock < 0) { perror("socket"); return 1; }

    unsigned char buffer[BUF_SIZE];
    struct sockaddr_in cli_addr;
    socklen_t cli_len = sizeof(cli_addr);

    printf("[Сервер] запущен, порт %u\n", port);

    while (1) {
        ssize_t n = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&cli_addr, &cli_len);
        if (n < 0) {
            if (errno == EINTR) continue;
            perror("recvfrom"); break;
        }
        if (n < (ssize_t)(sizeof(struct iphdr) + sizeof(struct udphdr))) continue;

        struct iphdr *ip = (struct iphdr *)buffer;
        if (ip->protocol != IPPROTO_UDP) continue;

        struct udphdr *udp = (struct udphdr *)(buffer + ip->ihl * 4);
        if (ntohs(udp->dest) != port) continue;

        int payload_len = ntohs(udp->len) - sizeof(struct udphdr);
        if (payload_len <= 0) continue;

        unsigned char *payload = buffer + ip->ihl * 4 + sizeof(struct udphdr);

        struct sockaddr_in target = {0};
        target.sin_family = AF_INET;
        target.sin_addr.s_addr = ip->saddr;
        target.sin_port = udp->source;

        if (payload_len == 5 && memcmp(payload, "CLOSE", 5) == 0) {
            remove_client(&target);
            printf("[Сервер] %s:%u отключился, клиентов: %d\n", inet_ntoa(target.sin_addr), ntohs(target.sin_port), client_count);
            continue;
        }

        struct client_counter *cc = get_client(&target);
        if (!cc) { fprintf(stderr, "[Сервер] лимит клиентов\n"); continue; }
        cc->count++;

        char reply[BUF_SIZE];
        int reply_len = snprintf(reply, sizeof(reply), "%.*s %d",
                                 payload_len, payload, cc->count);
        if (reply_len <= 0 || reply_len >= (int)sizeof(reply)) continue;

        send_udp(sock, reply, target, port);
    }

    close(sock);
    return 0;
}