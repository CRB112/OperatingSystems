#ifndef GLOBALS_H
#define GLOBALS_H

#include <semaphore.h>

#define BUFFERSIZE 2

typedef struct {
    int buffer[BUFFERSIZE];
    int in;
    int out;
    int count;

    sem_t *mutex;
    sem_t *not_full;
    sem_t *not_empty;
} shared_data_t;

#endif
