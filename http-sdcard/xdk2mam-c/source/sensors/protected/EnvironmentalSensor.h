
/* header definition ******************************************************** */
#ifndef XDK110_ENVIRONMENTALSENSOR_H_
#define XDK110_ENVIRONMENTALSENSOR_H_

/* public interface declaration ********************************************* */
#include "BCDS_Retcode.h"
/* public type and macro definitions */

/* public function prototype declarations */

/**
 * @brief The function initializes BME(Environmental)creates and starts a autoreloaded
 * timer task which gets and prints the Environmental raw data and actual data .
 *
 * @retval Retcode_T RETCODE_OK Initialization Success
 *
 * @retval Retcode_TComposed RETCODE error Initialization Failed,use Retcode_getCode() API to get the error code
 *
 */
void printEnvData(void *pvParameters);
char* processEnvSensorData(void * param1, uint32_t param2);
extern Retcode_T environmentalSensorInit(void);

/**
 *  @brief API to deinitialize the BME(Environmental) sensor.
 *
 *  @retval Retcode_T RETCODE_OK deinitialization Success
 *
 *  @retval Retcode_T Composed RETCODE error deinitialization Failed,use Retcode_getCode() API to get the error code
 *
 */

extern Retcode_T environmentalSensorDeinit(void);

/* public global variable declarations */

/* inline function definitions */

#endif /* XDK110_ENVIRONMENTALSENSOR_H_ */

/**@}*/

/** ************************************************************************* */
