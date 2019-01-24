

/* own header files */
#include "XDKAppInfo.h"
#undef BCDS_MODULE_ID
#define BCDS_MODULE_ID XDK_APP_MODULE_ID_ACOUSTIC
#include "Magnetometer.h"
#include "XdkSensorHandle.h"

/* additional interface header files */
#include "BCDS_Basics.h"
#include "XDK_NoiseSensor.h"
#include "BCDS_Assert.h"
#include "BCDS_CmdProcessor.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "math.h"


/* system header files */
#include <stdio.h>

/* constant definitions ***************************************************** */
#define TIMERDELAY       UINT32_C(3000)          /**< three seconds delay is represented by this macro */
#define TIMERBLOCKTIME   UINT32_C(0xffff)        /** Macro used to define blocktime of a timer*/
#define ZEROVALUE        UINT32_C(0x00)          /** Macro used to define default value*/
#define TIMER_NOT_ENOUGH_MEMORY            (-1L)/**<Macro to define not enough memory error in timer*/
#define TIMER_AUTORELOAD_ON             UINT32_C(1)             /**< Auto reload of timer is enabled*/



const float aku340CR = pow(10,(-38/20));


char* processAcousticData(void * param1, uint32_t param2)
{
    BCDS_UNUSED(param1);
    BCDS_UNUSED(param2);
    char  *buffer = calloc(255, sizeof(char));

    float acousticData;

	if (RETCODE_OK == NoiseSensor_ReadRmsValue(&acousticData,10U)) {
		sprintf(buffer,"{\"sensor\":\"Acoustic\",\"data\":[{\"mp\":\"%f\"}]}",
		    			acousticData/aku340CR);
	}
    else
    {
        printf("Acoustic Sensor read FAILED\n\r");
    }


    return (char*)buffer;
}


Retcode_T acousticSensorInit(void)
{
    Retcode_T acousticReturnValue = (Retcode_T) RETCODE_FAILURE;

    if (RETCODE_OK == NoiseSensor_Setup(22050U))
    {
    	acousticReturnValue = NoiseSensor_Enable();
    	if(RETCODE_OK == acousticReturnValue )
            printf("Acoustic Sensor initialization Success\n\r");
    	else
            printf("Acoustic initialization FAILED\n\r");
    }
    else
    {
        printf("Acoustic initialization FAILED\n\r");
    }
    return acousticReturnValue;
}

/**@} */
