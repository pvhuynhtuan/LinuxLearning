#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void main()
{
    printf("Run command <ls -lah> after 2 seconds\n");
    sleep(2);
    execl("/bin/ls", "ls", "-l", "-a", "-h", NULL);
    while(1);

    return;
}