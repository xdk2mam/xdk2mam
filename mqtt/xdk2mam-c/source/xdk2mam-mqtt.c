
#include "XDKAppInfo.h"
#undef BCDS_MODULE_ID
#define BCDS_MODULE_ID  XDK_APP_MODULE_ID_XDK2MAM_MQTT

/* system header files */
#include "xdk2mam-mqtt.h"

#include <stdio.h>
/* additional interface header files */
#include "BCDS_Basics.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "semphr.h"

#include "BCDS_NetworkConfig.h"
#include "BCDS_WlanConnect.h"
#include "BCDS_ServalPal.h"
#include "BCDS_ServalPalWiFi.h"


/* own header files */
#include "BCDS_CmdProcessor.h"
#include "BCDS_Assert.h"
#include "Serval_Mqtt.h"
#include <Serval_Clock.h>
#include <Serval_Msg.h>
#include <Serval_XUdp.h>
#include <Serval_Log.h>
#include "EnvironmentalSensor.h"
#include "Accelerometer.h"
#include "Gyroscope.h"
#include "InertialSensor.h"
#include "LightSensor.h"
#include "Magnetometer.h"

#include "XdkSensorHandle.h"

// Global array of all sensors => true : enable -- false : disable
bool typesSensors[6] = {
						true, //ENVIROMENTAL
						true, //ACCELEROMETER
						true, //GYROSCOPE
						true, //INERTIAL
						true, //LIGHT
						true  //MAGNETOMETER
					};

static CmdProcessor_T *AppCmdProcessor;
static CmdProcessor_T CmdProcessorHandleServalPAL;
static MqttSession_T Session;
static MqttSession_T *SessionPtr;


static uint8_t PublishInProgress = 0;

static TimerHandle_t PublishTimerHandle;

static const char *PublishTopic = TOPIC;
static StringDescr_T PublishTopicDescription;

static StringDescr_T Topics[1];
static Mqtt_qos_t Qos[1];

static char MqttBroker[50];
static const char MqttBrokerAddressFormat[50] = "mqtt://%s:%d";
static const char *DeviceName = DEVICE_NAME;
static uint32_t SysTime = UINT32_C(0);


static Retcode_T ServalPalSetup(void)
{
    Retcode_T returnValue = RETCODE_OK;
    returnValue = CmdProcessor_Initialize(&CmdProcessorHandleServalPAL, (char *)"Serval PAL", TASK_PRIORITY_SERVALPAL_CMD_PROC, TASK_STACK_SIZE_SERVALPAL_CMD_PROC, TASK_QUEUE_LEN_SERVALPAL_CMD_PROC);
    /* serval pal common init */
    if (RETCODE_OK == returnValue)
    {
        returnValue = ServalPal_Initialize(&CmdProcessorHandleServalPAL);
    }
    if (RETCODE_OK == returnValue)
    {
        returnValue = ServalPalWiFi_Init();
    }
    if (RETCODE_OK == returnValue)
    {
        ServalPalWiFi_StateChangeInfo_T stateChangeInfo = { SERVALPALWIFI_OPEN, 0 };
        returnValue = ServalPalWiFi_NotifyWiFiEvent(SERVALPALWIFI_STATE_CHANGE, &stateChangeInfo);
    }
    return returnValue;
}

static Retcode_T NetworkSetup(void)
{
    Retcode_T retcode = RETCODE_OK;
    WlanConnect_SSID_T connectSSID = (WlanConnect_SSID_T) WIFI_SSID;
    WlanConnect_PassPhrase_T connectPassPhrase;
    retcode = WlanConnect_Init();
    if (retcode == RETCODE_OK)
    {
        retcode = NetworkConfig_SetIpDhcp(0);
    }
    if (retcode == RETCODE_OK)
    {
        if (WIFI_PW) {
            connectPassPhrase = (WlanConnect_PassPhrase_T) WIFI_PW;
            retcode = WlanConnect_WPA(connectSSID, connectPassPhrase, NULL);
        } else
            retcode = WlanConnect_Open(connectSSID, NULL);
    }
    if (retcode == RETCODE_OK)
    {
        retcode = ServalPalSetup();
    }

    return retcode;
}

