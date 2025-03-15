#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


void main(int argc, char *argv[])
{
    int returnValue;
    int a = 0;

    printf("Number of arguments from shell: = %d\n", argc);

    if (strcmp(argv[1], "1") == 0)
    {
        printf("OK con de!\n");
    }
    else if (strcmp(argv[1], "2") == 0)
    {
        printf("OK con bo!\n");
    }

    // Fork
    returnValue = fork();
    if (0 == returnValue)
    {
        printf("\nIn the child process, counter = %d\n", a);
        printf("Child PID = %d, parent ID = %d\n", getpid(), getppid());
    }
    else if (0 < returnValue)
    {
        printf("\nIn the parent process, counter = %d\n", ++a);
        printf("parent PID = %d, parent of parent ID = %d\n", getpid(), getppid());
    }
    else
    {
        printf("Error\n");
    }
}