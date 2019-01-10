

/* header definition ******************************************************** */
#ifndef XDK110_LIGHTSENSOR_H_
#define XDK110_LIGHTSENSOR_H_

/* public interface declaration ********************************************* */
#include "BCDS_Retcode.h"
/* public type and macro definitions */

/* public function prototype declarations */

/**
 * @brief The function initializes light sensor and creates, starts timer task in autoreloaded mode
 * every three second which reads and prints the light sensor data.
 *
 * @retval Retcode_T RETCODE_OK Initialization Success
 *
 * @retval Retcode_T Composed RETCODE error Initialization Failed,use Retcode_getCode() API to get the error code
 */
char* processLightSensorData(void * param1, uint32_t param2);
Retcode_T lightsensorInit(void);

/**
 *  @brief  the function to deinitialize light sensor.
 *
 *  @retval Retcode_T RETCODE_OK deinitialization Success
 *
 *  @retval Retcode_T Composed RETCODE error deinitialization Failed,use Retcode_getCode() API to get the error code
 *
 */
Retcode_T lightsensorDeinit(void);

/* public global variable declarations */

/* inline function definitions */

#endif /* XDK110_LIGHTSENSOR_H_ */

/** ************************************************************************* */
