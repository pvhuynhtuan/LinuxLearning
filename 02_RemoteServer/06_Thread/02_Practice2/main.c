/***********************************************************
*       Practice 1 - thread synchronize with MUTEX         *
***********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_THREAD  3
#define COUNT_MAX   1000000
// option 1 to init mutex: static init
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int pBuff[5] = {0};

pthread_t tids[NUM_THREAD];
int count = 0;

void *thread_functions(void *agrv)
{
    for (int i = 0; i < COUNT_MAX; i++)
    {
        pthread_mutex_lock(&mutex);
        count++;
        pthread_mutex_unlock(&mutex); // if remove this line ==> dead lock.
    }
}

int main(void)
{
    for (int i = 0; i < NUM_THREAD; i++)
    {
        pthread_create(&tids[i], NULL, thread_functions, NULL);
    }

    for (int i = 0; i < NUM_THREAD; i++)
    {
        pthread_join(tids[i], NULL);
    }

    printf("count = %d\n", count);
    pthread_mutex_destroy(&mutex);
    return EXIT_SUCCESS;
}