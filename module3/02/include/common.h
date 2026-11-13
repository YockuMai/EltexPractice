#ifndef COMMON_H
#define COMMON_H
#include <sys/types.h>

#define MSG_KEY ((key_t)0x1234)
#define MAX_TEXT 512
#define MAX_TOPIC 64
#define MAX_SUBS 256
#define MAX_PUBS 256

struct msgbuf {
    long mtype;
    char mtext[MAX_TEXT];
};

#endif