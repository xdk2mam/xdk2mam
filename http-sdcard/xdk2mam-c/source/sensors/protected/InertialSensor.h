
/* header definition ******************************************************** */
#ifndef XDK110_INERTIALSENSOR_H_
#define XDK110_INERTIALSENSOR_H_

/* public interface declaration ********************************************* */
#include "BCDS_Retcode.h"
/* public type and macro definitions */

/* public function prototype declarations */

/**
 * @brief The function initializes BMI(Interial-accel & gyro)creates and starts a auto reloaded
 * timer task which gets and prints the accel and gyro data.
 *
 * @retval Retcode_T RETCODE_OK Initialization Success
 *
 * @retval Retcode_T Composed RETCODE error Initialization Failed,use Retcode_getCode() API to get the error code
 */
char* processInertiaSensor(void * param1, uint32_t param2);
Retcode_T inertialSensorInit(void);

/**
 *  @brief API to deinitialize the BMI(Interial-accel & gyro) sensor.
 *
 *  @retval Retcode_T RETCODE_OK deinitialization Success
 *
 *  @retval Retcode_T Composed RETCODE error deinitialization Failed,use Retcode_getCode() API to get the error code
 */
Retcode_T inertialSensorDeinit(void);

/* public global variable declarations */

/* inline function definitions */

#endif /* XDK110_INERTIALSENSOR_H_ */

/**@}*/

/** ************************************************************************* */
