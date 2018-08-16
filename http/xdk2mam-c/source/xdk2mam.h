
#ifndef XDK110_HTTPXDK2MAM_H
#define XDK110_HTTPXDK2MAM_H

#include "BCDS_Basics.h"
#include "BCDS_CmdProcessor.h"
/* local type and macro definitions */

#define MAX_SENSORS_ARRAY 6

typedef enum{
	ENVIROMENTAL = 0,
	ACCELEROMETER = 1,
	GYROSCOPE = 2,
	INERTIAL = 3,
	LIGHT = 4,
	MAGNETOMETER = 5
}types_of_sensors;


#define LOG_MODULE "NTP"
#define SNTP_DEFAULT_PORT UINT16_C(123)

/**
 * @brief Definition of the default SNTP Server host.
 */
#define SNTP_DEFAULT_ADDR "129.6.15.28"

#define NTP_PACKET_SIZE                                                        \
  UINT8_C(48)
#define NTP_DNS_TIMEOUT_IN_S UINT16_C(5)
#define NTP_DNS_RETRY_INTERVAL_IN_MS UINT16_C(100)


#define DEVICE_NAME			"XDK-aleelus"
/**
 * WLAN_CONNECT_WPA_SSID is the SSID of the WIFI network you want to connect to.
 */
#define WLAN_SSID                        "YourWifiNetwork"
/**
 * WLAN_CONNECT_WPA_PASS is the WPA/WPA2 passphrase (pre-shared key) of your WIFI network.
 */
#define WLAN_PSK                         "YourWifiPassword"

/**
 * DEST_SERVER_HOST is the host name of the web server we will send HTTP requests to.
 * If you want to test this example without setting up your own server, you can use publicly available services.
 */
#define DEST_SERVER_HOST                 "192.168.0.4"

/**
 * DEST_SERVER_PORT is the TCP port to which we will send HTTP requests to.
 * The default of 80 should be fine for most applications.
 */
#define DEST_SERVER_PORT                UINT16_C(8080)


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
 * DEST_POST_PATH is the path relative to the DEST_SERVER_HOST that we will send
 * the HTTP POST request to.
 *
 * Change this value if you use your own web server.
 */
#define DEST_POST_PATH                  "/sensors"

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


/**
 * The time we wait (in milliseconds) between sending HTTP requests.
 */
#define INTER_REQUEST_INTERVAL          UINT32_C(30000)


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
