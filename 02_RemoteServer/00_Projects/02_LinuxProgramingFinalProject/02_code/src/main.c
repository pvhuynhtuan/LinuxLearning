/********************************************************
*                    INCLUDE SECTION                    *
********************************************************/
// include frame work header

// Include user's header
#include "main.h"
#include "SG_MainProcess.h"
#include "SG_LogProcess.h"

/********************************************************
*                     DEFINE SECTION                    *
********************************************************/
#define COMMAND_BUFFER_SIZE             1024

/********************************************************
*                    TYPEDEF SECTION                    *
********************************************************/

/********************************************************
*                 CONSTANCE SECTION                     *
********************************************************/

/********************************************************
*          FUNCTION DECLARATION SECTION                 *
********************************************************/
void Signal_Handler_SIGTSTP(int num);
void SG_Exit();
void SG_PrintCpuUsage();

/********************************************************
*         GLOBAL VARIABLE DECLARATION SECTION           *
********************************************************/
gid_t gsMainProcessPID, gsLogProcessPID;


int main(int argc, char *argv[])
{
    char lpCommand[COMMAND_BUFFER_SIZE] = {0};

    if (argc < 2)
    {
        printf("SG > Please input the target port!\n");
        return EXIT_SUCCESS;
    }

    if(signal(SIGTSTP, Signal_Handler_SIGTSTP) == SIG_ERR)
    {
        fprintf(stderr, "SG > Can't handle SIGTSTP\n");
        exit(EXIT_FAILURE);
    }

    // Create FIFO if it doesn't exist
    if (mkfifo(SG_FIFO_FILENAME, 0666) == -1)
    {
        perror("SG > mkfifo");
    }

    //Req: SG-REQ-FNC-1
    if (0 == (gsMainProcessPID = fork()))
    {
        MP_main(atoi(argv[1]));
    }
    else if (0 == (gsLogProcessPID = fork()))
    {
        LP_main();
    }
    else
    {
        // This is the parent process
        while(1)
        {
            SG_PrintCpuUsage();
            sleep(5);
        }
    }
    return 1;
}

void Signal_Handler_SIGTSTP(int num)
{
    SG_Exit();
}

void SG_Exit()
{
    int ret;
    int status;

    printf ("SG > Terminating the application...\n");

    // get status of Main process
    ret = waitpid(gsMainProcessPID, &status, WNOHANG);

    if (0 == ret) // child is not terminated
    {
        //Killing the child Process
        printf("SG > Killing the Main Process\n");
        kill(gsMainProcessPID, SIGTSTP);
        waitpid(gsMainProcessPID, &status, 0);
    }
    else if (0 < ret)
    {
        //exit the parent if the child is end
        printf("\nSG > Main process is terminated!\n");
    }

    // get status of Log process
    ret = waitpid(gsLogProcessPID, &status, WNOHANG);

    if (0 == ret) // child is not terminated
    {
        //Killing the child Process
        printf("SG > Killing the Log Process\n");
        kill(gsLogProcessPID, SIGTSTP);
        waitpid(gsLogProcessPID, &status, 0);
    }
    else if (0 < ret)
    {
        //exit the parent if the child is end
        printf("\nSG > Log process is terminated!\n");
    }
    exit(EXIT_SUCCESS);
}

/*******************************************************************************************
 * Function name: SG_PrintCpuUsage
 * Functionality: The thread function of Data Manager
 * Functionality - How to calculate the CPU usage: 
 * These numbers are ever-increasing. You can't tell "current usage" from just one snapshot. But if you:
 * Take a first reading (e.g. idle1, total1)
 * Wait a moment (e.g. 1 second)
 * Take a second reading (idle2, total2)
 * Calculate the deltas:
 *       totald = total2 - total1
 *       idled = idle2 - idle1
 * Then:
 *       cpu_usage = (1 - idled / totald) * 100.0
 ******************************************************************************************/
void SG_PrintCpuUsage()
{
    int liStatReadFd;
    char lpReadBuffer[1024];
    unsigned long long int lullUser, lullNice, lullSystem, lullIdle, lullIowait, lullIrq, lullSoftirq;
    unsigned long long int lullPrevIdle, lullPrevTotal, lullIdleSecondRead, lullTotalSecondRead;

    // First read
    liStatReadFd = open("/proc/stat", O_RDONLY);
    if (-1 == liStatReadFd)
    {
        perror("SG > open");
        return;
    }

    read(liStatReadFd, lpReadBuffer, sizeof(lpReadBuffer) - 1);
    close(liStatReadFd);

    sscanf(lpReadBuffer, "cpu %llu %llu %llu %llu %llu %llu %llu", 
        &lullUser, &lullNice, &lullSystem, &lullIdle, &lullIowait, &lullIrq, &lullSoftirq);

    // Calculate the difference
    lullPrevIdle = lullIdle + lullIowait;
    lullPrevTotal = lullUser + lullNice + lullSystem + lullIdle + lullIowait + lullIrq + lullSoftirq;

    sleep(1); // wait 1 second for system update the status

    // Second read
    memset(lpReadBuffer, 0, sizeof(lpReadBuffer));
    liStatReadFd = open("/proc/stat", O_RDONLY);
    if (-1 == liStatReadFd)
    {
        perror("SG > open");
        return;
    }

    read(liStatReadFd, lpReadBuffer, sizeof(lpReadBuffer) - 1);
    close(liStatReadFd);

    sscanf(lpReadBuffer, "cpu %llu %llu %llu %llu %llu %llu %llu", 
        &lullUser, &lullNice, &lullSystem, &lullIdle, &lullIowait, &lullIrq, &lullSoftirq);

    lullIdleSecondRead = lullIdle + lullIowait;
    lullTotalSecondRead = lullUser + lullNice + lullSystem + lullIdle + lullIowait + lullIrq + lullSoftirq;

    // Calculate the difference
    double ldTotal = lullTotalSecondRead - lullPrevTotal;
    double ldIdle = lullIdleSecondRead - lullPrevIdle;

    printf("SG > CPU Usage: %.2f%%\n", (1.0 - ldIdle / ldTotal) * 100.0);
} /* End of SG_PrintCpuUsage function */