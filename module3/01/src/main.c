#include "copy_files.h"
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define PATHBUF 4096

int main(int argc, char **argv) {
    const char *fifo_base = NULL;
    int argi = 1;

    if (argi < argc && argv[argi][0] == '-' && argv[argi][1] == 'p' && argv[argi][2] == '\0') {
        if (argi + 1 >= argc) {
            fprintf(stderr, "Использование: %s [-p имя_канала] файл...\n", argv[0]);
            return 1;
        }
        fifo_base = argv[argi + 1];
        argi += 2;
    }
    if (argi >= argc) {
        fprintf(stderr, "Использование: %s [-p имя_канала] файл...\n", argv[0]);
        return 1;
    }

    int use_fifo = (fifo_base != NULL);
    int c2p[2] = {-1, -1}, p2c[2] = {-1, -1};
    char c2p_path[PATHBUF], p2c_path[PATHBUF];

    if (use_fifo) {
        snprintf(c2p_path, sizeof c2p_path, "%s.c2p", fifo_base);
        snprintf(p2c_path, sizeof p2c_path, "%s.p2c", fifo_base);
        unlink(c2p_path);
        unlink(p2c_path);
        if (mkfifo(c2p_path, 0600) < 0) { perror("mkfifo c2p"); return 1; }
        if (mkfifo(p2c_path, 0600) < 0) { perror("mkfifo p2c"); unlink(c2p_path); return 1; }
    } else {
        if (pipe(c2p) < 0) { perror("pipe c2p"); return 1; }
        if (pipe(p2c) < 0) { perror("pipe p2c"); return 1; }
    }

    pid_t pid = fork();
    if (pid < 0) { perror("fork"); return 1; }

    // Дочерний процесс
    if (pid == 0) {
        int readFromParent, writeToParent;

        if (use_fifo) {
            writeToParent = open(c2p_path, O_WRONLY);
            if (writeToParent < 0) { perror("child open c2p"); _exit(1); }
            readFromParent = open(p2c_path, O_RDONLY);
            if (readFromParent < 0) { perror("child open p2c"); close(writeToParent); _exit(1); }
        } else {
            close(c2p[0]);
            close(p2c[1]);
            readFromParent = p2c[0];
            writeToParent  = c2p[1];
        }

        int result = child_proc(readFromParent, writeToParent);

        close(readFromParent);
        close(writeToParent);
        _exit(result);
    }

    int readFromChild, writeToChild;

    if (use_fifo) {
        readFromChild = open(c2p_path, O_RDONLY);
        if (readFromChild < 0) {
            perror("parent open c2p");
            unlink(c2p_path); unlink(p2c_path);
            return 1;
        }
        writeToChild = open(p2c_path, O_WRONLY);
        if (writeToChild < 0) {
            perror("parent open p2c");
            close(readFromChild);
            unlink(c2p_path); unlink(p2c_path);
            return 1;
        }
    } else {
        close(p2c[0]);
        close(c2p[1]);
        readFromChild = c2p[0];
        writeToChild  = p2c[1];
    }

    int result = parent_proc(readFromChild, writeToChild, argc - argi, argv + argi);

    close(readFromChild);
    close(writeToChild);

    int status;
    waitpid(pid, &status, 0);

    if (result == 0 && WIFEXITED(status)) {
        puts("ОК");
        return 0;
    }
    fprintf(stderr, "Ребёнок завершился с ошибкой\n");
    return 1;
}