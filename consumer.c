#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>

#include "globals.h"

shared_data_t *shared_mem;
    sem_t *mutex;
    sem_t *not_full;
    sem_t *not_empty;

void* consumer(void* arg) {
    int countCons = 0;

    while (countCons < 10) {
        sem_wait(not_empty);
        sem_wait(mutex);

        int item = shared_mem->buffer[shared_mem->out];
        printf("Consumed %d at index %d\n", item, shared_mem->out); fflush(stdout);

        shared_mem->out = (shared_mem->out + 1) % BUFFERSIZE;
        shared_mem->count--;
        countCons++;

        sem_post(mutex);
        sem_post(not_full);
    }

    printf("Consumer done\n"); fflush(stdout);
    return NULL;
}

int main() {
    sleep(3);
    int shm_fd;
    shm_fd = shm_open("/PCProblem", O_RDWR, 0666);

    shared_mem = mmap(NULL, sizeof(shared_data_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    mutex = sem_open("/mutex", 0);
    not_full = sem_open("/not_full", 0);
    not_empty = sem_open("/not_empty", 0);


    pthread_t cons_thread;
    pthread_create(&cons_thread, NULL, consumer, NULL);
    pthread_join(cons_thread, NULL);

    sem_close(mutex);
    sem_close(not_full);
    sem_close(not_empty);
    sem_unlink("/mutex");
    sem_unlink("/not_full");
    sem_unlink("/not_empty");
    munmap(shared_mem, sizeof(shared_data_t));
    close(shm_fd);

    exit(0);
}
