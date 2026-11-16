#ifndef THREADS_H
#define THREADS_H

void threads_install_sigint(void);
void *sender_thread(void*);
void *receiver_thread(void*);

#endif