static retcode_t SubscribeToOwnPublishTopic(void)
{
    int8_t topic_buffer[40];
    strncpy((char *)topic_buffer, Topics[0].start, sizeof(topic_buffer));
    printf("Subscribing to topic: %s, Qos: %d\n\r", topic_buffer, Qos[0]);
    retcode_t rc_subscribe = Mqtt_subscribe(SessionPtr, 1, Topics, Qos);
    return rc_subscribe;
}

uint32_t GetUtcTime() {
  retcode_t rc = RC_CLOCK_ERROR_FATAL;
  uint32_t sysUpTime;
  rc = Clock_getTime(&sysUpTime);
  if (rc != RC_OK) {
    printf("Failed to get the Clock Time \r\n");
  }
  return sysUpTime + SysTime;
}



static char* receiveBufferFromSensors(void){
	int i=0;
	bool typeSensor;

    char  *buffer = calloc(1024, sizeof(char));
    char *aux;


	strcat(buffer,"{\"xdk2mam\":[");

	for(i=0;i<MAX_SENSORS_ARRAY;i++){

		typeSensor = typesSensors[i];

		if(typeSensor){

			switch(i)
		    {
				case ENVIROMENTAL:
					aux = processEnvSensorData(null,0);
					break;
				case ACCELEROMETER:
					aux = processAccelData(null,0);
					break;
				case GYROSCOPE:
					aux = processGyroData(null,0);
					break;
				case INERTIAL:
					aux = processInertiaSensor(null,0);
					break;
				case LIGHT:
					aux = processLightSensorData(null,0);
					break;
				case MAGNETOMETER:
					aux = processMagnetometerData(null,0);
					break;

		    }
			strcat(buffer,aux);
			strcat(buffer,",");
			free(aux);
		}
	}

	if(buffer[strlen(buffer)-1]==',')
		buffer[strlen(buffer)-1]=' ';

	char * deviceName = calloc(255, sizeof(char));
	char * timestamp = calloc(255, sizeof(char));
	sprintf(deviceName,"\"device\": \"%s\",",DEVICE_NAME);
	sprintf(timestamp,"\"timestamp\": \"%ld\"}",GetUtcTime());
	strcat(buffer,"],");
	strcat(buffer,deviceName);
	strcat(buffer,timestamp);
	free(deviceName);
	free(timestamp);

	return (char*)buffer;

}



static void PublishData(void *param1, uint32_t param2)
{
    BCDS_UNUSED(param1);
    BCDS_UNUSED(param2);
    retcode_t rc_publish;


    char* httpBodyBuffer =  receiveBufferFromSensors();

    rc_publish = Mqtt_publish(SessionPtr, PublishTopicDescription,httpBodyBuffer, 800, (uint8_t) MQTT_QOS_AT_MOST_ONE, false);
    if (rc_publish == RC_OK)
    {
        PublishInProgress = 1;
    }
    else
    {
        PublishInProgress = 0;
        printf("Mqtt_publish is failed:Stack erro code :%u \n\r",(unsigned int)rc_publish);
        Retcode_RaiseError(RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_MQTT_PUBLISH_FAIL));
    }
    free(httpBodyBuffer);
}

static void EnqueueMessagePublish(TimerHandle_t pxTimer)
{
    BCDS_UNUSED(pxTimer);

    if (!PublishInProgress)
    {
        Retcode_T retcode = CmdProcessor_Enqueue(AppCmdProcessor, PublishData, NULL, 0);
        if (RETCODE_OK != retcode)
        {
            printf("CmdProcessor_Enqueue is failed \n\r");
            Retcode_RaiseError(retcode);
        }
    }
}

/**
 * @brief Create and start software timer for MQTT publishing the sensor data
 *
 */
static void CreateAndStartPublishingTimer(void)
{
    PublishTimerHandle = xTimerCreate(
            (const char * const ) "Publish Timer",
            (PUBLISHTIMER_PERIOD_IN_MS/portTICK_RATE_MS),
            pdTRUE,
            NULL,
            EnqueueMessagePublish);
    if(NULL == PublishTimerHandle)
    {
        printf("xTimerCreate is failed \n\r");
        Retcode_RaiseError(RETCODE(RETCODE_SEVERITY_FATAL, RETCODE_OUT_OF_RESOURCES));
    }
    else if ( pdFAIL == xTimerStart(PublishTimerHandle, 10000))
    {
        printf("xTimerStart is failed \n\r");
    }
}

