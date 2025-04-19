#ifndef _SG_MAINPROCESS_H_
#define _SG_MAINPROCESS_H_

/********************************************************
*                    INCLUDE SECTION                    *
********************************************************/
#include "main.h"
#include "MP_ConnectionManager.h"
#include "MP_DataManager.h"

/********************************************************
*                     DEFINE SECTION                    *
********************************************************/

/********************************************************
*                    TYPEDEF SECTION                    *
********************************************************/

/********************************************************
*         GLOBAL VARIABLE DECLARATION SECTION           *
********************************************************/
extern pthread_mutex_t gsShareDataMutex;

/********************************************************
*          FUNCTION DECLARATION SECTION                 *
********************************************************/
extern void MP_main();

#endif /* _SG_MAINPROCESS_H_ */