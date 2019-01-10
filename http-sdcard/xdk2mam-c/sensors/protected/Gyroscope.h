


/* header definition ******************************************************** */
#ifndef XDK110_GYROSCOPE_H_
#define XDK110_GYROSCOPE_H_

/* public interface declaration ********************************************* */
#include "BCDS_Retcode.h"
/* public type and macro definitions */

/* public function prototype declarations */

/**
 * @brief The function initializes BMG160 sensor and creates, starts timer task in autoreloaded mode
 * every three second which reads and prints the Gyro sensor data .
 *
 * @retval Retcode_T RETCODE_OK Initialization Success
 *
 * @retval Retcode_T Composed RETCODE error Initialization Failed,use Retcode_getCode() API to get the error code
 *
 */
char* processGyroData(void * param1, uint32_t param2);
Retcode_T gyroscopeSensorInit(void);

/**
 *  @brief  the function to deinitialize the BMG160 Sensor.
 *
 *  @retval Retcode_T RETCODE_OK deinitialization Success
 *
 *  @retval Retcode_T Composed RETCODE error deinitialization Failed,use Retcode_getCode() API to get the error code
 *
 */
Retcode_T gyroscopeSensorDeinit(void);

/* public global variable declarations */

/* inline function definitions */

#endif /* XDK110_GYROSCOPE_H_ */

/** ************************************************************************* */
