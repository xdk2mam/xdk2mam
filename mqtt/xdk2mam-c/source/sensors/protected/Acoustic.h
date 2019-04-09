

/* header definition ******************************************************** */
#ifndef XDK110_ACOUSTIC_H_

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
char* processAcousticData(void * param1, uint32_t param2);
Retcode_T acousticSensorInit(void);


#endif /* XDK110_MAGNETOMETER_H_ */

/** ************************************************************************* */
