
typedef enum{
	ENVIROMENTAL = 0,
	ACCELEROMETER = 1,
	GYROSCOPE = 2,
	LIGHT = 3
}types_of_sensors;



typedef enum{
	t_APP_LORA_RX_WINDOW_FREQ = 1,
	t_APP_LORA_FREQUENCY = 2,
	t_APP_CONTROLLER_LORA_TX_DELAY = 3,
	t_APP_LORA_APP_EUI = 4,
	t_APP_LORA_APP_KEY = 5
}types_sd_card_inputs;



/* header definition ******************************************************** */
#ifndef APPCONTROLLER_H_
#define APPCONTROLLER_H_

/* local interface declaration ********************************************** */
#include "XDK_Utils.h"

#define MAX_SENSORS_ARRAY 4

#define FILE_NAME "config.cfg"

#define DEFAULT_LOGICAL_DRIVE   ""
#define DRIVE_ZERO  UINT8_C(0)
#define FORCE_MOUNT UINT8_C(1)
#define FIRST_LOCATION UINT8_C(0)

/* local type and macro definitions */
/**
 * APP_LORA_CODING_RATE is the Coding rate used for LORA communication.
 */
#define APP_LORA_CODING_RATE                      "4/5"

/**
 * APP_LORA_BUFFER_MAX_SIZE is the buffer size allocated for the transmission based the sensor data we send this has to be updated
 */
#define APP_LORA_BUFFER_MAX_SIZE  UINT32_C(48)
/**
 * APP_LORA_PORT is the LoraWan Port that will be used for sending.
 */
#define APP_LORA_PORT         UINT8_C(1)

/**
 * @brief Gives control to the Application controller.
 *
 * @param[in] cmdProcessorHandle
 * Handle of the main command processor which shall be used based on the application needs
 *
 * @param[in] param2
 * Unused
 */
void AppController_Init(void * cmdProcessorHandle, uint32_t param2);

#endif /* APPCONTROLLER_H_ */

/** ************************************************************************* */
