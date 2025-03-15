#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

void Signal_Handler_SIGINT(int num)
{
    printf ("I'm signal handler for SIGINT!\n");
    exit(EXIT_FAILURE);
}

void main(int argc, char *argv[])
{
    if(signal(SIGINT, Signal_Handler_SIGINT) == SIG_ERR)
    {
        fprintf(stderr, "Can't handle SIGINT\n");
        exit(EXIT_FAILURE);
    }

    printf("PID = %d\n", getpid());

    while(1);
}