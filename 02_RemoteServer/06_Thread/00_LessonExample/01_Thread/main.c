#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_THREAD  5
#define COUNT   1000000

pthread_t tids[NUM_THREAD];
int count = 0;

void *thread_functions(void *agrv)
{
    for (int i = 0; i < COUNT; i++)
    {
        count++;
    }
}

void *Thread_function1(void * argv)
{
    // struct abc *mabc = (struct abc *)(argv)
    while(1)
    {
        printf ("hello!\n");
    }
}

int main(void)
{
    pthread_t tid;

    // pthread_create(&tid, NULL, Thread_function1, NULL);

    // pthread_join(tid, NULL);
    // sleep(2);
    for (int i = 0; i < NUM_THREAD; i++)
    {
        pthread_create(&tids[i], NULL, thread_functions, NULL);
    }

    for (int i = 0; i < NUM_THREAD; i++)
    {
        pthread_join(tids[i], NULL);
    }

    printf("count = %d", count);
    return EXIT_SUCCESS;
}