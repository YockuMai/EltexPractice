#include "chat.h"
#include "common.h"

#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

/* Формат сообщения:  <id>|<name>|<type>|<text>
    type = 'J' — подключился, 'L' — отключился, 'M' — обычное сообщение
*/
int send_msg(int sock_fd, char type, const char* text, const char* my_id, const char* my_name, const char* dest_addr) {
    char buf[BUF_SIZE];

    int n = snprintf(buf, sizeof(buf), "%s|%s|%c|%s",
                     my_id, my_name, type, text ? text : "");
    if (n < 0 || (size_t)n >= sizeof buf) {
        fprintf(stderr, "send_msg: сообщение слишком длинное\n");
        return -1;
    }

    struct sockaddr_in dest = {0};
    dest.sin_family = AF_INET;
    dest.sin_port = htons(PORT);
    if (inet_pton(AF_INET, dest_addr, &dest.sin_addr) != 1) {
        perror("inet_pton");
        return -1;
    }

    ssize_t sent = sendto(sock_fd, buf, (size_t)n, 0,
                          (struct sockaddr*)&dest, sizeof(dest));
    if (sent < 0) {
        perror("sendto");
        return -1;
    }
    return 0;
}

int read_msg(int sock_fd,
             char* id, size_t id_sz,
             char* name, size_t name_sz,
             char* type,
             char* text, size_t text_sz) {
    char buf[BUF_SIZE];
    struct sockaddr_in sender;
    socklen_t slen = sizeof sender;

    ssize_t n = recvfrom(sock_fd, buf, sizeof buf - 1, 0,
                         (struct sockaddr*)&sender, &slen);
    if (n < 0) {
        perror("recvfrom");
        return -1;
    }
    buf[n] = '\0';

    /* парсим: id|name|type|text */
    char* p_id = buf;
    char* p_name = strchr(p_id, '|'); if (!p_name) return -1; *p_name++ = '\0';
    char* p_type = strchr(p_name, '|'); if (!p_type) return -1; *p_type++ = '\0';
    char* p_text = strchr(p_type, '|'); if (!p_text) return -1; *p_text++ = '\0';

    /* проверка длин, чтобы не переполнить буферы вызывающего */
    if (strlen(p_id) >= id_sz) return -1;
    if (strlen(p_name) >= name_sz) return -1;
    if (strlen(p_text) >= text_sz) return -1;

    strcpy(id, p_id);
    strcpy(name, p_name);
    strcpy(text, p_text);
    *type = *p_type;
    return 0;
}