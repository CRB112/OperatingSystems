# Threads Project (Operating Systems)
## Compiling and Running the Threads
Ensure that *PTHREADS* is installed on your linux based system.
After downloading the files, they can be compiled by running
```
gcc -pthread -lrt producer.c -o producer
```
and
```
gcc -pthread -lrt consumer.c -o consumer
```
In the terminal. This will produce `consumer.o` and `producer.o`.
These can now be run in seperate terminals using:
```
Terminal 1:
./consumer
```
```
Terminal 2:
./producer
```
or they can be run in the same terminal using:
```
./producer & ./consumer &
```
The producer will produce 10 items and the consumer will consumer 10 items within the 2 item buffer.

## Proof of Running:
After using the command above, it produces this output:

<img width="691" height="492" alt="SS" src="https://github.com/user-attachments/assets/1f689d9d-1d5f-4b31-8833-f9a1e73eac91" />

# How it works
The producer consumer problem uses a few key concepts that are necessary for the environment:
 - **globals.h** - A small header file that includes the definition for *shared_data_t* a struct with shared values necessary for the thread navigation, this includes the *BUFFERSIZE* and the buffer array itself
 - **Shared Memory** - Both files contain the lines:

   ```
   shm_fd = shm_open("/PCProblem", O_RDWR, 0666);

   shared_mem = mmap(NULL, sizeof(shared_data_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
   ```
   In order to open a shared memory object named *PCProblem*. *Producer* is used as the main constructor, with the additional tag *O_CREAT* in order to create the object if it has not already been instantiated before.
   The consumer looks for a shared memory object of the same name.
 - **Semaphores** - In order for the two threads to communicate their states with each other, a few sempaphores are used:
   -   *Mutex* - The mutex lock to control which thread has access to their critical section
   -   *not_full* - Gets posted when the *producer* inserts an item. Controls when the *consumer* can begin waiting for the *mutex lock*
   -   *not_empty* - Gets posted when the *consumer* consumes an itme. Controls when the *producer* can begin waiting for the *mutex lock*
   These sempahores are created and accessed  by both threads using the lines:
  ```
      mutex = sem_open("/mutex", 1);
      not_full = sem_open("/not_full", BUFFERSIZE - 1);
      not_empty = sem_open("/not_empty", 0);
  ```
   With the only difference being that the *consumer* will **NOT** be assigning these values, and will just read them, replacing the second parameter with **0**
 - **Thread creation / destruction** - In both files, the producer and consumer will create a new thread according to the thread function that lies above them (more on that soon). The system will then wait until these finish
   before destroying them / continuing to the last step of the program.
   ```
    pthread_t prod_thread;
    pthread_create(&prod_thread, NULL, producer, NULL);
    pthread_join(prod_thread, NULL);
   ```
 - **Cleanup and Exit** - Both files will dereference their semaphores, and unlink all of the data. The *producer* will also unmap the shared memory before exiting, to ensure theres no zombie data.
   ```
   sem_close(mutex);
   sem_close(not_full);
   sem_close(not_empty);
   sem_unlink("/mutex");
   sem_unlink("/not_full");
   sem_unlink("/not_empty");
   munmap(shared_mem, sizeof(shared_data_t));
   close(shm_fd);
   shm_unlink("/PCProblem");
   ```
## Thread execution
Each thread *consumer / producer* has its own function respectively, the return type is *void\** which, along with the *void\* arg* parameter is what makes them thread functions.

**CONSUMER**
 - While the consumer has still consumed less than 10 objects:
   - Wait for an object to be in the buffer
   - Wait for mutex lock access
   - Obtains item at the *out* position dictated by the shared structure
   - Prints item
   - Moves the *out* position forward 1 (circular)
   - Gives mutex lock to next thread
   - Posts to not full (lets producer produce)
  - Returns *NULL* to exit thread

**PRODUCER**
 - While the producer has still produced less than 10 objects:
   - Wait for an open spot to be in the buffer
   - Wait for mutex lock access
   - Obtains a random number from *rand()*
   - Sets the item at the *in* position dictated by the shared structure
   - Prints item
   - Moves the *in* position forward 1 (circular)
   - Gives mutex lock to next thread
   - Posts to not empty (lets lets consumer consume)
  - Returns *NULL* to exit thread





