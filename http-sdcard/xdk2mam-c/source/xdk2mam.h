
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
	t_INTER_REQUEST_INTERVAL = 6,
	t_DEST_POST_PATH = 7
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
 * WLAN_CONNECT_WPA_SSID is the SSID of the WIFI network you want to connect to.
 */
/**
 * WLAN_CONNECT_WPA_PASS is the WPA/WPA2 passphrase (pre-shared key) of your WIFI network.
 */

/**
 * DEST_SERVER_HOST is the host name of the web server we will send HTTP requests to.
 * If you want to test this example without setting up your own server, you can use publicly available services.
 */

/**
 * DEST_SERVER_PORT is the TCP port to which we will send HTTP requests to.
 * The default of 80 should be fine for most applications.
 */


/**
 * DEST_SERVER_PORT_SECURE is the TCP port to which we will send HTTPS requests to.
 * The default of 443 should be fine for most applications.
 */
#define DEST_SERVER_PORT_SECURE                UINT16_C(443)

/**
 * DEST_GET_PATH is the path relative to the DEST_SERVER_HOST that we will send
 * the HTTP GET request to. Using / will retrieve the index page of the web server
 * which for demo purposes may be enough.
 *
 * Change this value if you use your own web server.
 */
#define DEST_GET_PATH                   "/status"

/**
 * POST_REQUEST_CUSTOM_HEADER_0 is a custom header which is sent along with the
 * POST request. It's meant to demonstrate how to use custom header.
 */
#define POST_REQUEST_CUSTOM_HEADER_0    "X-AuthToken: InsertCrypticAuthenticationToken\r\n"

/**
 * POST_REQUEST_CUSTOM_HEADER_1 is a custom header which is sent along with the
 * POST request. It's meant to demonstrate how to use custom header.
 */
#define POST_REQUEST_CUSTOM_HEADER_1    "X-Foobar: AnotherCustomHeader\r\n"


/**
 * The time we wait (in milliseconds) between sending HTTP requests.
 */


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