static void HandleEventConnection(MqttConnectionEstablishedEvent_T connectionData)
{
    printf("Connection Event:\n\r"
            "\tServer Return Code: %d (0 for success)\n\r"
            "\tReused Session Flag: %d\n\r",
            (int) connectionData.connectReturnCode,
            (int) connectionData.sessionPresentFlag);
    if (connectionData.connectReturnCode == 0)
    {
        retcode_t rc = SubscribeToOwnPublishTopic();
        if(RC_OK != rc)
        {
            printf("SubscribeToOwnPublishTopic is failed\n\r");
            printf("Stack error code :%u \n\r",(unsigned int)rc);
            Retcode_RaiseError(RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_MQTT_SUBSCRIBE_FAIL));
        }
    }

}

static void HandleEventIncomingPublish(
        MqttPublishData_T publishData)
{
    char published_topic_buffer[COMMON_BUFFER_SIZE];
    char published_data_buffer[COMMON_BUFFER_SIZE];
    static int incoming_message_count = 0;

    strncpy(published_data_buffer, (const char *)publishData.payload, sizeof(published_data_buffer));
    strncpy(published_topic_buffer, publishData.topic.start, sizeof(published_topic_buffer));

    printf("#%d, Incoming Published Message:\n\r"
            "\tTopic: %s\n\r"
            "\tPayload: \n\r\"\"\"\n\r%s\n\r\"\"\"\n\r", incoming_message_count,
            published_topic_buffer, published_data_buffer);
    incoming_message_count++;
}


static void HandleEventSuccessfulPublish(void *param1, uint32_t param2)
{
    BCDS_UNUSED(param1);
    BCDS_UNUSED(param2);
    PublishInProgress = 0;

}


static retcode_t EventHandler(MqttSession_T* session, MqttEvent_t event,
        const MqttEventData_t* eventData)
{
    BCDS_UNUSED(session);
    Retcode_T retcode = RETCODE_OK;
    printf("EventHandler Event : %d\n\r", (int)event);
    switch (event)
    {
    case MQTT_CONNECTION_ESTABLISHED:
        HandleEventConnection(eventData->connect);
        break;
    case MQTT_CONNECTION_ERROR:
        HandleEventConnection(eventData->connect);
        break;
    case MQTT_INCOMING_PUBLISH:
        HandleEventIncomingPublish(eventData->publish);
        break;
    case MQTT_PUBLISHED_DATA:

        retcode = CmdProcessor_Enqueue(AppCmdProcessor, HandleEventSuccessfulPublish, NULL, 0);
        if (RETCODE_OK != retcode)
        {
            printf("CmdProcessor_Enqueue is failed \n\r");
            Retcode_RaiseError(retcode);
        }
        break;
    case MQTT_SUBSCRIBE_SEND_FAILED:
        printf("MQTT_SUBSCRIBE_SEND_FAILED\n\r");
        break;
    case MQTT_SUBSCRIPTION_ACKNOWLEDGED:
        if (USE_PUBLISH_TIMER)
        {
            CreateAndStartPublishingTimer();
        }
        else
        {

            PublishData(NULL, 0);
        }
        printf("MQTT_SUBSCRIPTION_ACKNOWLEDGED\n\r");
        break;
    case MQTT_CONNECT_TIMEOUT:
        printf("MQTT_CONNECT_TIMEOUT\n\r");
        break;
    default:
        printf("Unhandled MQTT Event Number: %d\n\r", event);
        break;
    }
    return RC_OK;
}


static void Start(void)
{
    retcode_t rc = RC_INVALID_STATUS;
    rc = Mqtt_connect(SessionPtr);
    if (rc != RC_OK)
    {
        printf("Could not Connect, error 0x%04x\n", rc);
        Retcode_RaiseError(RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_MQTT_CONNECT_FAIL));
    }
}


