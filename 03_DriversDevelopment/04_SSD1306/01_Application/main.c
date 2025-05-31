#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define DEVICE_FILE "/dev/ssd1306"

int fd;

void main()
{
    const char buf1 = 1;
    const char buf0 = 0;

    fd = open(DEVICE_FILE, O_CREAT | O_RDWR, 0666);

    write(fd, &buf1, 1);
    sleep(1);

    write(fd, &buf0, 1);
    sleep(1);
    
    write(fd, &buf1, 1);
    sleep(1);

    write(fd, &buf0, 1);
    sleep(1);

    close(fd);

    return;
}