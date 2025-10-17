#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "globals.h"

shared_data_t *shared_mem;

void* producer(void* arg) {
    int countProd = 0;

    while (countProd < 10) {
        int item = (rand() % 10) + 1;

        sem_wait(shared_mem -> mutex);
        printf("producer wiaitng on not full");
        sem_wait(shared_mem -> not_full); 
        printf("producer wiating on not empty");

        shared_mem -> buffer[shared_mem -> in] = item;
        printf("Produced %d at: %d\n", item, shared_mem -> in);
        shared_mem -> in = (shared_mem -> in + 1) % BUFFERSIZE;
        shared_mem -> count++;
        countProd++;

        printf("gort");
        sem_post(shared_mem -> mutex);
        printf("gleet");
        sem_post(shared_mem -> not_empty);
        printf("sss");
    }
    printf("Producer stopping");
    return NULL;
}

int main() {
    
    int sM_des = shm_open("/PCProblem", O_CREAT | O_RDWR, 0666);
    if (sM_des == -1)
        perror("AAA");

    ftruncate(sM_des, sizeof(shared_data_t));
    shared_mem = mmap(NULL, sizeof(shared_data_t), PROT_READ | PROT_WRITE, MAP_SHARED, sM_des, 0);

    shared_mem -> in = 0;
    shared_mem -> out = 0;
    shared_mem -> count = 0;

    shared_mem -> mutex = sem_open("/mutex", O_CREAT, 0666, 1);
    shared_mem -> not_full = sem_open("/not_full", O_CREAT, 0666, BUFFERSIZE);
    shared_mem -> not_empty = sem_open("/not_empty", O_CREAT, 0666, 0);


    pthread_t producer_t;

    pthread_create(&producer_t, NULL, producer, NULL);
    pthread_join(producer_t, NULL);

    sem_close(shared_mem->mutex);
    sem_close(shared_mem->not_full);
    sem_close(shared_mem->not_empty);
    sem_unlink("/not_full");
    sem_unlink("/not_empty");
    sem_unlink("/mutex");
    munmap(shared_mem, sizeof(shared_data_t));
    close(sM_des);
    shm_unlink("/PCProblem");

    return 0;
}


