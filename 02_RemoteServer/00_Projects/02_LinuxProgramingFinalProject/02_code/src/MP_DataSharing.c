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
 *      buf: The message to be sent
 *      prio: Priority of message
 * Return value: None
 * Functionality: Push the message to queue with expected priority
 * Requirement: 
 ******************************************************************************************/
int DS_QueuePush(char *buf, unsigned int prio)
{
    return mq_send(gsMessageQueueDes, buf, strlen(buf) + 1, prio);
} /* End of function DS_QueuePush */

/*******************************************************************************************
 * Function name: DS_QueueGet
 * Parameter:
 *      buf: the buffer to store the message
 *      prio: expecting priority of message
 * Return value:
 *      -1: Getting failed or the message priority is unmatch
 *      0: Getting successfully
 * Functionality: Get the message from queue with expected priority
 * Requirement: 
 ******************************************************************************************/
int DS_QueueGet(char *buf, unsigned int prio)
{
    unsigned int lulPriority;
    ssize_t llBytes_read = mq_receive(gsMessageQueueDes, buf, DS_MAX_SIZE, &lulPriority);
    if (llBytes_read >= 0)
    {
        // Check if the priority is matched with expected
        if (lulPriority == prio)
        {
            printf("Queue received, msg = \"%s\", priority = %d\n", buf, prio);
            return 0;
        }
        else
        {
            // Put the message back to the queue
            mq_send(gsMessageQueueDes, buf, llBytes_read, lulPriority);
            return -1;
        }
    } 
    else 
    {
        perror("mq_receive");
        return -1;
    }
} /* End of function DS_QueueGet */