/*******************************************************************************
*       Practice 4 - ignore signal SIGTSTP                                     *
* Question: process will be "Stopped" when using the combination key Ctrl + Z  *
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

void Signal_Handler_SIGTSTP(int num)
{
    printf ("SIGTSTP ignore!\n");
    // exit(EXIT_FAILURE);
}

void main(int argc, char *argv[])
{
    if(signal(SIGTSTP, Signal_Handler_SIGTSTP) == SIG_ERR)
    {
        fprintf(stderr, "Can't handle SIGTSTP\n");
        exit(EXIT_FAILURE);
    }

    printf("PID = %d\n", getpid());

    while(1);
}