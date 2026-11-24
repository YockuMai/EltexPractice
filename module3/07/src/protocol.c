#include "common.h"
#include "protocol.h"

#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>

int write_all(int fd, const void* buf, size_t len) {
    const char* p = (const char*)buf;
    while (len) {
        int n = write(fd, p, len);
        if (n <= 0) return -1;
        p+=n;
        len -= n;
    }
    return 0;
}

int read_all(int fd, void* buf, size_t len) {
    char* p = (char*)buf;

    while (len) {
        int n = read(fd, p, len);
        if (n <= 0) return -1;
        p+=n;
        len -= n;
    }
    return 0;
}

int send_msg(int sock, uint8_t type, const char* name, const void* buf, uint32_t size) {
    msg_hdr h = {0};
    h.type = type;
    strncpy(h.name, name, NAME_SIZE - 1);
    h.size = htonl(size);

    if (write_all(sock, &h, HDR_SIZE) < 0) return -1; // Отправка заголовка
    if (size > 0 && write_all(sock, buf, size) < 0) return -1; // Отправка содержимого сообщения
    return 0;
}

int send_file(int sock, const char* sender, const char* path) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) { perror("open"); return -1; }

    const char* base = strrchr(path, '/');
    base = base ? base + 1 : path;

    // Отправка имени файла
    if (send_msg(sock, 'F', sender, base, (uint32_t)strlen(base)) < 0) {
        close(fd); return -1;
    }

    // Отправка содержимого файла по частям
    char chunk[CHUNK_SIZE];
    int n;
    while ((n = read(fd, chunk, sizeof(chunk))) > 0) {
        if (send_msg(sock, 'D', sender, chunk, n) < 0) {
            close(fd); return -1;
        }
    }
    close(fd);

    // Отправка завершена
    send_msg(sock, 'Z', sender, NULL, 0);
    return 0;
}