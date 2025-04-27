#ifndef _SG_MAIN_H_
#define _SG_MAIN_H_

/********************************************************
*                    INCLUDE SECTION                    *
********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <poll.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

/********************************************************
*                     DEFINE SECTION                    *
********************************************************/
// ANSI color codes
#define RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"

#define SG_FIFO_FILENAME                "./bin/LogFifo"
#define SG_MAX_LOG_LENGTH               256

/********************************************************
*                    TYPEDEF SECTION                    *
********************************************************/

/********************************************************
*          FUNCTION DECLARATION SECTION                 *
********************************************************/

#endif /* _SG_MAIN_H_ */