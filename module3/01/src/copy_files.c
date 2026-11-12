#include "copy_files.h"

int parent_proc(int readFromChild, int writeToChild, int n, char** files) {
    
    ssize_t r;
    char ready;
    do {r = read(readFromChild, &ready, 1); } while (r < 0 && errno == EINTR);

    for (int i = 0; i < n; i++) {
        int file = open(files[i], O_RDONLY);
        if (file < 0) { perror(files[i]); continue; }

        off_t size = lseek(file, 0, SEEK_END);
        if (size < 0 || lseek(file, 0, SEEK_SET) < 0) { perror("lseek"); close(file); continue; }

        long name_len = 0;
        while (files[i][name_len] != '\0') name_len++;
        
        if (write(writeToChild, &name_len, sizeof(name_len)) != (ssize_t)sizeof(name_len) ||
            write(writeToChild, files[i], name_len) != (ssize_t)name_len ||
            write(writeToChild, &size, sizeof(size)) != (ssize_t)sizeof(size))
        {
            close(file); return 1;
        }

        char buf[BLOCK];
        long left = (long)size;
        while (left > 0) {
            size_t want = left < BLOCK ? (size_t)left : BLOCK;
            ssize_t r = read(file, buf, want);
            if (r < 0) { if (errno == EINTR) continue; perror("read"); close(file); return 1; }
            if (r == 0) break;

            ssize_t w = 0;
            while (w < r) {
                ssize_t k = write(writeToChild, buf + w, (size_t)(r - w));
                if (k < 0) { if (errno == EINTR) continue; close(file); return 1; }
                w += k;
            }

            left -= r;
        }

        close(file);
        printf("Родитель отправил файл %s (%ld байт)\n", files[i], (long)size);
    }

    long end = -1;
    write(writeToChild, &end, sizeof(end));
    return 0;
}

int child_proc(int readFromParent, int writeToParent) {
    char ready = 'R';
    if (write(writeToParent, &ready, 1) != 1) return 1;

    while (1) {
        long name_len;
        if (read(readFromParent, &name_len, sizeof(name_len)) != (ssize_t)sizeof name_len) return 1;
        if (name_len < 0) return 0;

        char name[NAMELEN];
        ssize_t got = 0;
        while (got < name_len) {
            ssize_t r = read(readFromParent, name + got, (size_t)(name_len - got));
            if (r < 0) { if (errno == EINTR) continue; return 1; }
            if (r == 0) return 1;
            got += r;
        }
        name[name_len] = '\0';

        off_t size;
        if (read(readFromParent, &size, sizeof(size)) != (ssize_t)sizeof(size)) return 1;

        char out[NAMELEN + 8];
        snprintf(out, sizeof(out), "%s.copy", name);

        int outputFile = open(out, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (outputFile < 0) {perror("open"); return 1;}

        char buf[BLOCK];
        long left = (long)size;

        while(left > 0) {
            size_t want = left < BLOCK ? (size_t)left : BLOCK;
            ssize_t r = read(readFromParent, buf, want);
            if (r < 0) { if (errno == EINTR) continue; close(outputFile); return 1; }
            if (r == 0) break;

            ssize_t w = 0;
            while (w < r) {
                ssize_t k = write(outputFile, buf + w, (size_t)(r - w));
                if (k < 0) { if (errno == EINTR) continue; close(outputFile); return 1; }
                w += k;
            }
            left -= r;
        }
        close(outputFile);
        printf("Ребёнок получил %s (%ld байт)\n", name, (long)size);
    }
}