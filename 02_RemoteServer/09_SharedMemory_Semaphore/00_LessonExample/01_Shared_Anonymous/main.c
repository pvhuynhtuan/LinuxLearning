#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/types.h>

int main(int argc, char const *argv[])
{
    pid_t child_pid;

    char *data = (char *)mmap(0, 1, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    // char *data = (char *)mmap(0, 1, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (MAP_FAILED == data)
    {
        printf("%s: mmap failed!\n", __FILE__);
    }
    *data = 'A';

    child_pid = fork();
    if (child_pid >= 0)
    {
        if (0 == child_pid)
        {
            // Child process
            printf("Child started, value = %c\n", ++(*data));
            if (munmap(data, sizeof(int)) == -1)
            {
                printf("munmap failed\n");
            }
            exit(EXIT_SUCCESS);
        }
        else
        {
            // Parent process
            wait(NULL);

            printf("In parent process, value = %c\n", *data);
            if (munmap(data, sizeof(int)) == -1)
            {
                printf("munmap failed\n");
            }
            exit(EXIT_SUCCESS);
        }
    }
    else
    {
        printf("fork() failed!\n");
    }

    return 0;
}