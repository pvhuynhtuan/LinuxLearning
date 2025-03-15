/***********************************************************
*       Practice 5 - Zombie and Orplan process             *
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

    // Fork
    returnValue = fork();
    if (0 == returnValue)
    {
        printf("\nIn the child zombie process, PID = %d\n", getpid());
        printf("Child exited\n");
        exit(EXIT_SUCCESS);
    }
    else if (0 < returnValue)
    {
        printf("\nStart of the parent process, infinity loop then call wait, PID = %d\n", getpid());
        while(1);
        // use ps -aux | grep -iE zombie.out to check the Z+ status
        // Kill the parent to kill the zombie
        printf("waiting!\n");
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
    }
    else
    {
        printf("Error\n");
    }
}