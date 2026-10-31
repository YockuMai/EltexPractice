#include "p_queue.h"
#include "ui.h"

int main(void) {
    PQueue pq;
    pq_init(&pq);
    ui_run(&pq);
    return 0;
}