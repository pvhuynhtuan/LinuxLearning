#ifndef _MP_DATASHARING_H_
#define _MP_DATASHARING_H_

/********************************************************
*                    INCLUDE SECTION                    *
********************************************************/
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/if.h>

/********************************************************
*                     DEFINE SECTION                    *
********************************************************/
#define DS_DATA_MANAGER_PRIO        1
#define DS_STORAGE_MANAGER_PRIO     2

#define DS_TEST_ENABLE              0

/********************************************************
*                    TYPEDEF SECTION                    *
********************************************************/
typedef struct
{
    struct sockaddr_in address;
    int SensorNodeID;
    double Temperature;
} SensorData_t;

/********************************************************
*         GLOBAL VARIABLE DECLARATION SECTION           *
********************************************************/

/********************************************************
*          FUNCTION DECLARATION SECTION                 *
********************************************************/
extern int DS_QueueInit();
extern void DS_Close();
extern int DS_QueuePush(SensorData_t sensorData, unsigned int prio);
extern int DS_QueueGet(SensorData_t *sensorData, unsigned int prio);

#endif /* _MP_DATASHARING_H_ */