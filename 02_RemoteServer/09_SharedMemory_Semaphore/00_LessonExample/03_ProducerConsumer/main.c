#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <semaphore.h>
#include <sys/mman.h>

#define BUFFER_SIZE 10

typedef struct Buffer
{
    char **Tuples;
    int inSlotIndex;
    int outSlotIndex;
} Buffer;

int main(int argc, char const *argv[])
{
    char *buffer = (char *)mmap(NULL, sizeof(int) * BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    
    sem_t *mutex = (sem_t *)mmap(NULL, sizeof(sem_t *), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sem_t *full = (sem_t *)mmap(NULL, sizeof(sem_t *), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sem_t *empty = (sem_t *)mmap(NULL, sizeof(sem_t *), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    sem_init(mutex, 1, 1);
    sem_init(full, 1, 1);
    sem_init(empty, 1, BUFFER_SIZE);

    pid_t producer, consumer;

    if ((producer = fork()) == 0)
    {
        while(1)
        {
            sem_wait(empty);
            sem_wait(mutex);

            printf("Producer creates something!\n");

            sem_post(mutex);
            sem_post(full);

            srand(time(NULL));
            sleep(rand() % 5);
        }
    }
    else if ((consumer = fork()) == 0)
    {
        while(1)
        {
            sem_wait(full);
            sem_wait(mutex);

            printf("consumer takes something!\n");

            sem_post(mutex);
            sem_post(empty);

            srand(time(NULL));
            sleep(rand() % 5);
        }
    }
    else
    {
        while(1)
        {
            sleep(10);
            int takenSlots;
            sem_getvalue(full, &takenSlots);
            printf("Items in the buffer: %d/%d\n", takenSlots, BUFFER_SIZE);
        }
    }

    return 0;
}