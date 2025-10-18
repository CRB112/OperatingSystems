#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "globals.h"

shared_data_t *shared_mem;
    sem_t *mutex;
    sem_t *not_full;
    sem_t *not_empty;


void* producer(void* arg) {
    int countProd = 0;

    while (countProd < 10) {
        int item = (rand() % 10) + 1;

        sem_wait(not_full);
        sem_wait(mutex);

        shared_mem->buffer[shared_mem->in] = item;
        printf("Produced %d at index %d\n", item, shared_mem->in); fflush(stdout);

        shared_mem->in = (shared_mem->in + 1) % BUFFERSIZE;
        shared_mem->count++;
        countProd++;

        sem_post(mutex);
        sem_post(not_empty);
    }

    printf("Producer done\n"); fflush(stdout);
    return NULL;
}

int main() {
    int shm_fd = shm_open("/PCProblem", O_CREAT | O_RDWR, 0666);

    ftruncate(shm_fd, sizeof(shared_data_t));
    shared_mem = mmap(NULL, sizeof(shared_data_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);


    shared_mem->in = 0;
    shared_mem->out = 0;
    shared_mem->count = 0;

    mutex = sem_open("/mutex", O_CREAT | O_EXCL, 0666, 1);
    if (mutex == SEM_FAILED) mutex = sem_open("/mutex", 0);

    not_full = sem_open("/not_full", O_CREAT | O_EXCL, 0666, BUFFERSIZE - 1);
    if (not_full == SEM_FAILED) not_full = sem_open("/not_full", 0);

    not_empty = sem_open("/not_empty", O_CREAT | O_EXCL, 0666, 0);
    if (not_empty == SEM_FAILED) not_empty = sem_open("/not_empty", 0);

    pthread_t prod_thread;
    pthread_create(&prod_thread, NULL, producer, NULL);
    pthread_join(prod_thread, NULL);

    sem_close(mutex);
    sem_close(not_full);
    sem_close(not_empty);
    sem_unlink("/mutex");
    sem_unlink("/not_full");
    sem_unlink("/not_empty");
    munmap(shared_mem, sizeof(shared_data_t));
    close(shm_fd);
    shm_unlink("/PCProblem");


    _exit(0);
}
