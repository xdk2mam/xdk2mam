#include "XDKAppInfo.h"
#undef BCDS_MODULE_ID
#define BCDS_MODULE_ID XDK_APP_MODULE_ID_ACCELEROMETER
#include "Accelerometer.h"
#include "XdkSensorHandle.h"
#include "BCDS_Basics.h"
#include "BCDS_Accelerometer.h"
#include "BCDS_CmdProcessor.h"
#include "BCDS_Assert.h"
#include "FreeRTOS.h"
#include "timers.h"
#include <stdio.h>


#define THREESECONDDELAY                UINT32_C(3000)
#define TIMERBLOCKTIME                  UINT32_C(0xffff)
#define VALUE_ZERO                      UINT32_C(0)
#define FAILURE                        UINT32_C(1)
#define EVENNUMBER_IDENTIFIER           UINT8_C(2)
#define ACCELEROMETER_SLOPE_THRESHOLD   UINT8_C(100)
#define INTERRUPT_DISABLE			    INT8_C(0)
#define INTERRUPT_ENABLE			    INT8_C(1)
#define TIMER_NOT_ENOUGH_MEMORY            (-1L)
#define TIMER_AUTORELOAD_ON             UINT32_C(1)


static uint32_t accelerometerIntConfigStatus = VALUE_ZERO; /**< variable to identify interrupt configuration is done or not*/


char* processAccelData(void * param1, uint32_t param2)
{
    BCDS_UNUSED(param1);
    BCDS_UNUSED(param2);

    char  *buffer = calloc(255, sizeof(char));


    Accelerometer_XyzData_T getaccelData = { INT32_C(0), INT32_C(0), INT32_C(0) };

    if ( RETCODE_OK == Accelerometer_readXyzGValue(xdkAccelerometers_BMA280_Handle, &getaccelData))
    {
        sprintf(buffer,"{\"sensor\":\"Accel\",\"data\":[{\"x\":\"%ld\"},{\"y\":\"%ld\"},{\"z\":\"%ld\"}]}",
							(long int) getaccelData.xAxisData, (long int) getaccelData.yAxisData, (long int) getaccelData.zAxisData);
    }
    else
    {
        printf("Accelerometer Gravity XYZ Data read FAILED\n\r");
    }

    return (char*)buffer;
}

Retcode_T accelerometerSensorInit(void)
{
    /* Return value for Timer start */

    Retcode_T returnVal = RETCODE_OK;

    Accelerometer_InterruptChannel_T interruptChannel = ACCELEROMETER_BMA280_INTERRUPT_CHANNEL1;
    Accelerometer_InterruptType_T interruptType = ACCELEROMETER_BMA280_SLOPE_INTERRUPT;
    Accelerometer_ConfigSlopeIntr_T slopeInterruptConfig;

    /*initialize accel*/

    returnVal = Accelerometer_init(xdkAccelerometers_BMA280_Handle);

    if (RETCODE_OK == returnVal)
    {

        if (RETCODE_OK == returnVal)
        {
            /*Configure interrupt conditions*/
            slopeInterruptConfig.slopeDuration = ACCELEROMETER_BMA280_SLOPE_DURATION4;
            slopeInterruptConfig.slopeThreshold = ACCELEROMETER_SLOPE_THRESHOLD;
            slopeInterruptConfig.slopeEnableX = INTERRUPT_DISABLE;
            slopeInterruptConfig.slopeEnableY = INTERRUPT_ENABLE;
            slopeInterruptConfig.slopeEnableZ = INTERRUPT_DISABLE;

            returnVal = Accelerometer_configInterrupt(xdkAccelerometers_BMA280_Handle, interruptChannel, interruptType, &slopeInterruptConfig);

            if ( RETCODE_OK != returnVal)
            {
                accelerometerIntConfigStatus = FAILURE;
                printf("accelerometerConfigureSlopeInterrupt Failed...\n\r");
            }
            else
            {
            	printf("Accelerometer Sensor initialization Success\n\r");
            }
        }
        else
        {
            accelerometerIntConfigStatus = FAILURE;
            printf("accelerometerRegisterRealTimeCallback Failed...\n\r");
        }

    }
    else
    {
        printf("Accelerometer initialization FAILED\n\r");
    }
    return returnVal;
}

Retcode_T accelerometerSensorDeinit(void)
{
    Retcode_T returnValue = (Retcode_T) RETCODE_FAILURE;
    returnValue = Accelerometer_deInit(xdkAccelerometers_BMA280_Handle);
    if (RETCODE_OK == returnValue)
    {
        printf("Accelerometer Deinit Success\n\r");
    }
    else
    {
        printf("Accelerometer Deinit Failed\n\r");
    }
    return returnValue;
}
