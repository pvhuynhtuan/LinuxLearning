#ifndef _MP_CONNECTIONMANAGER_H_
#define _MP_CONNECTIONMANAGER_H_

/********************************************************
*                    INCLUDE SECTION                    *
********************************************************/

/********************************************************
*                     DEFINE SECTION                    *
********************************************************/
#define CM_IP_BUFFER_SIZE 15

#define CM_MAX_MYIP_SIZE   5
#define CM_MAX_CONNECT_IP_SIZE   50

#define CM_MAX_RECEIVE_LEN 100
#define CM_MAX_TRANSMIT_LEN 100

/* Configuration (0 = disable; 1 = enable) */
#define CM_ENABLE_CONNECT_FUNCTION          0
#define CM_ENABLE_SEND_FUNCTION             0
#define CM_LOG_PRINT_ENABLE                 0
#define CM_SEND_DATASHARE_TO_DM_ENABLE      1   // Enable send data to Data Manager thread
#define CM_SEND_DATASHARE_TO_SM_ENABLE      1   // Enable send data to Storage Manager thread

#define CM_LOG_WRITER_ENABLE                1   // Enable the log data send to Log Process for writing to the file

/********************************************************
*                    TYPEDEF SECTION                    *
********************************************************/


/********************************************************
*         GLOBAL VARIABLE DECLARATION SECTION           *
********************************************************/

/********************************************************
*          FUNCTION DECLARATION SECTION                 *
********************************************************/

extern void *CM_MainThread(void * argv);
extern void CM_List();
extern void CM_Exit();

#endif /* _MP_CONNECTIONMANAGER_H_ */