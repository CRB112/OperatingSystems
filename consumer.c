#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "globals.h"


shared_data_t *shared_mem;

void* consumer(void* arg) {
    int consumed = 0;

    while (consumed < 10) {
        sem_wait(shared_mem -> not_empty);
        sem_wait(shared_mem -> mutex);

        int item = shared_mem -> buffer[shared_mem -> out];
        printf("Consumed %d at: %d\n", item, shared_mem -> out);
        shared_mem -> out = (shared_mem -> out + 1) % BUFFERSIZE;
        shared_mem -> count--;
        consumed++;

        sem_post(shared_mem -> not_full);
        sem_post(shared_mem -> mutex);

    }
    printf("Consumer Stopping");
    return NULL;
}

int main() {

    printf("A");

    int sM_des = shm_open("/PCProblem", O_CREAT | O_RDWR, 0666);

    shared_mem = mmap(NULL, sizeof(shared_data_t), PROT_READ | PROT_WRITE, MAP_SHARED, sM_des, 0);
    
    shared_mem -> mutex = sem_open("/mutex", 1);
    shared_mem -> not_full = sem_open("/not_full", BUFFERSIZE);
    shared_mem -> not_empty = sem_open("/not_empty", 0);

    pthread_t consumer_t;

    pthread_create(&consumer_t, NULL, consumer, NULL);
    pthread_join(consumer_t, NULL);

    sem_close(shared_mem->mutex);
    sem_close(shared_mem->not_full);
    sem_close(shared_mem->not_empty);
    shm_unlink("/PCProblem");

    return 0;
} 