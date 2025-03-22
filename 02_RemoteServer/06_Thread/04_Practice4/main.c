/***********************************************************
*         Practice 4 - Count the odd and even number       *
***********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define COUNT   100

int source[COUNT] = {0};
int numEven = 0;
int numOdd = 0;

void *Thread_CountEven(void *agrv)
{
    for (int i = 0; i < COUNT; i++)
    {
        numEven += (source[i]%2 == 0)?1:0;
    }
}

void *Thread_CountOdd(void * argv)
{
    for (int i = 0; i < COUNT; i++)
    {
        numOdd += source[i]%2;
    }
}

int main(void)
{
    pthread_t pthread_CountEven, pthread_CountOdd;
    
    // Initialize the source
    for (int i = 0; i < COUNT; i++)
    {
        source[i] = rand() % 100;
        printf("source[%d] = %d\n", i, source[i]);
    }

    pthread_create(&pthread_CountEven, NULL, Thread_CountEven, NULL);
    pthread_create(&pthread_CountOdd, NULL, Thread_CountOdd, NULL);

    pthread_join(pthread_CountEven, NULL);
    pthread_join(pthread_CountOdd, NULL);

    printf("\nNumber of Event items = %d\n", numEven);
    printf("Number of Odd items = %d\n", numOdd);
    return EXIT_SUCCESS;
}