#ifndef _MP_CONNECTIONMANAGER_H_
#define _MP_CONNECTIONMANAGER_H_

/********************************************************
*                    INCLUDE SECTION                    *
********************************************************/
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/if.h>

/********************************************************
*                     DEFINE SECTION                    *
********************************************************/
#define CM_IP_BUFFER_SIZE 15

#define CM_MAX_MYIP_SIZE   5
#define CM_MAX_CONNECT_IP_SIZE   50

#define CM_MAX_RECEIVE_LEN 100
#define CM_MAX_TRANSMIT_LEN 100

/* Configuration (0 = disable; 1 = enable) */
#define CM_ENABLE_CONNECT_FUNCTION 0
#define CM_ENABLE_SEND_FUNCTION 0

/********************************************************
*                    TYPEDEF SECTION                    *
********************************************************/
typedef struct
{
    int socket_fd;
    struct sockaddr_in address;
} Connection_t;

/********************************************************
*         GLOBAL VARIABLE DECLARATION SECTION           *
********************************************************/
extern int volatile giCMRequesState;

/********************************************************
*          FUNCTION DECLARATION SECTION                 *
********************************************************/

extern void *CM_MainThread(void * argv);
extern void CM_List();
extern void CM_Exit();

#endif /* _MP_CONNECTIONMANAGER_H_ */