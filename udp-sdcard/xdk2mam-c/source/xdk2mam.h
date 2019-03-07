
#ifndef XDK110_HTTPXDK2MAM_H
#define XDK110_HTTPXDK2MAM_H

#include "BCDS_Basics.h"
#include "BCDS_CmdProcessor.h"
/* local type and macro definitions */

#define MAX_SENSORS_ARRAY 7

#define DEFAULT_LOGICAL_DRIVE   ""
#define DRIVE_ZERO  UINT8_C(0)
#define FORCE_MOUNT UINT8_C(1)
#define FIRST_LOCATION UINT8_C(0)

typedef enum{
	ENVIROMENTAL = 0,
	ACCELEROMETER = 1,
	GYROSCOPE = 2,
	INERTIAL = 3,
	LIGHT = 4,
	MAGNETOMETER = 5,
	ACOUSTIC = 6
}types_of_sensors;

typedef enum{
	t_DEVICE_NAME = 1,
	t_WLAN_SSID = 2,
	t_WLAN_PSK = 3,
	t_DEST_SERVER_HOST = 4,
	t_DEST_SERVER_PORT = 5,
	t_INTER_REQUEST_INTERVAL = 6
}types_sd_card_inputs;


#define LOG_MODULE "NTP"
#define SNTP_DEFAULT_PORT UINT16_C(123)

/**
 * @brief Definition of the default SNTP Server host.
 */
#define SNTP_DEFAULT_ADDR "129.6.15.28"

#define FILE_NAME "config.cfg"

#define NTP_PACKET_SIZE                                                        \
  UINT8_C(48)
#define NTP_DNS_TIMEOUT_IN_S UINT16_C(5)
#define NTP_DNS_RETRY_INTERVAL_IN_MS UINT16_C(100)


/**
 * DEST_SERVER_PORT_SECURE is the TCP port to which we will send HTTPS requests to.
 * The default of 443 should be fine for most applications.
 */
#define DEST_SERVER_PORT_SECURE                UINT16_C(443)

/**
 * HTTP_SECURE_ENABLE is Set to Use HTTP With Security
 */
#define HTTP_SECURE_ENABLE          UINT32_C(0)


#define TIMER_AUTORELOAD_ON             UINT32_C(1)

#define TIMERBLOCKTIME                  UINT32_C(0xffff)

/**
 * The maximum amount of data we download in a single request (in bytes). This number is
 * limited by the platform abstraction layer implementation that ships with the
 * XDK. The maximum value that will work here is 512 bytes.
 */
#define REQUEST_MAX_DOWNLOAD_SIZE       UINT32_C(512)

#define APP_RESPONSE_FROM_HTTP_SERVER_POST_TIMEOUT      UINT32_C(25000)/**< Timeout for completion of HTTP rest client POST */

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
