/***********************************************************
*         Practice 1 - Initialize and observe process      *
* Requirement: Write a program to create a child process   *
* by involke fork(). Then child process shall print PID    *
* ifself. The parent process shall print the PID of child  *
***********************************************************/
#include <stdio.h>
#include <unistd.h>

void main(int argc, char *argv[])
{
    int returnValue;
    int a = 0;

    returnValue = fork();
    if (0 == returnValue)
    {
        printf("\nIn the child process, counter = %d\n", a);
        printf("Child PID = %d, parent PID = %d\n", getpid(), getppid());
    }
    else if (0 < returnValue)
    {
        printf("\nIn the parent process, counter = %d\n", ++a);
        printf("parent PID = %d, child process PID = %d\n", getpid(), returnValue);
    }
    else
    {
        printf("Error\n");
    }
}
