/***********************************************************
*           Practice 4 - Status of process                 *
***********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>


void main(int argc, char *argv[])
{
    int returnValue, status;
    int ret = 0;

    // Fork - EXIT_FAILURE
    returnValue = fork();
    if (0 == returnValue)
    {
        printf("\nIn the child process, PID = %d\n", getpid());
        printf("Child process terminate after 5 seconds\n");
        sleep(5);
        //while(1);
        exit(EXIT_FAILURE);
        //exit(EXIT_SUCCESS);
    }
    else if (0 < returnValue)
    {
        printf("\nStart of the parent process, PID = %d\n", getpid());
        ret = wait(&status);
        if (ret == -1)
        {
            printf("Wait() unsuccessful\n");
        }
        else
        {
            if (WIFEXITED(status))
            {
                printf("Normal termination, status = %d\n", WEXITSTATUS(status));
            }
            else if (WIFSIGNALED(status))
            {
                printf("Killed by signal, value = %d\n", WTERMSIG(status));
            }
        }

        printf("\nIn the parent process, PID = %d; child PID = %d\n", getpid(), ret);
    }
    else
    {
        printf("Error\n");
    }

    // Fork - EXIT_SUCCESS
    returnValue = fork();
    if (0 == returnValue)
    {
        printf("\nIn the child process, PID = %d\n", getpid());
        printf("Child process terminate after 5 seconds\n");
        sleep(5);
        //while(1);
        //exit(EXIT_FAILURE);
        exit(EXIT_SUCCESS);
    }
    else if (0 < returnValue)
    {
        printf("\nStart of the parent process, PID = %d\n", getpid());
        ret = wait(&status);
        if (ret == -1)
        {
            printf("Wait() unsuccessful\n");
        }
        else
        {
            if (WIFEXITED(status))
            {
                printf("Normal termination, status = %d\n", WEXITSTATUS(status));
            }
            else if (WIFSIGNALED(status))
            {
                printf("Killed by signal, value = %d\n", WTERMSIG(status));
            }
        }

        printf("\nIn the parent process, PID = %d; child PID = %d\n", getpid(), ret);
    }
    else
    {
        printf("Error\n");
    }
}