/***********************************************************
*    Practice 1 - using exec and environment variable      *
***********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void main(int argc, char *argv[])
{
    int returnValue;

    returnValue = fork();
    if (0 == returnValue)
    {
        printf("\nIn the child process, PID = %d\n", getpid());
        if (strcmp(argv[1], "1") == 0)
        {
            execlp("/bin/ls", "ls", "-l", NULL);
        }
        else if (strcmp(argv[1], "2") == 0)
        {
            execlp("/usr/bin/date", "date", NULL);
        }
    }
    else if (0 < returnValue)
    {
        printf("\nIn the parent process, PID = %d\n", getpid());
    }
    else
    {
        printf("Error\n");
    }
}