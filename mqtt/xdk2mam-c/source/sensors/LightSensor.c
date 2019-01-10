

/* own header files */
#include "XDKAppInfo.h"
#undef BCDS_MODULE_ID
#define BCDS_MODULE_ID XDK_APP_MODULE_ID_LIGHT_SENSOR
#include "LightSensor.h"
#include "XdkSensorHandle.h"

/* additional interface header files */
#include "BCDS_Basics.h"
#include "BCDS_LightSensor.h"
#include "BCDS_Assert.h"
#include "BCDS_CmdProcessor.h"
#include "FreeRTOS.h"
#include "timers.h"


/* system header files */
#include <stdio.h>

/* local prototypes ********************************************************* */

/* constant definitions ***************************************************** */
#define THREESECONDDELAY                UINT32_C(3000)      /**< three seconds delay is represented by this macro */
#define TIMERBLOCKTIME                  UINT32_C(0xffff)    /**< Macro used to define blocktime of a timer */
#define ZERO                            UINT32_C(0)          /**< default value */
#define ONE                             UINT8_C(1)          /**< default value */
#define DEFERRED_CALLBACK               UINT8_C(1)          /**< indicate deferred callback */
#define REALTIME_CALLBACK               UINT8_C(0)          /**< indicate real time callback */
#define UPPER_THRESHOLD_VALUE           UINT32_C(0x5a)      /**< upper threshold value */
#define LOWER_THRESHOLD_VALUE           UINT32_C(0x2a)      /**< lower threshold value */
#define THRESHOLD_TIMER_VALUE           UINT32_C(0X05)      /**< threshold timer value */
#define NIBBLE_SIZE                     UINT8_C(4)          /**< size of nibble */
#define MASK_NIBBLE                     UINT8_C(0x0F)       /**< macro to mask nibble */
#define EVENNUMBER_IDENTIFIER           UINT8_C(2)          /**< macro to identify even numbers */
#define APP_CALLBACK_DATA               UINT32_C(100)       /**< macro to indicate application time callback data for demo*/
#define FAILURE                         UINT32_C(1)
#define TIMER_NOT_ENOUGH_MEMORY            (-1L)/**<Macro to define not enough memory error in timer*/
#define TIMER_AUTORELOAD_ON             UINT32_C(1)             /**< Auto reload of timer is enabled*/


xTimerHandle printTimerHandle;




char* processLightSensorData(void * param1, uint32_t param2)
{
    BCDS_UNUSED(param1);
    BCDS_UNUSED(param2);
    Retcode_T returnValue = (Retcode_T) RETCODE_FAILURE;
    char  *buffer = calloc(255, sizeof(char));


    uint32_t milliLuxData = UINT32_C(0);
    /* read Raw sensor data */

    /* read sensor data in milli lux*/
    returnValue = LightSensor_readLuxData(xdkLightSensor_MAX44009_Handle, &milliLuxData);
    if (returnValue != RETCODE_OK)
    {
        printf("lightsensorReadInMilliLux Failed\n\r");
    }
    else
    {
        sprintf(buffer,"{\"sensor\":\"Light\",\"data\":[{\"milliLux\":\"%d\"}]}",
							(unsigned int) milliLuxData);
    }



    return (char*)buffer;
}


Retcode_T lightsensorInit(void)
{
    Retcode_T returnValue = (Retcode_T) RETCODE_FAILURE;


    /*initialize lightsensor*/
    returnValue = LightSensor_init(xdkLightSensor_MAX44009_Handle);
    if (RETCODE_OK == returnValue)
    {
    	printf("Light Sensor initialization Success\n\r");

    }
    else
    {
        printf("lightsensorInit Failed\n\r");
    }


    return returnValue;
}

Retcode_T lightsensorDeinit(void)
{
    Retcode_T returnValue = (Retcode_T) RETCODE_FAILURE;

    returnValue = LightSensor_deInit(xdkLightSensor_MAX44009_Handle);
    if (RETCODE_OK == returnValue)
    {
        printf("lightsensorDeinit Success\n\r");
    }
    else
    {
        printf("lightsensorDeinit Failed\n\r");
    }
    return returnValue;
}

/**@} */
/** ************************************************************************* */
