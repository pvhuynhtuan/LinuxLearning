/***********************************************************
*          Practice 2 - handle signal SIGALRM              *
***********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

int signal_count = 0;

void Signal_Handler_SIGALRM(int num)
{
    printf ("SIGALRM received!, time = %d\n", signal_count);
    if(signal_count >= 9)
    {
        printf("10s, exited!\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        signal_count++;
        alarm(1);
    }
}

void main(int argc, char *argv[])
{
    int returnValue;

    if(signal(SIGALRM, Signal_Handler_SIGALRM) == SIG_ERR)
    {
        fprintf(stderr, "Can't handle SIGINT\n");
        exit(EXIT_FAILURE);
    }

    alarm(1);
    while(1);
    printf("PID = %d\n", getpid());
}