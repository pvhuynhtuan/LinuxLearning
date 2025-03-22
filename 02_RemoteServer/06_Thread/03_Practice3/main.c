/***********************************************************
*          Practice 3 - Condition variables                *
***********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define LOOP_TIME   10
#define BUFFER_SIZE 5

// option 1 to init mutex: static init
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition_ProduceDone = PTHREAD_COND_INITIALIZER;
pthread_cond_t condition_ConsumerDone = PTHREAD_COND_INITIALIZER;

int pBuff[BUFFER_SIZE] = {0};
int RunCount = 0;

pthread_t tid_producer[LOOP_TIME];
pthread_t tid_consumer[LOOP_TIME];

void *Thread_producer(void *agrv)
{
    printf("\nThis is the Producer!\n");
    for (int i = 0; i < BUFFER_SIZE; i++)
    {
        pthread_mutex_lock(&mutex);
        if (RunCount == BUFFER_SIZE)
        {
            pthread_cond_wait(&condition_ConsumerDone, &mutex);
        }
        pBuff[i] = rand() % 100;
        RunCount++;
        printf("pBuff[%d] = %d, count = %d\n", i, pBuff[i], RunCount);
        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&condition_ProduceDone);
    }
    printf("Producer Done %d !\n", RunCount);
}

void *Thread_consumer(void * argv)
{
    printf("\n\tThis is the Consumer! %d\n", RunCount);
    for (int i = 0; i < BUFFER_SIZE; i++)
    {
        pthread_mutex_lock(&mutex);
        if (RunCount == 0);
        {
            pthread_cond_wait(&condition_ProduceDone, &mutex);
        }
        RunCount--;
        printf("\tpBuff[%d] = %d, count = %d\n", i, pBuff[i], RunCount);
        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&condition_ConsumerDone);
    }
    printf("\tConsumer Done!\n");
}

int main(void)
{
    for (int i = 0; i < LOOP_TIME; i++)
    {
        pthread_create(&tid_consumer[i], NULL, Thread_consumer, NULL);
        pthread_create(&tid_producer[i], NULL, Thread_producer, NULL);
    }

    for (int i = 0; i < LOOP_TIME; i++)
    {
        pthread_join(tid_consumer[i], NULL);
        pthread_join(tid_producer[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    return EXIT_SUCCESS;
}