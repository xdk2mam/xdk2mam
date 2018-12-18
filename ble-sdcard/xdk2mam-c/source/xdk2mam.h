
#ifndef XDK110_HTTPXDK2MAM_H
#define XDK110_HTTPXDK2MAM_H

#include "BCDS_Basics.h"
#include "BCDS_CmdProcessor.h"
/* local type and macro definitions */

#define MAX_SENSORS_ARRAY 6

#define DEFAULT_LOGICAL_DRIVE   ""
#define DRIVE_ZERO  UINT8_C(0)
#define FORCE_MOUNT UINT8_C(1)
#define FIRST_LOCATION UINT8_C(0)
#define BLE_START_SYNC_TIMEOUT UINT32_C(5000)
#define BLE_WAKEUP_SYNC_TIMEOUT UINT32_C(5000)
//#define BLE_SEND_TIMEOUT UINT32_C(1000)

typedef enum{
	ENVIROMENTAL = 0,
	ACCELEROMETER = 1,
	GYROSCOPE = 2,
	INERTIAL = 3,
	LIGHT = 4,
	MAGNETOMETER = 5
}types_of_sensors;

typedef enum{
	t_DEVICE_NAME = 1,
	t_INTER_REQUEST_INTERVAL = 2,
	t_INTERVAL_STREAM_DIVIDER_BLE = 3
};


#define FILE_NAME "config.cfg"

/**
 * POST_REQUEST_CUSTOM_HEADER_0 is a custom header which is sent along with the
 * POST request. It's meant to demonstrate how to use custom header.
 */
#define POST_REQUEST_CUSTOM_HEADER_0    "X-AuthToken: XDK2MAM\r\n"

/**
 * POST_REQUEST_CUSTOM_HEADER_1 is a custom header which is sent along with the
 * POST request. It's meant to demonstrate how to use custom header.
 */
#define POST_REQUEST_CUSTOM_HEADER_1    "X-Foobar: XDK2MAM\r\n"



#define TIMER_AUTORELOAD_ON             UINT32_C(1)

#define TIMERBLOCKTIME                  UINT32_C(0xffff)

/**
 * The maximum amount of data we download in a single request (in bytes). This number is
 * limited by the platform abstraction layer implementation that ships with the
 * XDK. The maximum value that will work here is 512 bytes.
 */
#define REQUEST_MAX_DOWNLOAD_SIZE       UINT32_C(512)

/* local module global variable declarations */

/* local inline function definitions */
/**
 * @brief This is a template function where the user can write his custom application.
 *
 * @param[in] CmdProcessorHandle Handle of the main command processor
 *
 * @param[in] param2  Currently not used will be used in future
 *
 */
void appInitSystem(void * CmdProcessorHandle, uint32_t param2);
CmdProcessor_T * GetAppCmdProcessorHandle(void);


#endif /* XDK110_HTTPXDK2MAM_H */
