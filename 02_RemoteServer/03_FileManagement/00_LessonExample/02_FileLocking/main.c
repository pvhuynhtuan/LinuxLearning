#include <stdio.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

void main()
{
    int fd; // file descriptor
    int num_read, num_write;
    char buf[16] = {0};

    sprintf(buf, "Hi there!");

    printf("This is the lesson #2: File Management - File locking!\n");

    fd = open("hello_file.txt", O_RDWR);

    if (-1 == fd)
    {
        printf("open file hello_file.txt failed!\n");
    }
    else
    {
        printf("open file hello_file.txt success!\n");
        if (flock(fd, LOCK_EX) == -1)
        {
            printf("Can't get write lock!\n");
        }
        else
        {
            printf("Set lock!\n");

            write(fd, buf, sizeof(buf) - 1);
        }

        // while(1)
        // {
        //     sleep(1);
        // }
        close(fd);
    }
}
