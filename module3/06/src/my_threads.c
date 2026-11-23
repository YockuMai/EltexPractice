#include "my_threads.h"
#include "chat.h"
#include "common.h"

#include <stdio.h>
#include <string.h>

void* recv_thread(void* arg) {
    chat_ctx* ctx = (chat_ctx*)arg;

    char id[ID_SIZE], name[NAME_SIZE], type, text[BUF_SIZE];

    while (ctx->running) {
        if (read_msg(ctx->sock_fd,
                     id, sizeof(id),
                     name, sizeof(name),
                     &type,
                     text, sizeof(text)) != 0) {
            continue;
        }
        if (strcmp(id, ctx->my_id) == 0) continue;

        switch (type) {
            case 'J': printf("\n%s Подключился\n> ", name); break;
            case 'L': printf("\n %s Отключился\n> ", name); break;
            case 'M': printf("\n[%s]: %s\n> ", name, text); break;
        }
        fflush(stdout);
    }
    return NULL;
}