#include "queue.h"
#include <stdio.h>
#include <errno.h>

/* 
Функция пытается создать очередь сообщений с заданным именем, режимом доступа, максимальным количеством сообщений и размером сообщения
Если очередь с таким именем уже существует, она открывает существующую очередь
Возвращает 0, если очередь успешно создана, 1, если очередь уже существует, и -1 в случае ошибки 
*/
int create_queue(mqd_t *queue, const char *name, mode_t mode, long maxmsg, long msgsize) {
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = maxmsg;
    attr.mq_msgsize = msgsize;
    attr.mq_curmsgs = 0;

    mqd_t temp_queue = mq_open(name, O_CREAT | O_RDWR | O_EXCL, mode, &attr);
    if (temp_queue != (mqd_t)-1) {
        *queue = temp_queue;
        return 0; // Очередь успешно создана
    } else if (errno == EEXIST) {
        temp_queue = mq_open(name, O_RDWR);
        if (temp_queue != (mqd_t)-1) {
            *queue = temp_queue;
            return 1; // Очередь уже существует
        } else {
            perror("Ошибка при открытии существующей очереди сообщений");
            return -1;
        }
    } else {
        perror("Ошибка при создании очереди сообщений");
        return -1;
    }
}