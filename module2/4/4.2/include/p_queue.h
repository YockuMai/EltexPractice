#ifndef PQUEUE_H
#define PQUEUE_H
#define N_PRIO 256

typedef struct Node {
    int payload;
    int prio;
    struct Node *next;
} Node;

typedef struct {
    Node *head, *tail;
} Fifo;

typedef struct {
    Fifo q[N_PRIO]; // 0 — самый высокий приоритет
} PQueue;

void pq_init(PQueue*);
int pq_push(PQueue*, int, int);
int pq_pop_first(PQueue*, int*);
int pq_pop_prio(PQueue*, int, int*);
int pq_pop_at_least(PQueue*, int, int*);

#endif