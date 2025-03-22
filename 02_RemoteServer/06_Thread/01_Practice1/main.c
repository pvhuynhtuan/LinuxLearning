/***********************************************************
*          Practice 1 - handle thread basic                *
***********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

void *thread_function1(void *agrv)
{
    printf("Thread 1 Hello!\n");
}

void *thread_function2(void *agrv)
{
    printf("Thread 2 Hello!\n");
}

int main(void)
{
    pthread_t tid1, tid2;

    pthread_create(&tid1, NULL, thread_function1, NULL);
    pthread_create(&tid2, NULL, thread_function2, NULL);

    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

    return EXIT_SUCCESS;
}