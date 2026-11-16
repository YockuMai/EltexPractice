#ifndef QUEUE_H
#define QUEUE_H

#define MAX_MSGS 10
#define MSG_SIZE 256
#define NORMAL_PRIORITY 1
#define EXIT_PRIORITY (NORMAL_PRIORITY + 1)

#include <mqueue.h>

int create_queue(mqd_t *queue, const char *name, mode_t mode, long maxmsg, long msgsize);

#endif