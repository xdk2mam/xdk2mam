
#ifndef XDK2MAM_MQTT_H_
#define XDK2MAM_MQTT_H_

#include "BCDS_Retcode.h"

#define MAX_SENSORS_ARRAY 7

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
	t_MQTT_BROKER_HOST = 4,
	t_MQTT_BROKER_PORT = 5,
	t_PUBLISHTIMER_PERIOD_IN_MS = 6,
	t_MQTT_USERNAME = 7,
	t_MQTT_PASSWORD = 8,
	t_TOPIC = 9
}types_sd_card_inputs;

#define LOG_MODULE "NTP"
#define SNTP_DEFAULT_PORT UINT16_C(123)

#define DEFAULT_LOGICAL_DRIVE   ""
#define DRIVE_ZERO  UINT8_C(0)
#define FORCE_MOUNT UINT8_C(1)
#define FIRST_LOCATION UINT8_C(0)

/**
 * @brief Definition of the default SNTP Server host.
 */
#define SNTP_DEFAULT_ADDR "129.6.15.28"

#define NTP_PACKET_SIZE                                                        \
  UINT8_C(48)
#define NTP_DNS_TIMEOUT_IN_S UINT16_C(5)
#define NTP_DNS_RETRY_INTERVAL_IN_MS UINT16_C(100)


#define USE_PUBLISH_TIMER 	true

#define PUBLISH_BUFFER_SIZE 1024

#define COMMON_BUFFER_SIZE 	PUBLISH_BUFFER_SIZE


#define FILE_NAME "config.cfg"


enum App_Retcode_E
{
    RETCODE_MQTT_PUBLISH_FAIL = RETCODE_FIRST_CUSTOM_CODE,
    RETCODE_TIMER_START_FAIL,
    RETCODE_MQTT_SUBSCRIBE_FAIL,
    RETCODE_MQTT_CONNECT_FAIL,
    RETCODE_MQTT_DISCONNECT_FAIL,
};

void AppInitSystem(void * CmdProcessorHandle, uint32_t param2);

#endif
