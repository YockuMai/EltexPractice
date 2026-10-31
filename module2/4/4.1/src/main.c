#include "Interface.h"

int main() {
    PhoneBook pb;
    pb_init(&pb);
    menu_run(&pb);
    pb_free(&pb);
    return 0;
}