#ifndef _MP_DATASHARING_H_
#define _MP_DATASHARING_H_

/********************************************************
*                    INCLUDE SECTION                    *
********************************************************/

/********************************************************
*                     DEFINE SECTION                    *
********************************************************/
#define DS_DATA_MANAGER_PRIO        1
#define DS_STORAGE_MANAGER_PRIO     2

/********************************************************
*                    TYPEDEF SECTION                    *
********************************************************/

/********************************************************
*         GLOBAL VARIABLE DECLARATION SECTION           *
********************************************************/

/********************************************************
*          FUNCTION DECLARATION SECTION                 *
********************************************************/
extern int DS_QueueInit();
extern void DS_Close();
extern int DS_QueuePush(char *buf, unsigned int prio);
extern int DS_QueueGet(char *buf, unsigned int prio);

#endif /* _MP_DATASHARING_H_ */