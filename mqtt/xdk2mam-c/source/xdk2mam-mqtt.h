
#ifndef XDK2MAM_MQTT_H_
#define XDK2MAM_MQTT_H_

#include "BCDS_Retcode.h"

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



#define WIFI_SSID 			"YourWifiNetwork"

#define WIFI_PW				"YourWifiPassword"

#define MQTT_BROKER_HOST	"MQTT_IP_BROKER_HOST"


#define MQTT_BROKER_PORT	UINT16_C(17567) //<-- MQTT_PORT_BROKER

#define MQTT_USERNAME	"YourMqttUsername"

#define MQTT_PASSWORD	"YourMqttPassowrd"

#define DEVICE_NAME			"XDK-DEVICE-NAME"	

#define USE_PUBLISH_TIMER 	true

#define PUBLISH_BUFFER_SIZE 1024

#define COMMON_BUFFER_SIZE 	PUBLISH_BUFFER_SIZE

#define TOPIC				"YourTopicName"

#define PUBLISHTIMER_PERIOD_IN_MS 30000

enum App_Retcode_E
{
    RETCODE_MQTT_PUBLISH_FAIL = RETCODE_FIRST_CUSTOM_CODE,
    RETCODE_TIMER_START_FAIL,
    RETCODE_MQTT_SUBSCRIBE_FAIL,
    RETCODE_MQTT_CONNECT_FAIL,
    RETCODE_MQTT_DISCONNECT_FAIL,
    RETCODE_MQTT_IPCONIG_FAIL,
};

void AppInitSystem(void * CmdProcessorHandle, uint32_t param2);

#endif
