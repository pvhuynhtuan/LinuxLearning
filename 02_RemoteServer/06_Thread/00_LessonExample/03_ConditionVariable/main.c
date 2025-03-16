#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define BUFFER_SIZE 5
#define NUM_THREAD 10

// option 1 to init mutex: static init
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition_notempty = PTHREAD_COND_INITIALIZER;
pthread_cond_t condition_notfull = PTHREAD_COND_INITIALIZER;

int pBuff[BUFFER_SIZE] = {0};
int putptr = 0;
int takeptr = 0;
int count = 0;

pthread_t tid_producer[NUM_THREAD];
pthread_t tid_consumer[NUM_THREAD];

void *Thread_producer(void *agrv)
{
    int loop = BUFFER_SIZE * 2;
    while (loop-- > 0)
    {
        pthread_mutex_lock(&mutex);
        if (BUFFER_SIZE == count)
        {
            pthread_cond_wait(&condition_notfull, &mutex);
        }
        pBuff[putptr] = rand() % 100;
        count++;

        printf("Producer pBuff[%d] = %d\n", putptr, pBuff[putptr]);
        putptr = (putptr + 1) % BUFFER_SIZE;
        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&condition_notempty);
    }
}

void *Thread_consumer(void * argv)
{
    int loop = BUFFER_SIZE * 2;
    while (loop-- > 0)
    {
        pthread_mutex_lock(&mutex);
        if (0 == count)
        {
            pthread_cond_wait(&condition_notempty, &mutex);
        }
        printf("\tConsumer pBuff[%d] = %d\n", takeptr, pBuff[takeptr]);
        count--;
        takeptr = (takeptr + 1) % BUFFER_SIZE;
        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&condition_notfull);
    }
}

int main(void)
{
    pthread_t tid;
    for (int i = 0; i < NUM_THREAD; i++)
    {
        pthread_create(&tid_producer[i], NULL, Thread_producer, NULL);
        pthread_create(&tid_consumer[i], NULL, Thread_consumer, NULL);
    }
    
    for (int i = 0; i < NUM_THREAD; i++)
    {
        pthread_join(tid_producer[i], NULL);
        pthread_join(tid_consumer[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    return EXIT_SUCCESS;
}