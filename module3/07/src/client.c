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

static int g_sock = -1; // Сокет подключения к серверу
static char g_name[NAME_SIZE]; // Имя клиента

static char g_inbuf[BUF_MAX]; // Буфер для приёма байтов от сервера
static size_t g_inlen = 0;

struct incoming { // Структура для отслеживания файлов клиентов
    int active;
    char sender[NAME_SIZE];
    FILE* fp;
    char fname[600];
};
static struct incoming g_recv[MAX_CLIENTS]; // Отслеживание файлов каждого клиента

static void on_sigint(int sig) { // Обработка сигнала
    (void)sig;
    if (g_sock >= 0) {
        send_msg(g_sock, 'L', g_name, NULL, 0);
        close(g_sock);
    }
    _exit(0);
}

// Ищет ячейку для приёма файла от конкретного отправителя
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

// Интерпретация типов сообщений
static void handle_frame(const struct msg_hdr* h, const char* payload, uint32_t sz) {
    char name[NAME_SIZE];
    memcpy(name, h->name, NAME_SIZE);
    name[NAME_SIZE - 1] = '\0';

    switch (h->type) {
    case 'J': // Подключение клиента
        printf("\n* %s присоединился\n> ", name);
        break;
    case 'L': // Отключение клиента
        printf("\n* %s отключился\n> ", name);
        break;
    case 'M': // Просто сообщение
        printf("\n[%s]: %.*s\n> ", name, (int)sz, payload);
        break;

    case 'F': { // Начало приёма файла
        // name = sender (из заголовка), payload = имя файла
        const char* sender = name;
        
        char fname[CHUNK_SIZE + 1];
        size_t flen = (sz < CHUNK_SIZE) ? sz : CHUNK_SIZE; // Определение больше ли файл заданного буфера
        memcpy(fname, payload, flen);
        fname[flen] = '\0';
        if (flen == 0) { printf("\n[пустое имя файла]\n> "); break; }
        
        struct incoming* s = find_slot(sender, 1); // Создание ячейки конкретного клиента
        if (!s) { printf("\n[Слишком много входящих сообщений]\n> "); break; }
        if (s->active) { fclose(s->fp); s->active = 0; }
        
        snprintf(s->fname, sizeof s->fname, "recv_%s_%s", sender, fname); // Формирование имени файла
        s->fp = fopen(s->fname, "wb"); // Открытие файла на запись
        if (!s->fp) { perror("fopen"); break; }
        
        strncpy(s->sender, sender, NAME_SIZE - 1);
        s->sender[NAME_SIZE - 1] = '\0';
        s->active = 1;
        printf("\n[%s отправляется '%s']\n> ", sender, fname);
        break;
    }
    case 'D': { // Часть файла
        struct incoming* s = find_slot(name, 0); // Находим ячейку клиента
        if (s && s->fp) fwrite(payload, 1, sz, s->fp); // Дописываем файл
        break;
    }
    case 'Z': { // Конец передачи файла
        struct incoming* s = find_slot(name, 0); // Находим ячейку клиента
        if (s && s->fp) {
            fclose(s->fp); // Закрываем файл
            s->active = 0; // Запись закончена
            printf("\n[Сохранён: %s]\n> ", s->fname);
        }
        break;
    }
    }
    fflush(stdout);
}

// Отправка сообщения или файла
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

    int sock = socket(AF_INET, SOCK_STREAM, 0); // Создание сокета
    if (sock < 0) { perror("socket"); return 1; }

    struct sockaddr_in a = {0}; // Структура для IPv4
    a.sin_family = AF_INET;
    a.sin_port = htons((uint16_t)port);
    // Перевод строкового IP в двоичное число
    if (inet_pton(AF_INET, host, &a.sin_addr) != 1) { perror("inet_pton"); return 1; }
    // Подключение к серверу
    if (connect(sock, (struct sockaddr*)&a, sizeof a) < 0) { perror("connect"); return 1; }
    g_sock = sock;

    struct sigaction sa = {0}; // Обработка сигнала
    sa.sa_handler = on_sigint;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    send_msg(sock, 'J', g_name, NULL, 0); // Отправка сообщения о подключении
    printf("Подключён как %s. Напишите текст или /file <путь>. Ctrl+C для выхода.\n> ", g_name);
    fflush(stdout);

    while (1) {
        fd_set rfds; // Структура для Select
        FD_ZERO(&rfds);
        FD_SET(STDIN_FILENO, &rfds);
        FD_SET(sock, &rfds);
        int maxfd = sock > STDIN_FILENO ? sock : STDIN_FILENO; // На случай если поток ввода будет открыт после сокета

        int r = select(maxfd + 1, &rfds, NULL, NULL, NULL); // Ожидание сообщений от сервера
        if (r < 0) { if (errno == EINTR) continue; perror("select"); break; }

        // Ввод пользовтеля с клавиатуры
        if (FD_ISSET(STDIN_FILENO, &rfds)) {
            char line[4096];
            if (!fgets(line, sizeof line, stdin)) break;
            size_t L = strlen(line);
            if (L && line[L - 1] == '\n') line[--L] = '\0';
            if (L == 0) { printf("> "); fflush(stdout); continue; }
            send_input_line(sock, line);
        }

        // Получение данных из сокета
        if (FD_ISSET(sock, &rfds)) {
            ssize_t n = recv(sock, g_inbuf + g_inlen, sizeof g_inbuf - g_inlen, 0);
            if (n <= 0) { printf("\n[Отключён от сервера]\n"); break; }
            g_inlen += (size_t)n;

            size_t off = 0;
            int bad = 0;
            while (g_inlen - off >= (size_t)HDR_SIZE) { // Чтение полученных байтов по частям
                struct msg_hdr h;
                memcpy(&h, g_inbuf + off, HDR_SIZE);
                uint32_t sz = ntohl(h.size); // Перевод из сетевого в хост
                if (sz > CHUNK_SIZE) { bad = 1; break; }
                if (g_inlen - off < (size_t)(HDR_SIZE + sz)) break;
                handle_frame(&h, g_inbuf + off + HDR_SIZE, sz); // Определение типа сообщения и действий на него
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