static void ConfigureSession(void)
{
    // set target
    int8_t server_ip_buffer[13];
    Ip_Address_T ip;

    Retcode_T rc;
    rc = NetworkConfig_GetIpAddress((uint8_t *) MQTT_BROKER_HOST, &ip);
    if(RETCODE_OK != rc)
    {
        printf("Getting Broker address failure\n\r");
        Retcode_RaiseError(RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_MQTT_IPCONIG_FAIL));
    }
    else
    {
        int32_t retval = Ip_convertAddrToString(&ip, (char *)server_ip_buffer);
        if(0 == retval)
        {
            printf("Ip_convertAddrToString return value is zero\n\r");
            Retcode_RaiseError(RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_MQTT_IPCONIG_FAIL));
        }
        else
        {
            sprintf(MqttBroker, MqttBrokerAddressFormat, server_ip_buffer,
            MQTT_BROKER_PORT);

            rc = SupportedUrl_fromString((const char *)MqttBroker, (uint16_t) strlen((const char *)MqttBroker),
                    &SessionPtr->target);
            if(RC_OK != rc)
            {
                printf("SupportedUrl_fromString failure\n\r");
                Retcode_RaiseError(RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_MQTT_IPCONIG_FAIL));
            }
            else
            {
                SessionPtr->target.scheme = SERVAL_SCHEME_MQTT;

                printf("Broker address: %s\n\r", MqttBroker);

                SessionPtr->onMqttEvent = EventHandler;


                SessionPtr->MQTTVersion = 3;
                SessionPtr->keepAliveInterval = 100;
                SessionPtr->cleanSession = true;
                SessionPtr->will.haveWill = false;

                StringDescr_T username;
                StringDescr_wrap(&username, MQTT_USERNAME);
                SessionPtr->username = username;
                StringDescr_T password;
                StringDescr_wrap(&password, MQTT_PASSWORD);
                SessionPtr->password = password;

                StringDescr_T device_name_descr;
                StringDescr_wrap(&device_name_descr, DeviceName);
                SessionPtr->clientID = device_name_descr;

                StringDescr_wrap(&PublishTopicDescription, (const char *)PublishTopic);
                StringDescr_wrap(&(Topics[0]), PublishTopic);
                Qos[0] = MQTT_QOS_AT_MOST_ONE;
            }
        }
    }
}
void SetUtcTime(uint32_t utcTime) {
  retcode_t rc = RC_CLOCK_ERROR_FATAL;
  uint32_t sysUpTime;
  rc = Clock_getTime(&sysUpTime);
  if (rc != RC_OK) {
    printf("Failed to get the Clock Time \r\n");
  }
  SysTime = utcTime - sysUpTime;
}


static void ReceiveCallback(Msg_T *msg_ptr, retcode_t status) {


  unsigned int payloadLen;
  uint8_t *payload_ptr;

  if (status != RC_OK) {

  }
  XUdp_getXUdpPayload(msg_ptr, &payload_ptr, &payloadLen);



  if (payloadLen >= NTP_PACKET_SIZE) {
    uint64_t secsSince1900;
    secsSince1900 = (unsigned long)payload_ptr[40] << 24;
    secsSince1900 |= (unsigned long)payload_ptr[41] << 16;
    secsSince1900 |= (unsigned long)payload_ptr[42] << 8;
    secsSince1900 |= (unsigned long)payload_ptr[43];
    uint64_t secsSince1970 = secsSince1900 - 2208988800UL;
    printf("NTP got UTC secSince1970: %llu\n\r", secsSince1970);
    SetUtcTime(secsSince1970);
  } else {
    printf("NTP response not valid!\n\r");
  }
}


static void SendCallback(Msg_T *msg_ptr, retcode_t status) {
  BCDS_UNUSED(msg_ptr);

  printf("NTP request Sending Complete\n\r");

  if (status != RC_OK) {
    printf("Sending status not RC_OK; status=" RC_RESOLVE_FORMAT_STR "\n\r",
           RC_RESOLVE((int)status));
  }
}


