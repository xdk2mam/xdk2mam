

/* own header files */
#include "XDKAppInfo.h"
#undef BCDS_MODULE_ID
#define BCDS_MODULE_ID XDK_APP_MODULE_ID_INERTIAL_SENSOR
#include "InertialSensor.h"
#include "XdkSensorHandle.h"

/* additional interface header files */
#include "BCDS_Basics.h"
#include "BCDS_Accelerometer.h"
#include "BCDS_Gyroscope.h"
#include "BCDS_Assert.h"
#include "BCDS_CmdProcessor.h"
#include "FreeRTOS.h"
#include "timers.h"


/* system header files */
#include <stdio.h>

/* constant definitions ***************************************************** */
#define TIMER_NOT_ENOUGH_MEMORY            (-1L)/**<Macro to define not enough memory error in timer*/
#define TIMER_AUTORELOAD_ON             UINT32_C(1)             /**< Auto reload of timer is enabled*/
#define THREESECONDDELAY  UINT32_C(3000)      /**< three seconds delay is represented by this macro */
#define TIMERBLOCKTIME  UINT32_C(0xffff)    /** Macro used to define blocktime of a timer*/


char* processInertiaSensor(void * param1, uint32_t param2)
{
    BCDS_UNUSED(param1);
    BCDS_UNUSED(param2);

    /* Return value for Accel Sensor */
    Retcode_T gyroReturnValue = (Retcode_T) RETCODE_FAILURE;

    Gyroscope_XyzData_T getConvData = { INT32_C(0), INT32_C(0), INT32_C(0) };

    char  *buffer = calloc(255, sizeof(char));

    /* read sensor data in milli Degree*/
    gyroReturnValue = Gyroscope_readXyzDegreeValue(xdkGyroscope_BMI160_Handle, &getConvData);
    if (RETCODE_OK == gyroReturnValue)
    {

        sprintf(buffer,"{\"sensor\":\"Inertial\",\"data\":[{\"x\":\"%ld\"},{\"y\":\"%ld\"},{\"z\":\"%ld\"}]}",
							(long int) getConvData.xAxisData, (long int) getConvData.yAxisData, (long int) getConvData.zAxisData);

    }
    else
    {
        printf("BMI160 GyrosensorReadInMilliDeg Failed\n\r");
    }

    return (char*)buffer;
}



Retcode_T inertialSensorInit(void)
{

    /* Return value for Accel Sensor */
    Retcode_T returnValue = (Retcode_T) RETCODE_FAILURE;

    /*initialize accel*/
    returnValue = Accelerometer_init(xdkAccelerometers_BMI160_Handle);
    if (RETCODE_OK == returnValue)
    {
        returnValue = Gyroscope_init(xdkGyroscope_BMI160_Handle);
    }
    if ((RETCODE_OK == returnValue))
    {
        uint32_t Ticks = THREESECONDDELAY;

        if (Ticks != UINT32_MAX) /* Validated for portMAX_DELAY to assist the task to wait Infinitely (without timing out) */
        {
            Ticks /= portTICK_RATE_MS;
        }
        if (UINT32_C(0) == Ticks) /* ticks cannot be 0 in FreeRTOS timer. So ticks is assigned to 1 */
        {
            Ticks = UINT32_C(1);
        }
    }
    else
    {
        printf("InertialSensor init Failed\n\r");
    }
    printf("Inertial Sensor initialization Success\n\r");

    return returnValue;
}

Retcode_T inertialSensorDeinit(void)
{

    Retcode_T returnValue = (Retcode_T) RETCODE_FAILURE;

    /*initialize accel*/
    returnValue = Accelerometer_deInit(xdkAccelerometers_BMI160_Handle);
    if (RETCODE_OK == returnValue)
    {
        /*initialize gyro*/
        returnValue = Gyroscope_deInit(xdkGyroscope_BMI160_Handle);
    }

    if (RETCODE_OK == returnValue)
    {
        printf("InertialSensor Deinit Success\n\r");
    }
    else
    {
        printf("InertialSensor Deinit Failed\n\r");
    }
    return returnValue;
}
/**@} */
/** ************************************************************************* */
