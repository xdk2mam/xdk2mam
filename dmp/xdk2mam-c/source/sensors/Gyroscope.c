

/* own header files */
#include "XDKAppInfo.h"
#undef BCDS_MODULE_ID
#define BCDS_MODULE_ID XDK_APP_MODULE_ID_GYROSCOPE
#include "Gyroscope.h"
#include "XdkSensorHandle.h"

/* additional interface header files */
#include "BCDS_Basics.h"
#include "BCDS_Gyroscope.h"
#include "BCDS_Assert.h"
#include "BCDS_CmdProcessor.h"
#include "FreeRTOS.h"
#include "timers.h"


/* system header files */
#include <stdio.h>

/* local prototypes ********************************************************* */

/* constant definitions ***************************************************** */
#define THREESECONDDELAY                UINT32_C(3000)     /**< three seconds delay is represented by this macro */
#define TIMERBLOCKTIME                  UINT32_C(0xffff)   /**< Macro used to define blocktime of a timer */
#define ZERO                            UINT32_C(0)         /**< default value */
#define ONE                             UINT8_C(1)         /**< default value */
#define TIMER_NOT_ENOUGH_MEMORY            (-1L)           /**<Macro to define not enough memory error in timer*/
#define TIMER_AUTORELOAD_ON             UINT32_C(1)        /**< Auto reload of timer is enabled*/

/* local variables ********************************************************** */

/* global variables ********************************************************* */

/* variable to store timer handle*/
xTimerHandle printTimerHandle;

char* processGyroData(void * param1, uint32_t param2)
{
    BCDS_UNUSED(param1);
    BCDS_UNUSED(param2);

    Retcode_T advancedApiRetValue = (Retcode_T) RETCODE_FAILURE;

    Gyroscope_XyzData_T getMdegData = { INT32_C(0), INT32_C(0), INT32_C(0) };
    /* read Raw sensor data */


    char  *buffer = calloc(255, sizeof(char));

    /* read sensor data in milli Deg*/
    advancedApiRetValue = Gyroscope_readXyzDegreeValue(xdkGyroscope_BMG160_Handle, &getMdegData);
    if (RETCODE_OK == advancedApiRetValue)
    {


        sprintf(buffer,"{\"sensor\":\"Gyroscope\",\"data\":[{\"x\":\"%ld\"},{\"y\":\"%ld\"},{\"z\":\"%ld\"}]}",
							(long int) getMdegData.xAxisData, (long int) getMdegData.yAxisData, (long int) getMdegData.zAxisData);

    }
    else
    {
        printf("GyrosensorReadInMilliDeg Failed\n\r");
    }
    return (char*)buffer;
}


Retcode_T gyroscopeSensorInit(void)
{
    Retcode_T returnValue = (Retcode_T) RETCODE_FAILURE;


    /*initialize Gyro sensor*/
    returnValue = Gyroscope_init(xdkGyroscope_BMG160_Handle);

    if (RETCODE_OK == returnValue)
    {
    	printf("Gyroscope Sensor initialization Success\n\r");
    }
    else
    {
        printf("GyroInit Failed\n\r");
    }


    return returnValue;
}

Retcode_T gyroscopeSensorDeinit(void)
{
    Retcode_T returnValue = (Retcode_T) RETCODE_FAILURE;
    returnValue = Gyroscope_deInit(xdkGyroscope_BMG160_Handle);
    if (RETCODE_OK == returnValue)
    {
        printf("gyroscopeSensor Deinit Success\n\r");
    }
    else
    {
        printf("gyroscopeSensor Deinit Failed\n\r");
    }
    return returnValue;
}
