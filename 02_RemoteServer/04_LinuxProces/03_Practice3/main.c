/***********************************************************
*             Practice 3 - signal of process               *
***********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

void Signal_Handler_SIGUSR1(int num)
{
    printf ("I'm signal handler for SIGUSR1! PID = %d\n", getpid());
    exit(EXIT_FAILURE);
}

void main(int argc, char *argv[])
{
    int returnValue;

    returnValue = fork();
    if (0 == returnValue)
    {
        printf("\nIn the child process, PID = %d\n", getpid());
        if(signal(SIGUSR1, Signal_Handler_SIGUSR1) == SIG_ERR)
        {
            fprintf(stderr, "Can't handle SIGUSR1\n");
            exit(EXIT_FAILURE);
        }
        while(1);
    }
    else if (0 < returnValue)
    {
        printf("\nIn the parent process, PID = %d\n", getpid());
        printf("Parent process will send the SIGUSR1 tp child after 5 seconds\n");
        sleep(5);
        kill(returnValue, SIGUSR1);
    }
    else
    {
        printf("Error\n");
    }
}