#include "common.h"
#include "protocol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>

static int  g_sock = -1;
static char g_name[NAME_SIZE];

static char g_inbuf[BUF_MAX];
static size_t g_inlen = 0;

struct incoming {
    int active;
    char sender[NAME_SIZE];
    FILE* fp;
    char fname[600];
};
static struct incoming g_recv[MAX_CLIENTS];

static void on_sigint(int sig) {
    (void)sig;
    if (g_sock >= 0) {
        send_msg(g_sock, 'L', g_name, NULL, 0);
        close(g_sock);
    }
    _exit(0);
}

static struct incoming* find_slot(const char* sender, int create) {
    for (int i = 0; i < MAX_CLIENTS; i++)
        if (g_recv[i].active &&
            strncmp(g_recv[i].sender, sender, NAME_SIZE) == 0)
            return &g_recv[i];
    if (!create) return NULL;
    for (int i = 0; i < MAX_CLIENTS; i++)
        if (!g_recv[i].active) return &g_recv[i];
    return NULL;
}

static void handle_frame(const struct msg_hdr* h, const char* payload, uint32_t sz) {
    char name[NAME_SIZE];
    memcpy(name, h->name, NAME_SIZE);
    name[NAME_SIZE - 1] = '\0';

    switch (h->type) {
    case 'J':
        printf("\n* %s присоединился\n> ", name);
        break;
    case 'L':
        printf("\n* %s отключился\n> ", name);
        break;
    case 'M':
        printf("\n[%s]: %.*s\n> ", name, (int)sz, payload);
        break;

    case 'F': {
        char* bar = strchr(name, '|');
        const char* sender = name;
        const char* fname  = bar ? bar + 1 : name;
        if (bar) *bar = '\0';

        struct incoming* s = find_slot(sender, 1);
        if (!s) { printf("\n[Слишком много входящих сообщений]\n> "); break; }
        if (s->active) { fclose(s->fp); s->active = 0; }

        snprintf(s->fname, sizeof s->fname, "recv_%s_%s", sender, fname);
        s->fp = fopen(s->fname, "wb");
        if (!s->fp) { perror("fopen"); break; }

        strncpy(s->sender, sender, NAME_SIZE - 1);
        s->sender[NAME_SIZE - 1] = '\0';
        s->active = 1;
        printf("\n[%s отправляется '%s']\n> ", sender, fname);
        break;
    }
    case 'D': {
        struct incoming* s = find_slot(name, 0);
        if (s && s->fp) fwrite(payload, 1, sz, s->fp);
        break;
    }
    case 'Z': {
        struct incoming* s = find_slot(name, 0);
        if (s && s->fp) {
            fclose(s->fp);
            s->active = 0;
            printf("\n[Сохранён: %s]\n> ", s->fname);
        }
        break;
    }
    }
    fflush(stdout);
}

static void send_input_line(int sock, char* line) {
    if (strncmp(line, "/file ", 6) == 0) {
        const char* path = line + 6;
        if (send_file(sock, g_name, path) < 0)
            printf("[Ошибка отправки '%s']\n> ", path);
        else
            printf("[Файл '%s' отправлен]\n> ", path);
        fflush(stdout);
        return;
    }
    send_msg(sock, 'M', g_name, line, (uint32_t)strlen(line));
    printf("> ");
    fflush(stdout);
}

int run_client(const char* name, const char* host, int port) {
    strncpy(g_name, name, NAME_SIZE - 1);
    if (strchr(g_name, '|')) {
        fprintf(stderr, "Имя не должно содержать '|'\n");
        return 1;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { perror("socket"); return 1; }

    struct sockaddr_in a = {0};
    a.sin_family = AF_INET;
    a.sin_port = htons((uint16_t)port);
    if (inet_pton(AF_INET, host, &a.sin_addr) != 1) { perror("inet_pton"); return 1; }
    if (connect(sock, (struct sockaddr*)&a, sizeof a) < 0) { perror("connect"); return 1; }
    g_sock = sock;

    struct sigaction sa = {0};
    sa.sa_handler = on_sigint;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    send_msg(sock, 'J', g_name, NULL, 0);
    printf("Подключён как %s. Напишите текст или /file <путь>. Ctrl+C для выхода.\n> ", g_name);
    fflush(stdout);

    while (1) {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(STDIN_FILENO, &rfds);
        FD_SET(sock, &rfds);
        int maxfd = sock > STDIN_FILENO ? sock : STDIN_FILENO;

        int r = select(maxfd + 1, &rfds, NULL, NULL, NULL);
        if (r < 0) { if (errno == EINTR) continue; perror("select"); break; }

        if (FD_ISSET(STDIN_FILENO, &rfds)) {
            char line[4096];
            if (!fgets(line, sizeof line, stdin)) break;
            size_t L = strlen(line);
            if (L && line[L - 1] == '\n') line[--L] = '\0';
            if (L == 0) { printf("> "); fflush(stdout); continue; }
            send_input_line(sock, line);
        }

        if (FD_ISSET(sock, &rfds)) {
            ssize_t n = recv(sock, g_inbuf + g_inlen, sizeof g_inbuf - g_inlen, 0);
            if (n <= 0) { printf("\n[Отключён от сервера]\n"); break; }
            g_inlen += (size_t)n;

            size_t off = 0;
            int bad = 0;
            while (g_inlen - off >= (size_t)HDR_SIZE) {
                struct msg_hdr h;
                memcpy(&h, g_inbuf + off, HDR_SIZE);
                uint32_t sz = ntohl(h.size);
                if (sz > CHUNK_SIZE) { bad = 1; break; }
                if (g_inlen - off < (size_t)(HDR_SIZE + sz)) break;
                handle_frame(&h, g_inbuf + off + HDR_SIZE, sz);
                off += HDR_SIZE + sz;
            }
            if (bad) { printf("\n[Ошибка протокола]\n"); break; }
            if (off > 0) {
                memmove(g_inbuf, g_inbuf + off, g_inlen - off);
                g_inlen -= off;
            }
        }
    }
    close(sock);
    return 0;
}