/*******************************************************************************
*       Practice 5                                                             *
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/poll.h>

void Signal_Handler_SIGINT(int num)
{
    printf ("SIGINT Received!\n");
    // exit(EXIT_FAILURE);
}

void Signal_Handler_SIGTERM(int num)
{
    printf ("SIGTERM Received!\n");
    exit(EXIT_FAILURE);
}

void main(int argc, char *argv[])
{
    struct pollfd fds[2];
	int ret;

    if(signal(SIGINT, Signal_Handler_SIGINT) == SIG_ERR)
    {
        fprintf(stderr, "Can't handle SIGINT\n");
        exit(EXIT_FAILURE);
    }

    // Handle for SIGTERM
    if(signal(SIGTERM, Signal_Handler_SIGTERM) == SIG_ERR)
    {
        fprintf(stderr, "Can't handle SIGTERM\n");
        exit(EXIT_FAILURE);
    }

    printf("PID = %d\n", getpid());

	/* watch stdin for input */
	fds[0].fd = STDIN_FILENO;
	fds[0].events = POLLIN;

	/* watch stdout for ability to write */
	fds[1].fd = STDOUT_FILENO;
	fds[1].events = POLLOUT;

	ret = poll(fds, 2, 5000);

	if (ret == -1) {
		printf("poll error!\n");
		exit(EXIT_FAILURE);
	}

	if (0 < ret) {
		printf ("%d seconds elapsed.\n", 5);
	}

	if (fds[0].revents & POLLIN)
    {
        printf ("stdin is readable\n");
    }

	if (fds[1].revents & POLLOUT)
    {
        printf ("stdout is writable\n");
    }
    while(1);
}