#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

void main()
{
    int fd; // file descriptor
    int num_read, num_write;
    char buf1[] = "Hi file!\n";

    printf("This is the lesson #2: File Management!\n");

    fd = open("hello_file.txt", O_WRONLY);
    if (-1 == fd)
    {
        printf("open file hello_file.txt failed!\n");
    }

    num_write = write(fd, buf1, sizeof(buf1)); //can replace sizeof = strlen
    printf("write %d byte to hello_file.txt!\n", num_write);

    lseek(fd, 5, SEEK_SET);
    write(fd, "AAAAA", 5);

    close(fd);
}
