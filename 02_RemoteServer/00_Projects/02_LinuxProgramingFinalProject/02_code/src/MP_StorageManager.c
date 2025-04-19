/********************************************************
*                    INCLUDE SECTION                    *
********************************************************/
#include <sqlite3.h>

#include "main.h"
#include "SG_MainProcess.h"
#include "MP_DataSharing.h"
#include "MP_StorageManager.h"

/********************************************************
*                     DEFINE SECTION                    *
********************************************************/
#define SM_DB_FILE_PATH                     "./db/StorageManager.db"
#define SM_MAX_OPEN_DB_TIMES                3
#define SM_MAX_QUERY_LENGTH                 1024

#define SM_SQL_INSERTORUPDATE_QUERY_FORMAT "INSERT INTO Nodes (Id, Address, Port, Temperature) VALUES (%d, '%s', %d, %lf) ON CONFLICT(Id) DO UPDATE SET Address=excluded.Address, Port=excluded.Port, Temperature=excluded.Temperature;"

/********************************************************
*                    TYPEDEF SECTION                    *
********************************************************/

/********************************************************
*                 CONSTANCE SECTION                     *
********************************************************/
const char *gpSqlSropTableQuery = "DROP TABLE IF EXISTS Nodes";
const char *gpSqlCreateQuery = "CREATE TABLE IF NOT EXISTS Nodes(Id INT PRIMARY KEY, Address TEXT, Port INT, Temperature REAL);";

/********************************************************
*          FUNCTION DECLARATION SECTION                 *
********************************************************/

/********************************************************
*         GLOBAL VARIABLE DECLARATION SECTION           *
********************************************************/
sqlite3 *gsDataBase;

int giSMRequesState;

/*******************************************************************************************
 * Function name: DM_MainThread
 * Functionality: The thread function of Data Manager
 * Requirement: 
 ******************************************************************************************/
void *SM_MainThread(void * argv)
{
    SensorData_t lsReadData;
    int liReturnValue;
    char *lpErrMsg = NULL;

    char lpQueryBuffer[SM_MAX_QUERY_LENGTH];
    memset(lpQueryBuffer, 0, SM_MAX_QUERY_LENGTH);

    giSMRequesState = 1;

    int liOpenDbTimes = 0;
    do
    {
        // Open (or create) a database
        liReturnValue = sqlite3_open(SM_DB_FILE_PATH, &gsDataBase);
        if (SQLITE_OK != liReturnValue)
        {
            liOpenDbTimes++;

            // Exit the thread if database can't open after SM_MAX_OPEN_DB_TIMES times
            if (SM_MAX_OPEN_DB_TIMES <= liOpenDbTimes)
            {
                printf("Storage Manager > Cannot open DB: %s\n", sqlite3_errmsg(gsDataBase));
                pthread_exit(NULL); // Exit the thread (T.B.D, try to send out something to force other thread exit)
            }
        }
        else
        {
            // Do nothing
        }
    } while (SQLITE_OK != liReturnValue);

    // Create a table in database
    liReturnValue = sqlite3_exec(gsDataBase, gpSqlCreateQuery, 0, 0, &lpErrMsg);
    if (SQLITE_OK != liReturnValue)
    {
        printf("Storage Manager > SQL error: %s\n", lpErrMsg);
        sqlite3_free(lpErrMsg);
    }
    else
    {
        printf("Storage Manager > SQL table created successfully!\n");
    }

    // Loop for reading the queue
    while (giSMRequesState == 1)
    {
        // Getting the Queue
        pthread_mutex_lock(&gsShareDataMutex);
        liReturnValue = DS_QueueGet(&lsReadData, DS_STORAGE_MANAGER_PRIO);
        pthread_mutex_unlock(&gsShareDataMutex);

        // Check if data is come
        if (liReturnValue >= 0)
        {
            #if (SM_DEBUG_PRINT_ENABLE == 1)
            printf("\nStorage Manager > Read data = %f\n", lsReadData.Temperature);
            printf("Storage Manager > Writing to DataBase...\n\n");
            #endif

            // Insert data or update the data
            sprintf(lpQueryBuffer, SM_SQL_INSERTORUPDATE_QUERY_FORMAT,
                lsReadData.SensorNodeID,
                inet_ntoa(lsReadData.address.sin_addr),
                ntohs(lsReadData.address.sin_port),
                lsReadData.Temperature
            );
            // Execute the query
            liReturnValue = sqlite3_exec(gsDataBase, lpQueryBuffer, 0, 0, &lpErrMsg);
            if (SQLITE_OK != liReturnValue)
            {
                printf("Storage Manager > SQL error (INSERT): %s\n", lpErrMsg);
                sqlite3_free(lpErrMsg);
            }
            else
            {
                #if (SM_DEBUG_PRINT_ENABLE == 1)
                printf("Storage Manager > Records inserted successfully\n");
                #endif
            }
        }
    }
    // Close the database
    sqlite3_close(gsDataBase);
}

/*******************************************************************************************
 * Function name: SM_Exit
 * Functionality: Request to exit the Storage Manager
 * Requirement: 
 ******************************************************************************************/
void SM_Exit()
{
    printf("Storage Manager > Exiting Storage Manager ...\n");
    giSMRequesState = 0;
} /* End of SM_Exit function */