#include "common.h"
#include "protocol.h"
#include "server.h"

#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <poll.h>
#include <string.h>

typedef struct client{
    int fd; // Сокет клиента
    char inbuf[BUF_MAX]; // Буфер принятых байт
    size_t inlen; // Сколько бай реально лежит в inbuf
} client;

static client g_clients[MAX_CLIENTS]; // Список клиентов
static int g_n = 0;

static void drop_client(int i) {
    close(g_clients[i].fd);
    g_clients[i] = g_clients[g_n -1];
    g_n--;
}

static void relay(int except, const char* buf, size_t len) { // Рассылка всем пользователям
    for (int i = 0; i < g_n; i++) {
        if (i == except) continue;
        write_all(g_clients[i].fd, buf, len);
    }
}

int run_server(int port) {
    int lfd = socket(AF_INET, SOCK_STREAM, 0); // Создание сокета
    if (lfd < 0) { perror("socket"); return 1; }

    int one = 1;
    setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)); // Позволяет повторно занять порт после перезапуска сервера

    struct sockaddr_in a = {0}; // Структура для IPv4
    a.sin_family = AF_INET;
    a.sin_port = htons(port);
    a.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(lfd, (struct sockaddr*)&a, sizeof(a)) < 0) { perror("bind"); return 1;} // Присоединение сокета к адресу
    if (listen(lfd, 16) < 0) { perror("listen"); return 1;}

    printf("Сервер запущен на порту %d\n", port);
    fflush(stdout);

    // Структура дял мультиплексирования
    struct pollfd pfds[MAX_CLIENTS + 1];

    while (1) {
        // Заполнение структуры
        pfds[0].fd = lfd; pfds[0].events = POLLIN; pfds[0].revents = 0;
        for (int i = 0; i < g_n; i++) {
            pfds[i + 1].fd = g_clients[i].fd;
            pfds[i + 1].events = POLLIN;
            pfds[i + 1].revents = 0;
        }

        int r = poll(pfds, g_n + 1, -1); // Ожидание изменнения дескрипторов
        if (r < 0) { if (errno == EINTR) continue; perror("poll"); break; }

        // Подключился новый клиент
        if (pfds[0].revents & POLLIN) {
            int cfd = accept(lfd, NULL, NULL); // Извлечение клиента из очереди
            if (cfd >= 0 && g_n < MAX_CLIENTS) {
                g_clients[g_n].fd = cfd;
                g_clients[g_n].inlen = 0;
                g_n++;
                printf("Клиент подключился (%d всего)\n", g_n);
                fflush(stdout);
            } else if (cfd >= 0) {
                close(cfd);
            }
        }

        // Проход по всем клиентам и проверка произошло ли событие
        for (int i = g_n - 1; i >= 0; i--) {
            if (!(pfds[i + 1].revents & (POLLIN | POLLHUP | POLLERR))) continue;
            struct client* c = &g_clients[i];

            // Чтение сообщения от клиента
            ssize_t n = recv(c->fd, c->inbuf + c->inlen, sizeof(c->inbuf) - c->inlen, 0);
            if (n <= 0) {
                printf("Клиент отключился (%d всего)\n", g_n - 1);
                fflush(stdout);
                drop_client(i);
                continue;
            }
            c->inlen += (size_t)n;

            size_t off = 0;
            int bad = 0;
            // Рассылка части сообщения всем подключённым клиентам
            while (c->inlen - off >= (size_t)HDR_SIZE) {
                msg_hdr h;
                memcpy(&h, c->inbuf + off, HDR_SIZE);
                uint32_t sz = ntohl(h.size); // Превод из сетевого порядка в хост
                if (sz > CHUNK_SIZE) { bad = 1; break; }
                if (c->inlen - off < (size_t)(HDR_SIZE + sz)) break;
                relay(i, c->inbuf + off, HDR_SIZE + sz);
                off += HDR_SIZE + sz;
            }
            if (bad) { drop_client(i); continue; }
            if (off > 0) {
                memmove(c->inbuf, c->inbuf + off, c->inlen - off);
                c->inlen -= off;
            }
        }
    }
    return 0;
}