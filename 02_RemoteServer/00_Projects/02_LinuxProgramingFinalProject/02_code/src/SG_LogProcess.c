#include <stdio.h>
#include <unistd.h>

#include "SG_LogProcess.h"

/********************************************************
*                 CONSTANCE SECTION                     *
********************************************************/

/********************************************************
*         GLOBAL VARIABLE DECLARATION SECTION           *
********************************************************/

void LP_main()
{
    while(1)
    {
        printf("Log process: This is the main function\n");
        sleep(3);
    }
}
