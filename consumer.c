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
        
        printf("consumer waiting on not empty");
        sem_wait(shared_mem -> mutex);
        printf("consumer waiting on not full");

        int item = shared_mem -> buffer[shared_mem -> out];
        printf("Consumed %d at: %d\n", item, shared_mem -> out);
        printf("consumed");
        shared_mem -> out = (shared_mem -> out + 1) % BUFFERSIZE;
        printf("ploob");
        shared_mem -> count--;
        printf("weewee");
        consumed++;

        printf("consumer hit mutex");
        sem_post(shared_mem -> mutex);
        printf("consumer hit not full");
        sem_post(shared_mem -> not_full);

    }
    printf("Consumer Stopping");
    return NULL;
}

int main() {

    sleep(1);
     fflush(stdout);
    printf("A");

    int sM_des = shm_open("/PCProblem", O_RDWR, 0666);
    if (sM_des == -1) {
        perror("shm_open failed in consumer");
        return 1;  // Exit on failure
    }

    shared_mem = mmap(NULL, sizeof(shared_data_t), PROT_READ | PROT_WRITE, MAP_SHARED, sM_des, 0);
    if (shared_mem == MAP_FAILED) {
        perror("mmap failed in consumer");
        close(sM_des);  // Clean up file descriptor
        return 1;  // Exit on failure
    }

    
    shared_mem -> mutex = sem_open("/mutex", 0);
    if (shared_mem->mutex == SEM_FAILED) {
        perror("sem_open for mutex failed in consumer");
        munmap(shared_mem, sizeof(shared_data_t));  // Clean up mmap
        close(sM_des);
        return 1;  // Exit on failure
    }
    shared_mem -> not_full = sem_open("/not_full", 0);
    if (shared_mem->not_full == SEM_FAILED) {
        perror("sem_open for not_full failed in consumer");
        sem_close(shared_mem->mutex);  // Clean up previous semaphore
        munmap(shared_mem, sizeof(shared_data_t));
        close(sM_des);
        return 1;
    }
    shared_mem -> not_empty = sem_open("/not_empty", 0);
    if (shared_mem->not_empty == SEM_FAILED) {
        perror("sem_open for not_empty failed in consumer");
        sem_close(shared_mem->mutex);
        sem_close(shared_mem->not_full);
        munmap(shared_mem, sizeof(shared_data_t));
        close(sM_des);
        return 1;
    }

    pthread_t consumer_t;

    pthread_create(&consumer_t, NULL, consumer, NULL);
    pthread_join(consumer_t, NULL);

    sem_close(shared_mem->mutex);
    sem_close(shared_mem->not_full);
    sem_close(shared_mem->not_empty);
    close(sM_des);

    return 0;
} 