#include "common.h"
#include "client.h"
#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void usage(const char* p) {
    fprintf(stderr,
        "Использование:\n"
        "  %s -s [-p PORT] run server\n"
        "  %s -c -n NAME [-h HOST] [-p PORT] run client\n", p, p);
}

int main(int argc, char** argv) {
    int mode = 0; // 0 = client, 1 = server
    const char* name = NULL;
    const char* host = "127.0.0.1";
    int port = PORT;

    int opt;
    while ((opt = getopt(argc, argv, "scn:h:p:")) != -1) {
        switch (opt) {
        case 's': mode = 1; break;
        case 'c': mode = 0; break;
        case 'n': name = optarg; break;
        case 'h': host = optarg; break;
        case 'p': port = atoi(optarg); break;
        default:  usage(argv[0]); return 1;
        }
    }

    if (mode == 1) return run_server(port);
    if (!name) { fprintf(stderr, "-c -n ИМЯ\n"); return 1; }
    return run_client(name, host, port);
}