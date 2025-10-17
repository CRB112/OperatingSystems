#ifndef GLOBALS
#define GLOBALS

#include<semaphore.h>

#define BUFFERSIZE 2

void* producer(void* arg);
void* consumer(void* arg);

typedef struct {
    int buffer[BUFFERSIZE];
    int in;
    int out;
    int count;
    sem_t *mutex;
    sem_t *not_full;
    sem_t *not_empty;
} shared_data_t;

extern shared_data_t *shared_mem;

#endif