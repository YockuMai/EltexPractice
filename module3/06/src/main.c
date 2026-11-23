#include "common.h"
#include "my_threads.h"
#include "chat.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <pthread.h>

// Глобальная структура, чтобы обработчик сигнала видел её
static struct chat_ctx* g_ctx = NULL;

// Функция для обработки сигнала
static void on_sigint(int sig) {
    (void)sig;
    if (g_ctx) {
        g_ctx->running = 0;
        // прощальный broadcast
        send_msg(g_ctx->sock_fd, 'L', "", g_ctx->my_id, g_ctx->my_name, g_ctx->bcast_ip);
        close(g_ctx->sock_fd);
    }
    _exit(0);
}

int main(int argc, char** argv) {
    if (argc < 2) { 
        fprintf(stderr, "Использование: %s <Имя> [broadcast_ip]\n", argv[0]); 
        return 1; 
    }

    struct chat_ctx ctx; // Создание управляющей структуры
    memset(&ctx, 0, sizeof(ctx));
    ctx.running = 1;
    strncpy(ctx.my_name, argv[1], NAME_SIZE - 1);

    ctx.bcast_ip = (argc >= 3) ? argv[2] : BROADCAST_ADDR; // Запись широковещательного IPv4

    srand((unsigned)time(NULL) ^ (unsigned)getpid());
    snprintf(ctx.my_id, ID_SIZE, "%d-%d", getpid(), rand()); // Формирование уникального идентификатора

    ctx.sock_fd = socket(AF_INET, SOCK_DGRAM, 0); // Создание UDP сокета
    if (ctx.sock_fd < 0) { perror("socket"); return 1; }

    int one = 1;
    if (setsockopt(ctx.sock_fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) < 0) { // Разрешает повторно привязать сокет
        perror("SO_REUSEADDR");
        return 1;
    }
    if (setsockopt(ctx.sock_fd, SOL_SOCKET, SO_BROADCAST, &one, sizeof(one)) < 0) { // Разрешает сокету отправлять широковещательные пакеты
        perror("SO_BROADCAST");
        return 1;
    }

    struct sockaddr_in addr = {0}; // Структура дял привязки сокета к адресу и порту
    addr.sin_family = AF_INET; // Запись семейства адресов
    addr.sin_port = htons(PORT); // Перевод порта из порядка хоста в сетевой
    addr.sin_addr.s_addr = htonl(INADDR_ANY); // Перевод IPv4 из порядка хоста в сетевой
    if (bind(ctx.sock_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) { // Привязка сокета к IP адресу и порту
        perror("bind");
        return 1;
    }

    g_ctx = &ctx;
    struct sigaction sa = {0}; // Действия на сигнал завершения
    sa.sa_handler = on_sigint;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    if (send_msg(ctx.sock_fd, 'J', "", ctx.my_id, ctx.my_name, ctx.bcast_ip) < 0) { // Отправка сообщения о подключении
        fprintf(stderr, "Внимание: Не удалось отправить приветствие\n");
    }

    pthread_t th;
    if (pthread_create(&th, NULL, recv_thread, &ctx) != 0) { // Поток для чтения сообщений
        perror("pthread_create");
        return 1;
    }

    printf("Чат начат под именем %s (id=%s). Ctrl+C для выхода.\n> ",
           ctx.my_name, ctx.my_id);
    fflush(stdout);

    char line[BUF_SIZE];
    while (ctx.running && fgets(line, sizeof line, stdin)) { // Ввод сообщения с клавиатуры
        size_t len = strlen(line);
        if (len && line[len - 1] == '\n') line[--len] = '\0';
        if (len == 0) { printf("> "); fflush(stdout); continue; }

        if (send_msg(ctx.sock_fd, 'M', line, ctx.my_id, ctx.my_name, ctx.bcast_ip) < 0) { // Отправка сообщения
            fprintf(stderr, "Ошибка оптравки сообщения\n");
        }
        printf("> ");
        fflush(stdout);
    }

    ctx.running = 0; // Программа завершила свою работу
    send_msg(ctx.sock_fd, 'L', "", ctx.my_id, ctx.my_name, ctx.bcast_ip); // Отправка сообщения об отключении
    close(ctx.sock_fd); // Закрытие сокета
    pthread_join(th, NULL); // Ожидание завершения потока
    return 0;
}