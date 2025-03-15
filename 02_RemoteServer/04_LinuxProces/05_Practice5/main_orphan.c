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
        printf("Infinity loop to the child running\n");
        while(1);
    }
    else if (0 < returnValue)
    {
        printf("\nStart of the parent process then stop in 2 seconds, PID = %d\n", getpid());
        sleep(2);
        printf("exited!\n");
        exit(EXIT_SUCCESS); // Kill the parent to make the child become orphan
    }
    else
    {
        printf("Error\n");
    }
}