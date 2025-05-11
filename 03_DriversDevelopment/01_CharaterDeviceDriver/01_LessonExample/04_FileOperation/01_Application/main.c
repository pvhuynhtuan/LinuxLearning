#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define DEVICE_FILE "/dev/my_HT_device"

void main()
{
    int fd;
    const char * message = "Hello from application";

    fd = open(DEVICE_FILE, O_CREAT | O_RDWR, 0666);
    
    write(fd, message, strlen(message) + 1);

    close(fd);

    return;
}