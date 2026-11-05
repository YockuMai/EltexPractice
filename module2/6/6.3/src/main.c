#include "Interface.h"

int main(int argc, char **argv) {
    const char *lib_dir = (argc > 1) ? argv[1] : "build/libs";
    menu_run(lib_dir);
    return 0;
}