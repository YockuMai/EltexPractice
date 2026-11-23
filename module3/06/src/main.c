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

static struct chat_ctx* g_ctx = NULL;

static void on_sigint(int sig) {
    (void)sig;
    if (g_ctx) {
        g_ctx->running = 0;
        // прощальный broadcast
        send_msg(g_ctx->sock_fd, 'L', "",
                 g_ctx->my_id, g_ctx->my_name);
        close(g_ctx->sock_fd);
    }
    _exit(0);
}

int main(int argc, char** argv) {
    if (argc < 2) { fprintf(stderr, "Использование: %s Имя\n", argv[0]); return 1; }

    struct chat_ctx ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.running = 1;
    strncpy(ctx.my_name, argv[1], NAME_SIZE - 1);

    srand((unsigned)time(NULL) ^ (unsigned)getpid());
    snprintf(ctx.my_id, ID_SIZE, "%d-%d", getpid(), rand());

    ctx.sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (ctx.sock_fd < 0) { perror("socket"); return 1; }

    int one = 1;
    if (setsockopt(ctx.sock_fd, SOL_SOCKET, SO_REUSEADDR,
                   &one, sizeof(one)) < 0) {
        perror("SO_REUSEADDR");
        return 1;
    }
    if (setsockopt(ctx.sock_fd, SOL_SOCKET, SO_BROADCAST,
                   &one, sizeof(one)) < 0) {
        perror("SO_BROADCAST");
        return 1;
    }

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(ctx.sock_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return 1;
    }

    g_ctx = &ctx;
    struct sigaction sa = {0};
    sa.sa_handler = on_sigint;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    if (send_msg(ctx.sock_fd, 'J', "",
                 ctx.my_id, ctx.my_name) < 0) {
        fprintf(stderr, "Внимание: Не удалось отправить приветствие\n");
    }

    pthread_t th;
    if (pthread_create(&th, NULL, recv_thread, &ctx) != 0) {
        perror("pthread_create");
        return 1;
    }

    printf("Чат начат под именем %s (id=%s). Ctrl+C для выхода.\n> ",
           ctx.my_name, ctx.my_id);
    fflush(stdout);

    char line[BUF_SIZE];
    while (ctx.running && fgets(line, sizeof line, stdin)) {
        size_t len = strlen(line);
        if (len && line[len - 1] == '\n') line[--len] = '\0';
        if (len == 0) { printf("> "); fflush(stdout); continue; }

        if (send_msg(ctx.sock_fd, 'M', line,
                     ctx.my_id, ctx.my_name) < 0) {
            fprintf(stderr, "Ошибка оптравки сообщения\n");
        }
        printf("> ");
        fflush(stdout);
    }

    ctx.running = 0;
    send_msg(ctx.sock_fd, 'L', "", ctx.my_id, ctx.my_name);
    close(ctx.sock_fd);
    pthread_join(th, NULL);
    return 0;
}