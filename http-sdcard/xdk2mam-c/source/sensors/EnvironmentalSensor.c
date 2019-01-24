
/* module includes ********************************************************** */

/* own header files */
#include "XDKAppInfo.h"
#undef BCDS_MODULE_ID
#define BCDS_MODULE_ID XDK_APP_MODULE_ID_ENVIRONMENTAL_SENSOR
#include "EnvironmentalSensor.h"
#include "XdkSensorHandle.h"

/* additional interface header files */
#include "BCDS_Basics.h"
#include "BCDS_Environmental.h"
#include "BCDS_Assert.h"
#include "BCDS_CmdProcessor.h"
#include "FreeRTOS.h"
#include "timers.h"


/* system header files */
#include <stdio.h>

/* constant definitions ***************************************************** */
#define THREESECONDDELAY  UINT32_C(3000)        /**< three seconds delay is represented by this macro */
#define TIMERBLOCKTIME  UINT32_C(0xffff)    /** Macro used to define blocktime of a timer*/
#define TIMER_NOT_ENOUGH_MEMORY            (-1L)/**<Macro to define not enough memory error in timer*/
#define TIMER_AUTORELOAD_ON             UINT32_C(1)             /**< Auto reload of timer is enabled*/


char * processEnvSensorData(void * param1, uint32_t param2)
{
	BCDS_UNUSED(param1);
    BCDS_UNUSED(param2);

    char  *buffer = calloc(255, sizeof(char));

    Retcode_T returnValue = (Retcode_T) RETCODE_FAILURE;
    Environmental_Data_T bme280 = { INT32_C(0), UINT32_C(0), UINT32_C(0) };

    /* Read temperature,pressure,humidity actual values */
    returnValue = Environmental_readData(xdkEnvironmental_BME280_Handle, &bme280);
    if ( RETCODE_OK == returnValue)
    {

       sprintf(buffer,"{\"sensor\": \"Environmental\",\"data\":[{\"Pressure\":\"%ld\"},{\"Temp\":\"%ld\"},{\"Humidity\":\"%ld\"}]}",
        		(long int) bme280.pressure, (long int) bme280.temperature, (long int) bme280.humidity);

    }
    else
    {
        printf("Environmental Read actual Data Failed\n\r");
    }


    return (char*)buffer;
}


Retcode_T environmentalSensorInit(void)
{
    Retcode_T returnValue = (Retcode_T) RETCODE_FAILURE;

    /*initialize Environmental sensor*/
    returnValue = Environmental_init(xdkEnvironmental_BME280_Handle);
    if ( RETCODE_OK == returnValue)
    {
        printf("Environmental Sensor initialization Success\n\r");
    }
    else
    {
        printf("Environmental Sensor initialization Failed\n\r");
    }
    return returnValue;
}

Retcode_T environmentalSensorDeinit(void)
{
    Retcode_T returnValue = (Retcode_T) RETCODE_FAILURE;

    returnValue = Environmental_deInit(xdkEnvironmental_BME280_Handle);
    if (RETCODE_OK == returnValue)
    {
        printf("Environmental sensor Deinit Success\n\r");
    }
    else
    {
        printf("Environmental sensor Deinit Failed\n\r");
    }
    return returnValue;
}
