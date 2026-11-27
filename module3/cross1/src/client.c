#include "protocol.h"
#include "client.h"

#include <signal.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netinet/udp.h>
#include <netinet/ip.h>

static volatile sig_atomic_t running = 1;
static void sigint_handler(int sig) { (void)sig; running = 0; }

static int wait_reply(int sock, struct in_addr server_ip,
                      unsigned short server_port, unsigned short local_port,
                      char *out, int out_size) {
    while (running) {
        unsigned char buf[BUF_SIZE];
        ssize_t n = recvfrom(sock, buf, sizeof(buf), 0, NULL, NULL);
        if (n < 0) {
            if (errno == EINTR) continue;
            perror("recvfrom"); return -1;
        }
        if (n < (ssize_t)(sizeof(struct iphdr) + sizeof(struct udphdr))) continue;

        struct iphdr *ip = (struct iphdr *)buf;
        if (ip->protocol != IPPROTO_UDP) continue;
        if (ip->saddr != server_ip.s_addr) continue;

        struct udphdr *udp = (struct udphdr *)(buf + ip->ihl * 4);
        if (ntohs(udp->source) != server_port) continue;
        if (ntohs(udp->dest) != local_port) continue;

        int plen = ntohs(udp->len) - sizeof(struct udphdr);
        if (plen <= 0 || plen >= out_size) continue;

        unsigned char *payload = buf + ip->ihl * 4 + sizeof(struct udphdr);
        memcpy(out, payload, plen);
        out[plen] = '\0';
        return plen;
    }
    return -1;
}

int run_client(const char* server_ip, unsigned short server_port, unsigned short local_port) {
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (sock < 0) { perror("socket"); return 1; }

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) < 0) {
        perror("sigaction");
        close(sock);
        return 1;
    }

    struct sockaddr_in srv = {0};
    srv.sin_family = AF_INET;
    srv.sin_port = htons(server_port);
    if (!inet_aton(server_ip, &srv.sin_addr)) {
        fprintf(stderr, "плохой IP: %s\n", server_ip);
        close(sock);
        return 1;
    }

    printf("[Клиент] локальный порт %u -> %s:%u (Ctrl+C — выход)\n",
           local_port, server_ip, server_port);

    char line[BUF_SIZE];
    while (running) {
        printf("> ");
        fflush(stdout);
        if (!fgets(line, sizeof(line), stdin)) break;
        line[strcspn(line, "\n")] = '\0';
        if (!line[0]) continue;

        if (send_udp(sock, line, srv, local_port) < 0) {
            perror("sendto");
            continue;
        }

        char reply[BUF_SIZE];
        int rlen = wait_reply(sock, srv.sin_addr, server_port, local_port,
                              reply, sizeof(reply));
        if (rlen > 0) {
            printf("Ответ: %s\n", reply);
        } else if (!running) {
            break;
        }
    }

    printf("\n[Клиент] отправка CLOSE...\n");
    if (send_udp(sock, "CLOSE", srv, local_port) < 0)
        perror("sendto CLOSE");

    close(sock);
    return 0;
}