#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "main.h"
#include "SG_MainProcess.h"

/********************************************************
*                 CONSTANCE SECTION                     *
********************************************************/

/********************************************************
*         GLOBAL VARIABLE DECLARATION SECTION           *
********************************************************/
pthread_t gsConnectionManagerTID;
pthread_t gsDataManagerTID;
pthread_t gsStorageManagerTID;

/********************************************************
*          FUNCTION DECLARATION SECTION                 *
********************************************************/
void MP_Signal_Handler_SIGTSTP(int num);

void MP_main(int port)
{
    if(signal(SIGTSTP, MP_Signal_Handler_SIGTSTP) == SIG_ERR)
    {
        fprintf(stderr, "Main Process > Can't handle SIGTSTP\n");
        exit(EXIT_FAILURE);
    }

    pthread_create(&gsConnectionManagerTID, NULL, CM_MainThread, &port);

    pthread_join(gsConnectionManagerTID, NULL);
    exit(EXIT_SUCCESS);
}


void MP_Signal_Handler_SIGTSTP(int num)
{
    // Trigger the close condition for Connection Manager
    giCMRequesState = 0;
    printf("Main Process > Closing the Connection Manager!\n");
}
