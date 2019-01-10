

/* header definition ******************************************************** */
#ifndef XDK110_MAGNETOMETER_H_
#define XDK110_MAGNETOMETER_H_

/* public interface declaration ********************************************* */
#include "BCDS_Retcode.h"
/* public type and macro definitions */

/* public function prototype declarations */

/**
 * @brief The function initializes BMM150(magnetometer)creates and starts a autoreloaded
 * timer task which gets and prints the Magnetometer lsb and converted data.
 *
 * @retval Retcode_T RETCODE_OK Initialization Success
 *
 * @retval Retcode_T Composed RETCODE error Initialization Failed,use Retcode_getCode() API to get the error code
 *
 */
char* processMagnetometerData(void * param1, uint32_t param2);
Retcode_T magnetometerSensorInit(void);

/**
 *  @brief API to de-initialize the BMM150(magnetometer) sensor.
 *
 *  @retval Retcode_T RETCODE_OK deinitialization Success
 *
 *  @retval Retcode_T Composed RETCODE error deinitialization Failed,use Retcode_getCode() API to get the error code
 */
Retcode_T magnetometerSensorDeinit(void);

/* public global variable declarations */

/* inline function definitions */

#endif /* XDK110_MAGNETOMETER_H_ */

/** ************************************************************************* */
