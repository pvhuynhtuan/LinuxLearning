/********************************************************
*                    INCLUDE SECTION                    *
********************************************************/
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/if.h>

#include "main.h"
#include "SG_MainProcess.h"
#include "MP_DataManager.h"
#include "MP_DataSharing.h"

/********************************************************
*                     DEFINE SECTION                    *
********************************************************/
#define DM_MAX_PROCESSING_NODE          1024
#define DM_MAX_PROCESSING_DATA          100

#define DM_MIN_WARNING_TEMPER           5
#define DM_MAX_WARNING_TEMPER           30

/********************************************************
*                    TYPEDEF SECTION                    *
********************************************************/

/********************************************************
*                 CONSTANCE SECTION                     *
********************************************************/

/********************************************************
*          FUNCTION DECLARATION SECTION                 *
********************************************************/
double DM_CalAverageTemper(int nodeId);
void DM_AppendNodeData(int nodeId, double temper);

/********************************************************
*         GLOBAL VARIABLE DECLARATION SECTION           *
********************************************************/
double gpNodeTemperatureBuffer[DM_MAX_PROCESSING_NODE][DM_MAX_PROCESSING_DATA];
int giNumberOfNode;
int gpNumNodeData[DM_MAX_PROCESSING_NODE];

int volatile giDMRequesState;

/*******************************************************************************************
 * Function name: DM_MainThread
 * Functionality: The thread function of Data Manager
 * Requirement: 
 ******************************************************************************************/
void *DM_MainThread(void * argv)
{
    SensorData_t lsReadData;
    int liReturnValue = 0;

    double ldAverageTemper = 0;

    // Clear the data on buffer
    memset(gpNodeTemperatureBuffer, 0, sizeof(double) * DM_MAX_PROCESSING_NODE * DM_MAX_PROCESSING_DATA);

    // Start the condition to loop this thread
    giDMRequesState = 1;

    while(giDMRequesState == 1)
    {
        pthread_mutex_lock(&gsShareDataMutex);
        liReturnValue = DS_QueueGet(&lsReadData, DS_DATA_MANAGER_PRIO);
        pthread_mutex_unlock(&gsShareDataMutex);

        if (liReturnValue >= 0)
        {
            DM_AppendNodeData(lsReadData.SensorNodeID, lsReadData.Temperature);
            ldAverageTemper = DM_CalAverageTemper(lsReadData.SensorNodeID);

            #if (DM_DEBUG_PRINT_ENABLE == 1)
            printf("Data Manager > Read data = %f\n\n", lsReadData.Temperature);
            printf("Data Manager > Average temperature of node %d = %f\n\n",lsReadData.SensorNodeID, ldAverageTemper);
            #endif

            if (ldAverageTemper <= DM_MIN_WARNING_TEMPER)
            {
                printf(RED "Data Manager > The sensor node with ID <%d> reports it’s too cold (running avg temperature = %f\n" RESET "\n",
                    lsReadData.SensorNodeID, ldAverageTemper);
            }
            else if (ldAverageTemper >= DM_MAX_WARNING_TEMPER)
            {
                printf(RED "Data Manager > The sensor node with ID <%d> reports it’s too hot (running avg temperature = %f\n" RESET "\n",
                    lsReadData.SensorNodeID, ldAverageTemper);
            }
        }
        else
        {
            // Do nothing
        }
    }
}

/*******************************************************************************************
 * Function name: DM_Exit
 * Functionality: Termindate all connection
 * Requirement: 
 ******************************************************************************************/
void DM_Exit()
{
    printf("Data Manager > Exiting Data Manager ...\n");
    giDMRequesState = 0;
} /* End of DM_Exit function */

/*******************************************************************************************
 * Function name: DM_CalAverageTemper
 * Input:
 *      nodeID: ID of node intended to calculate
 * Output: The avarage value of temperature
 * Functionality: Calculate the average temperature of indicated node ID
 * Requirement: 
 ******************************************************************************************/
double DM_CalAverageTemper(int nodeId)
{
    double ldReturnValue = 0;

    for (int index = 0; index < gpNumNodeData[nodeId]; index++)
    {
        ldReturnValue += gpNodeTemperatureBuffer[nodeId][index]/((double)gpNumNodeData[nodeId]);
    }
    return ldReturnValue;
}

/*******************************************************************************************
 * Function name: DM_CalAverageTemper
 * Input:
 *      nodeID: ID of node intended to calculate
 *      data: temperature data
 * Functionality: Calculate the average temperature of indicated node ID
 * Requirement: 
 ******************************************************************************************/
void DM_AppendNodeData(int nodeId, double temper)
{
    #if (DM_DEBUG_PRINT_ENABLE == 1)
    printf("Data Manager > Appending node ID = %d\n", nodeId);
    #endif
    // Check that the node is newly added
    if (nodeId <= DM_MAX_PROCESSING_NODE)
    {
        if (gpNumNodeData[nodeId] < DM_MAX_PROCESSING_DATA)
        {
            gpNodeTemperatureBuffer[nodeId][gpNumNodeData[nodeId]] = temper;
            gpNumNodeData[nodeId]++;
        }
        else if (gpNumNodeData[nodeId] == DM_MAX_PROCESSING_DATA)
        {
            // Buffer is full, remove the first data and shift all other by 1
            for (int index = 0; index < (DM_MAX_PROCESSING_DATA - 1); index++)
            {
                // Shift all other data by 1
                gpNodeTemperatureBuffer[nodeId][index] = gpNodeTemperatureBuffer[nodeId][index + 1];
            }
            gpNodeTemperatureBuffer[nodeId][DM_MAX_PROCESSING_DATA - 1] = temper;
        }
        
        // To prevent the overflow buffer by random failure, always keep the count equal to max
        if (gpNumNodeData[nodeId] > DM_MAX_PROCESSING_DATA)
        {
            gpNumNodeData[nodeId] = DM_MAX_PROCESSING_DATA;
        }
    }
}