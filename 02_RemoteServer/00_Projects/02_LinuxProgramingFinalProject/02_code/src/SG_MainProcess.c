#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "SG_MainProcess.h"
#include "MP_DataSharing.h"
#include "MP_StorageManager.h"

/********************************************************
*                 CONSTANCE SECTION                     *
********************************************************/

/********************************************************
*         GLOBAL VARIABLE DECLARATION SECTION           *
********************************************************/
pthread_t gsConnectionManagerTID;
pthread_t gsDataManagerTID;
pthread_t gsStorageManagerTID;

pthread_mutex_t gsShareDataMutex = PTHREAD_MUTEX_INITIALIZER;

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

    if(-1 == DS_QueueInit())
    {
        exit(EXIT_FAILURE);
    }

    pthread_create(&gsConnectionManagerTID, NULL, CM_MainThread, &port);
    pthread_create(&gsDataManagerTID, NULL, DM_MainThread, NULL);
    pthread_create(&gsStorageManagerTID, NULL, SM_MainThread, NULL);

    pthread_join(gsConnectionManagerTID, NULL);
    pthread_join(gsDataManagerTID, NULL);
    pthread_join(gsStorageManagerTID, NULL);

    DS_Close();
    pthread_mutex_destroy(&gsShareDataMutex);
    exit(EXIT_SUCCESS);
}


void MP_Signal_Handler_SIGTSTP(int num)
{
    // Trigger the close condition for Connection Manager
    CM_Exit();
    DM_Exit();
    SM_Exit();
    printf("Main Process > Closing the Connection Manager!\n");
}
