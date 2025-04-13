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
#define DS_QUEUE_NAME  "/bin/sg_msg_queue"
#define DS_MAX_SIZE    1024
#define DS_MSG_STOP    "exit"

#define DS_SENSOR_DATA_FORMAT   "%d;%s;%d;%d"

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

    gsMessageQueueDes = mq_open(DS_QUEUE_NAME, O_CREAT | O_RDWR, 0666, &attr);
    if (gsMessageQueueDes == (mqd_t)-1)
    {
        perror("mq_open");
        return -1; // Return fail
    }
    else
    {
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
 * Return value: None
 * Functionality: Push the message to queue with expected priority
 * Requirement: 
 ******************************************************************************************/
int DS_QueuePush(SensorData_t sensorData, unsigned int prio)
{
    char lpBuf[DS_MAX_SIZE] = {0};

    int liNodeID = 0;
    char lpAddress[INET_ADDRSTRLEN] = {0};
    int liPort = 0;
    int liTemperature = 0;

    // Get the data
    liNodeID = sensorData.SensorNodeID;
    strlcpy(lpAddress, inet_ntoa(sensorData.address.sin_addr), INET_ADDRSTRLEN);
    liPort = ntohs(sensorData.address.sin_port);
    liTemperature = sensorData.Temperature;

    // Prepare the buffer
    sprintf(lpBuf, DS_SENSOR_DATA_FORMAT, liNodeID, lpAddress, liPort, liTemperature);

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
 *      0: Getting successfully
 * Functionality: Get the message from queue with expected priority
 * Requirement: 
 ******************************************************************************************/
int DS_QueueGet(SensorData_t *sensorData, unsigned int prio)
{
    char lpBuf[DS_MAX_SIZE] = {0};

    int liNodeID = 0;
    char lpAddress[INET_ADDRSTRLEN] = {0};
    int liPort = 0;
    int liTemperature = 0;

    unsigned int lulPriority;

    ssize_t llBytes_read = mq_receive(gsMessageQueueDes, lpBuf, DS_MAX_SIZE, &lulPriority);
    if (llBytes_read >= 0)
    {
        // Check if the priority is matched with expected
        if (lulPriority == prio)
        {
            printf("Queue received, msg = \"%s\", priority = %d\n", lpBuf, prio);
            sscanf (lpBuf, DS_SENSOR_DATA_FORMAT, &liNodeID, lpAddress, &liPort, &liTemperature);

            // Set the data
            sensorData->SensorNodeID = liNodeID;
            sensorData->address.sin_addr.s_addr = inet_addr(lpAddress);
            sensorData->address.sin_port = htons(liPort);
            sensorData->Temperature = liTemperature;

            return 0;
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
        perror("mq_receive");
        return -1;
    }
} /* End of function DS_QueueGet */