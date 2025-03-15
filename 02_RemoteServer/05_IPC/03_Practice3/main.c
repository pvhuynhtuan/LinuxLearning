/***********************************************************
*       Practice 3 - process communication by signal       *
***********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

int signal_count = 0;
int returnValue;

void Signal_Handler_SIGUSR1(int num)
{
    printf ("Received signal SIGUSR1 from parent!, time = %d\n", ++signal_count);
    if(signal_count >= 5)
    {
        printf("5 times received, Child exited!\n");
        exit(EXIT_FAILURE);
    }
}

void Signal_Handler_SIGALRM(int num)
{
    int ret;
    int status;

    // get status of child process
    ret = waitpid(returnValue, &status, WNOHANG);

    if (0 == ret) // child is not terminated
    {
        printf("\nThe signal SIGUSR1 to be sent to child process, PID = %d\n",returnValue );
        kill(returnValue, SIGUSR1);
        // set alarm to send in next cycle
        alarm(2);
    }
    else if (0 < ret)
    {
        //exit the parent if the child is end
        printf("\nChild is terminated, exit!\n");
        exit(EXIT_SUCCESS);
    }
}

void main(int argc, char *argv[])
{
    int status;
    int ret = 0;

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
        if(signal(SIGALRM, Signal_Handler_SIGALRM) == SIG_ERR)
        {
            fprintf(stderr, "Can't handle SIGINT\n");
            exit(EXIT_FAILURE);
        }

        alarm(2);
        while(1);
        printf("PID = %d\n", getpid());
    }
    else
    {
        printf("Error\n");
    }
}