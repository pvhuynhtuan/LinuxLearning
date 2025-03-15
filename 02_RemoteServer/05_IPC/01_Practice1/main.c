/***********************************************************
*          Practice 1 - handle signal SIGINT               *
***********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

int signal_count = 0;

void Signal_Handler_SIGINT(int num)
{
    signal_count++;
    printf ("SIGINT received!, num = %d\n", num);
    if(signal_count >= 3)
    {
        // If ignore the SIGINT signal, the process can't exit by SIGINT ==> kill by other signals like -9
        printf("Interrupt 3 times, exited!\n");
        exit(EXIT_FAILURE);
    }
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