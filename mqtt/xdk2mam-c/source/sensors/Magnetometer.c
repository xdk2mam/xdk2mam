

/* own header files */
#include "XDKAppInfo.h"
#undef BCDS_MODULE_ID
#define BCDS_MODULE_ID XDK_APP_MODULE_ID_MAGNETOMETER
#include "Magnetometer.h"
#include "XdkSensorHandle.h"

/* additional interface header files */
#include "BCDS_Basics.h"
#include "BCDS_Magnetometer.h"
#include "BCDS_Assert.h"
#include "BCDS_CmdProcessor.h"
#include "FreeRTOS.h"
#include "timers.h"


/* system header files */
#include <stdio.h>

/* constant definitions ***************************************************** */
#define TIMERDELAY       UINT32_C(3000)          /**< three seconds delay is represented by this macro */
#define TIMERBLOCKTIME   UINT32_C(0xffff)        /** Macro used to define blocktime of a timer*/
#define ZEROVALUE        UINT32_C(0x00)          /** Macro used to define default value*/
#define TIMER_NOT_ENOUGH_MEMORY            (-1L)/**<Macro to define not enough memory error in timer*/
#define TIMER_AUTORELOAD_ON             UINT32_C(1)             /**< Auto reload of timer is enabled*/


char* processMagnetometerData(void * param1, uint32_t param2)
{
    BCDS_UNUSED(param1);
    BCDS_UNUSED(param2);
    char  *buffer = calloc(255, sizeof(char));
    Retcode_T sensorApiRetValue = (Retcode_T) RETCODE_FAILURE;
    Magnetometer_XyzData_T getMagData =
            { INT32_C(0), INT32_C(0), INT32_C(0), INT32_C(0) };

    sensorApiRetValue = Magnetometer_readXyzTeslaData(xdkMagnetometer_BMM150_Handle, &getMagData);
    if ( RETCODE_OK == sensorApiRetValue)
    {
    	//microtesla
        sprintf(buffer,"{\"sensor\":\"Magnetometer\",\"data\":[{\"x\":\"%ld\"},{\"y\":\"%ld\"},{\"z\":\"%ld\"}]}",
							(long int) getMagData.xAxisData, (long int) getMagData.yAxisData, (long int) getMagData.zAxisData);
    }
    else
    {
        printf("Magnetometer XYZ MicroTeslaData read FAILED\n\r");
    }
    return (char*)buffer;
}


Retcode_T magnetometerSensorInit(void)
{
    /* Return value for magnetometerInit and magnetometerSetMode api*/
    Retcode_T magReturnValue = (Retcode_T) RETCODE_FAILURE;


    /* Initialization for Magnetometer Sensor */
    magReturnValue = Magnetometer_init(xdkMagnetometer_BMM150_Handle);

    if (RETCODE_OK == magReturnValue)
    {
        printf("Magnetometer Sensor initialization Success\n\r");
    }
    else
    {
        printf("Magnetometer initialization FAILED\n\r");
    }

    return magReturnValue;
}

Retcode_T magnetometerSensorDeinit(void)
{
    Retcode_T returnValue = (Retcode_T) RETCODE_FAILURE;
    returnValue = Magnetometer_deInit(xdkMagnetometer_BMM150_Handle);
    if (RETCODE_OK == returnValue)
    {
        printf("Magnetometer Deinit Success\n\r");
    }
    else
    {
        printf("Magnetometer Deinit Failed\n\r");
    }
    return returnValue;
}
/**@} */