void InitSntpTime() {
  uint32_t now = 0;
  retcode_t rc = RC_OK;
  Msg_T *MsgHandlePtr = NULL;
  Ip_Port_T port = Ip_convertIntToPort(
      SNTP_DEFAULT_PORT);

  rc = XUdp_initialize();
  if (rc != RC_OK) {
	 printf("FAILED TO INIT\n");
    LOG_ERROR("Failed to init XUDP; rc=" RC_RESOLVE_FORMAT_STR "\n",RC_RESOLVE((int)rc));
    return;
  }

  rc = XUdp_start(port, ReceiveCallback);
  if (rc != RC_OK) {
	  printf("FAILED TO START\n");
    LOG_ERROR("Failed to start XUDP; rc=" RC_RESOLVE_FORMAT_STR "\n",RC_RESOLVE((int)rc));
    return;
  }

  Ip_Address_T sntpIpAddress;
  uint8_t buffer[NTP_PACKET_SIZE];
  unsigned int bufferLen = NTP_PACKET_SIZE;

  /* init request: */
  memset(buffer, 0, NTP_PACKET_SIZE);
  buffer[0] = 0b11100011; /* LI, Version, Mode */ /*lint !e146 */
  buffer[1] = 0;    /* Stratum, or type of clock */
  buffer[2] = 6;    /* Polling Interval */
  buffer[3] = 0xEC; /* Peer Clock Precision */
  /* 8 bytes of zero for Root Delay & Root Dispersion */
  buffer[12] = 49;
  buffer[13] = 0x4E;
  buffer[14] = 49;
  buffer[15] = 52;

  rc = Clock_getTime(&now);
  if (RC_OK == rc) {
    /* time available, use timeout */
    const uint32_t dnsEndTime = now + NTP_DNS_TIMEOUT_IN_S;
    while (dnsEndTime > now) {
      rc = NetworkConfig_GetIpAddress((uint8_t *)SNTP_DEFAULT_ADDR,
                            (Ip_Address_T *)&sntpIpAddress);
      if (RC_OK == rc)
        break;
      vTaskDelay(NTP_DNS_RETRY_INTERVAL_IN_MS / portTICK_RATE_MS);
      if (RC_OK != Clock_getTime(&now))
        break;
    }
  } else {
    /* no time, no retry */
    rc = NetworkConfig_GetIpAddress((uint8_t *)SNTP_DEFAULT_ADDR,
                          (Ip_Address_T *)&sntpIpAddress);
  }

  /* resolve IP address of server hostname: 0.de.pool.ntp.org eg.: 176.9.104.147
   */
  if (RC_OK != rc) {
    printf("NTP Server %s could not be resolved. Use a static IP "
           "(129.6.15.28) instead!\n\r",
           SNTP_DEFAULT_ADDR);
    /* dirctly use a "0.de.pool.ntp.org" pool IP: 176.9.104.147 */
    Ip_convertOctetsToAddr(129, 6, 15, 28, &sntpIpAddress);
  }

  /* now send request: */
  rc = XUdp_push(&sntpIpAddress, SNTP_DEFAULT_PORT, buffer,
                 bufferLen, // to default NTP server port */
                 SendCallback, &MsgHandlePtr);
  if (rc != RC_OK) {
    LOG_ERROR("Sending failure; rc=" RC_RESOLVE_FORMAT_STR "\n",
              RC_RESOLVE((int)rc));
    return;
  }
  LOG_INFO("Pushed Echo\n");
  return;
}


static void Init(void)
{
    retcode_t rc_initialize = Mqtt_initialize();

    if (rc_initialize != RC_OK)
    {
        printf("Could not initialize mqtt, error 0x%04x\n", rc_initialize);
    }
    else
    {
        SessionPtr = &Session;
        memset(SessionPtr, 0, sizeof(*SessionPtr));
        rc_initialize = Mqtt_initializeInternalSession(SessionPtr);

        InitSntpTime();

        if (rc_initialize != RC_OK)
        {
            printf("Mqtt_initializeInternalSession is failed\n\r");
        }
        else
        {
        	int i=0;
			for(i=0;i<MAX_SENSORS_ARRAY;i++){
				if(typesSensors[i]){
					switch(i)
					{
						case ENVIROMENTAL:
							environmentalSensorInit();
							break;
						case ACCELEROMETER:
							accelerometerSensorInit();
							break;
						case GYROSCOPE:
							gyroscopeSensorInit();
							break;
						case INERTIAL:
							inertialSensorInit();
							break;
						case LIGHT:
							lightsensorInit();
							break;
						case MAGNETOMETER:
							magnetometerSensorInit();
							break;

					}
				}
			}
            ConfigureSession();
            Start();
        }
    }
}

void AppInitSystem(void * cmdProcessorHandle, uint32_t param2)
{
    if (cmdProcessorHandle == NULL)
    {
        printf("Command processor handle is null \n\r");
        Retcode_RaiseError(RETCODE(RETCODE_SEVERITY_FATAL,RETCODE_NULL_POINTER));
    }
    BCDS_UNUSED(param2);
    AppCmdProcessor = (CmdProcessor_T *) cmdProcessorHandle;

    Retcode_T connect_rc = NetworkSetup();
    if (connect_rc == RETCODE_OK)
    {
        Init();
    }
    else
    {
        printf("Connect Failed, so example is not started\n\r");
        Retcode_RaiseError(connect_rc);
    }

}

/** ************************************************************************* */
