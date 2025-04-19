/********************************************************
*                    INCLUDE SECTION                    *
********************************************************/
// include frame work header
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>

// Include user's header
#include "main.h"
#include "MP_DataSharing.h"

/********************************************************
*                     DEFINE SECTION                    *
********************************************************/
#define DS_QUEUE_NAME  "/sg_msg_queue"
// #define DS_MQ_MODE (S_IRUSR | S_IWUSR ) 
#define DS_MAX_SIZE    1024
#define DS_MSG_STOP    "exit"

#define DS_SENSOR_DATA_FORMAT   "%d %s %d %lf"

/********************************************************
*                    TYPEDEF SECTION                    *
********************************************************/

/********************************************************
*                 CONSTANCE SECTION                     *
********************************************************/

/********************************************************
*          FUNCTION DECLARATION SECTION                 *
********************************************************/

/********************************************************
*         GLOBAL VARIABLE DECLARATION SECTION           *
********************************************************/
mqd_t gsMessageQueueDes;

/*******************************************************************************************
 * Function name: DS_QueueInit
 * Functionality: Initialize the message queue
 ******************************************************************************************/
int DS_QueueInit()
{
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = DS_MAX_SIZE;
    attr.mq_curmsgs = 0;

    mq_unlink(DS_QUEUE_NAME);  // Ensure clean state before starting

    gsMessageQueueDes = mq_open(DS_QUEUE_NAME, O_CREAT | O_RDWR | O_NONBLOCK, 0666, &attr);
    if (gsMessageQueueDes == (mqd_t)-1)
    {
        perror("Data Sharing > mq_open");
        return -1; // Return fail
    }
    else
    {
        printf("Data Sharing > Queue init success!\n");
        return 0; // Return successful
    }
} /* End of function DS_QueueInit */

/*******************************************************************************************
 * Function name: DS_Close
 * Functionality: Close the message queue
 ******************************************************************************************/
void DS_Close()
{
    mq_close(gsMessageQueueDes);
    mq_unlink(DS_QUEUE_NAME);
} /* End of function DS_Close */

/*******************************************************************************************
 * Function name: DS_QueuePush
 * Parameter:
 *      sensorData: The node data (struct)
 *      prio: Priority of message
 * Return value:
 *      -1: Sending failed
 *       0: Sending successfully
 * Functionality: Push the message to queue with expected priority
 * Requirement: 
 ******************************************************************************************/
int DS_QueuePush(SensorData_t sensorData, unsigned int prio)
{
    char lpBuf[DS_MAX_SIZE] = {0};
    // Zero out the structure
    memset(lpBuf, 0, DS_MAX_SIZE);

    int liNodeID = 0;
    char lpAddress[INET_ADDRSTRLEN] = {0};
    int liPort = 0;
    double ldTemperature;

    // Get the data
    liNodeID = sensorData.SensorNodeID;
    strlcpy(lpAddress, inet_ntoa(sensorData.address.sin_addr), INET_ADDRSTRLEN);
    liPort = ntohs(sensorData.address.sin_port);
    ldTemperature = sensorData.Temperature;

    // Prepare the buffer
    sprintf(lpBuf, DS_SENSOR_DATA_FORMAT, liNodeID, lpAddress, liPort, ldTemperature);

    // Test print
    #if (DS_TEST_ENABLE == 1)
    printf("Data Sharing > Pushed Data: \n");
    printf("\tliNodeID = %d\n", liNodeID);
    printf("\tlpAddress = %s\n", lpAddress);
    printf("\tliPort = %d\n", liPort);
    printf("\tldTemperature = %lf\n", ldTemperature);
    printf("\tMessage: %s\n", lpBuf);
    printf("\tPriority = %d\n", prio);
    #endif

    //Send the data via Queue
    return mq_send(gsMessageQueueDes, lpBuf, strlen(lpBuf) + 1, prio);
} /* End of function DS_QueuePush */

/*******************************************************************************************
 * Function name: DS_QueueGet
 * Parameter:
 *      sensorData: The returned node data (struct)
 *      prio: expecting priority of message
 * Return value:
 *      -1: Getting failed or the message priority is unmatch
 *       number: Getting successfully and return the number of bytes
 * Functionality: Get the message from queue with expected priority
 * Requirement: 
 ******************************************************************************************/
int DS_QueueGet(SensorData_t *sensorData, unsigned int prio)
{
    char lpBuf[DS_MAX_SIZE] = {0};
    // Zero out the structure
    memset(lpBuf, 0, DS_MAX_SIZE);

    int liNodeID = 0;
    char lpAddress[INET_ADDRSTRLEN] = {0};
    int liPort = 0;
    double ldTemperature;

    unsigned int lulPriority;

    ssize_t llBytes_read = mq_receive(gsMessageQueueDes, lpBuf, DS_MAX_SIZE, &lulPriority);
    if (llBytes_read >= 0)
    {
        // Check if the priority is matched with expected priority
        if (lulPriority == prio)
        {
            // printf("Data Sharing > Queue received, msg = \"%s\", priority = %d\n", lpBuf, prio);
            sscanf(lpBuf, DS_SENSOR_DATA_FORMAT, &liNodeID, lpAddress, &liPort, &ldTemperature);

            // Set the data
            sensorData->SensorNodeID = liNodeID;
            sensorData->address.sin_addr.s_addr = inet_addr(lpAddress);
            sensorData->address.sin_port = htons(liPort);
            sensorData->Temperature = ldTemperature;

            // Test print
            #if (DS_TEST_ENABLE == 1)
            printf("Data Sharing > Get Data: \n");
            printf("\tliNodeID = %d\n", liNodeID);
            printf("\tlpAddress = %s\n", lpAddress);
            printf("\tliPort = %d\n", liPort);
            printf("\tldTemperature = %lf\n", ldTemperature);
            printf("\tMessage: %s\n", lpBuf);
            printf("\tlulPriority = %d\n", lulPriority);
            #endif

            return llBytes_read;
        }
        else
        {
            // Put the message back to the queue
            mq_send(gsMessageQueueDes, lpBuf, llBytes_read, lulPriority);
            return -1;
        }
    } 
    else 
    {
        //perror("mq_receive");
        return -1;
    }
} /* End of function DS_QueueGet */
