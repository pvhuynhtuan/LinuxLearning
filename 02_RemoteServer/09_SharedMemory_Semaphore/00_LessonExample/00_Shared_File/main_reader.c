#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#define SHARED_MEM_SIZE 100

#define FILE_NAME   "LinuxShared_mmap"


int main(int argc, char const *argv[])
{
    int shm_fd;

    shm_fd = shm_open(FILE_NAME, O_RDWR, 0666);

    if (shm_fd < 0)
    {
        printf("shm_open() is failed! %s\n", strerror(errno));
        return -1;
    }

    ftruncate(shm_fd, SHARED_MEM_SIZE);

    char *data = (char *)mmap(0, SHARED_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE, shm_fd, 0);

    printf("%s: Read data: %s\n", __FILE__, data);

    munmap(data, SHARED_MEM_SIZE);

    close(shm_fd);
    return 0;
}