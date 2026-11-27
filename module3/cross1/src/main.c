#include "server.h"
#include "client.h"

#include <stdio.h>
#include <bits/getopt_core.h>
#include <stdlib.h>

static void usage(const char *prog) {
    fprintf(stderr,
        "Использование:\n"
        "  %s -s [-p server_port]                             — сервер\n"
        "  %s -c server_ip [-p server_port] [-l local_port]   — клиент\n",
        prog, prog);
}

int main(int argc, char **argv) {
    int is_server = -1;
    const char *server_ip = "127.0.0.1";
    unsigned short server_port = 5555;
    unsigned short local_port = 4444;

    int opt;
    while ((opt = getopt(argc, argv, "scp:l:h")) != -1) {
        switch (opt) {
        case 's': is_server = 1; break;
        case 'c':
            is_server = 0;
            if (optind < argc) server_ip = argv[optind++];
            else { fprintf(stderr, "-c требует IP сервера\n"); return 1; }
            break;
        case 'p': server_port = (unsigned short)atoi(optarg); break;
        case 'l': local_port  = (unsigned short)atoi(optarg); break;
        case 'h':
        default: usage(argv[0]); return 1;
        }
    }

    if (is_server == -1) { usage(argv[0]); return 1; }

    return is_server ? run_server(server_port)
                     : run_client(server_ip, server_port, local_port);
}