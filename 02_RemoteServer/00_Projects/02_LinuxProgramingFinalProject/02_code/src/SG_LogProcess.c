#include <stdio.h>
#include <unistd.h>

#include "main.h"
#include "SG_LogProcess.h"

/********************************************************
*                     DEFINE SECTION                    *
********************************************************/
#define LP_LOG_FILENAME         "./bin/gateway.log"

/********************************************************
*                 CONSTANCE SECTION                     *
********************************************************/

/********************************************************
*         GLOBAL VARIABLE DECLARATION SECTION           *
********************************************************/
int giReadFifoFD;
int giLogFileFD;

/********************************************************
*          FUNCTION DECLARATION SECTION                 *
********************************************************/
void LP_Signal_Handler_SIGTSTP(int num);

void LP_main()
{
    int liReadBytes;
    char lpLogBuffer[SG_MAX_LOG_LENGTH];
    // Start the FIFO with the write only
    giReadFifoFD = open(SG_FIFO_FILENAME, O_RDONLY);
    if (-1 == giReadFifoFD)
    {
        perror("Log Process > open (reader)");
        exit(EXIT_FAILURE);
    }

    giLogFileFD = open(LP_LOG_FILENAME, O_WRONLY | O_CREAT | O_APPEND, 0666);
    if (-1 == giLogFileFD)
    {
        perror(RED "Log Process > Failed to open log file" RESET "\n");
        exit(EXIT_FAILURE);
    }

    while(1)
    {
        // Clear Data before read
        memset(lpLogBuffer, 0, SG_MAX_LOG_LENGTH);

        //Read the data
        liReadBytes = read(giReadFifoFD, lpLogBuffer, SG_MAX_LOG_LENGTH);
        if (liReadBytes == -1) {
            printf("Log Process > read() failed\n");
            exit(0);
        } else if (liReadBytes == 0) {
            printf("Log Process > pipe end-of-pipe\n");
            break;
        } else {
            #if (LP_DEBUG_LOG_ENABLE == 1)
            printf("Log Process > msg: %s\n", lpLogBuffer);
            #endif

            // Write the line to the file
            ssize_t liBytesWritten = write(giLogFileFD, lpLogBuffer, strlen(lpLogBuffer));
            if (-1 == liBytesWritten) {
                perror(RED "Log Process > Failed to write to file" RESET "\n");
                close(liBytesWritten);
                exit(EXIT_FAILURE);
            }
        }
    }
}

void LP_Signal_Handler_SIGTSTP(int num)
{
    // Trigger the close condition for Connection Manager
    close(giReadFifoFD);
    close(giLogFileFD);
    printf("Log Process > Closing the Log Process!\n");
    exit(EXIT_SUCCESS);
}