#include "p_queue.h"
#include <stdlib.h>

void pq_init(PQueue *pq) {
    for (int i = 0; i < N_PRIO; ++i) {
        pq->q[i].head = NULL;
        pq->q[i].tail = NULL;
    }
}

int pq_push(PQueue *pq, int payload, int prio) {
    if (prio < 0 || prio >= N_PRIO) return 0;

    Node *n = malloc(sizeof *n);
    if (!n) return 0;

    n->payload = payload;
    n->prio = prio;
    n->next = NULL;

    Fifo *f = &pq->q[prio];
    if (f->tail) f->tail->next = n;
    else f->head = n;
    f->tail = n;
    return 1;
}

static int pop_from(Fifo *f, int *out) {
    Node *n = f->head;
    if (!n) return 0;
    *out = n->payload;
    f->head = n->next;
    if (!f->head) f->tail = NULL;
    free(n);
    return 1;
}

int pq_pop_first(PQueue *pq, int *out) {
    for (int p = 0; p < N_PRIO; ++p)
        if (pq->q[p].head) return pop_from(&pq->q[p], out);
    return 0;
}

int pq_pop_prio(PQueue *pq, int prio, int *out) {
    if (prio < 0 || prio >= N_PRIO) return 0;
    return pop_from(&pq->q[prio], out);
}

int pq_pop_at_least(PQueue *pq, int prio, int *out) {
    if (prio < 0 || prio > N_PRIO) return 0;
    int start = prio - 1;
    for (int p = start; p >= 0; --p)
        if (pq->q[p].head) return pop_from(&pq->q[p], out);
    return 0;
}