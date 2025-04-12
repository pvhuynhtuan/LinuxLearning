#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <semaphore.h>

#define POSIX_SEM_NAMED "/named_app"
#define R_W_PERM    0666
#define SEM_WAITING_TIMEOUT 10000
#define MSECS_INSEC 1000

int main(int argc, char const *argv[])
{
    char c;
    int ret = -1;
    int current_value;
    sem_t *sem;
    struct timespec timeout;

    sem = sem_open(POSIX_SEM_NAMED, O_CREAT | O_EXCL, R_W_PERM, 1);
    if (SEM_FAILED == sem)
    {
        if (EEXIST != errno)
        {
            printf ("Failed to open semaphore, error: %s\n", strerror(errno));
            return -1;
        }

        printf("%s, Reading available semaphore.\n", argv[0]);
        sem = sem_open(POSIX_SEM_NAMED, 0);
        if(SEM_FAILED == sem)
        {
            printf ("Failed to open semaphore, error: %s\n", strerror(errno));
            return -1;
        }
    }
    
    /* Get current semaphore */
    sem_getvalue(sem, &current_value);
    printf("Current semaphore value = %d.\n", current_value);

    // locking with timeout
    if (clock_gettime(CLOCK_REALTIME, &timeout) == -1)
    {
        printf("Failed to get current time, error: %s\n", strerror(errno));
    }

    timeout.tv_sec += SEM_WAITING_TIMEOUT / MSECS_INSEC;
    ret = sem_timedwait(sem, &timeout);
    if (-1 == ret)
    {
        printf("Failed to wait semaphore error: %s\n", strerror(errno));
        return -1;
    }

    // Get any character to go next
    printf("%s, Please type any character: ", argv[0]);
    c = getchar();

    ret = sem_post(sem);
    if (ret == -1)
    {
        printf("Failed to release semaphore, error: %s\n", strerror(errno));
        return -1;
    }

    //Enter to exit
    printf("\n%s, Please type any character to exit ...\\n", argv[0]);
    getchar();

    ret = sem_close(sem);
    if (-1 == ret)
    {
        printf("Failed to close semaphore, error: %s\n", strerror(errno));
        return -1; 
    }

    sem_unlink(POSIX_SEM_NAMED);

    return 0;
}