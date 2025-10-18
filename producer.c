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

        sem_wait(shared_mem->not_full);
        sem_wait(shared_mem->mutex);

        shared_mem->buffer[shared_mem->in] = item;
        printf("Produced %d at index %d\n", item, shared_mem->in); fflush(stdout);

        shared_mem->in = (shared_mem->in + 1) % BUFFERSIZE;
        shared_mem->count++;
        countProd++;

        sem_post(shared_mem->mutex);
        sem_post(shared_mem->not_empty);
    }

    printf("Producer done\n"); fflush(stdout);
    return NULL;
}

int main() {
    int shm_fd = shm_open("/PCProblem", O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open failed");
        exit(1);
    }

    ftruncate(shm_fd, sizeof(shared_data_t));
    shared_mem = mmap(NULL, sizeof(shared_data_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_mem == MAP_FAILED) {
        perror("mmap failed");
        exit(1);
    }

    // Initialize shared memory fields
    shared_mem->in = 0;
    shared_mem->out = 0;
    shared_mem->count = 0;

    // Create and initialize semaphores
    shared_mem->mutex = sem_open("/mutex", O_CREAT | O_EXCL, 0666, 1);
    if (shared_mem->mutex == SEM_FAILED) shared_mem->mutex = sem_open("/mutex", 0);

    shared_mem->not_full = sem_open("/not_full", O_CREAT | O_EXCL, 0666, BUFFERSIZE);
    if (shared_mem->not_full == SEM_FAILED) shared_mem->not_full = sem_open("/not_full", 0);

    shared_mem->not_empty = sem_open("/not_empty", O_CREAT | O_EXCL, 0666, 0);
    if (shared_mem->not_empty == SEM_FAILED) shared_mem->not_empty = sem_open("/not_empty", 0);

    pthread_t prod_thread;
    pthread_create(&prod_thread, NULL, producer, NULL);
    pthread_join(prod_thread, NULL);

    // Cleanup
    sem_close(shared_mem->mutex);
    sem_close(shared_mem->not_full);
    sem_close(shared_mem->not_empty);
    sem_unlink("/mutex");
    sem_unlink("/not_full");
    sem_unlink("/not_empty");
    munmap(shared_mem, sizeof(shared_data_t));
    close(shm_fd);
    shm_unlink("/PCProblem");

    return 0;
}
