#include "XDKAppInfo.h" 

#undef BCDS_MODULE_ID  /* Module ID define before including Basics package*/
#define BCDS_MODULE_ID XDK_APP_MODULE_ID_HTTP_XDK2MAM_CLIENT

/* own header files */
#include "xdk2mam.h"

/* system header files */
#include <stdio.h>

/* additional interface header files */

#include "BCDS_WlanConnect.h"
#include "BCDS_NetworkConfig.h"
#include "BCDS_CmdProcessor.h"

#include <Serval_HttpClient.h>
#include <Serval_XUdp.h>
#include <Serval_Network.h>
#include <Serval_Clock.h>
#include <Serval_Log.h>
#include "BCDS_ServalPal.h"
#include "BCDS_ServalPalWiFi.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "Serval_Http.h"
#include "EnvironmentalSensor.h"
#include "Accelerometer.h"
#include "Gyroscope.h"
#include "InertialSensor.h"
#include "LightSensor.h"
#include "Magnetometer.h"
#include "semphr.h"
#include "BCDS_SDCard_Driver.h"
#include "ff.h"
#include "fs.h"

static xTaskHandle httpGetTaskHandle;
static xTaskHandle httpPostTaskHandle;
static xTimerHandle triggerHttpRequestTimerHandle;
static uint32_t httpGetPageOffset = 0;
static CmdProcessor_T CommandProcessorHandle;
CmdProcessor_T *AppCmdProcessorHandle;
static uint32_t SysTime = UINT32_C(0);
static FIL fileObject;
static SemaphoreHandle_t semPost = NULL;

// Global array of all sensors => true : enable -- false : disable
bool typesSensors[6] = {
						true, //ENVIROMENTAL
						true, //ACCELEROMETER
						true, //GYROSCOPE
						true, //INERTIAL
						true, //LIGHT
						true  //MAGNETOMETER
					};

char* DEVICE_NAME;
char* WLAN_SSID;
char* WLAN_PSK;
char* DEST_SERVER_HOST;
char* DEST_SERVER_PORT;
char* INTER_REQUEST_INTERVAL;

static Retcode_T ServalPalSetup(void)
{
    Retcode_T returnValue = RETCODE_OK;
    returnValue = CmdProcessor_Initialize(&CommandProcessorHandle, "Serval PAL", TASK_PRIORITY_SERVALPAL_CMD_PROC, TASK_STACK_SIZE_SERVALPAL_CMD_PROC, TASK_QUEUE_LEN_SERVALPAL_CMD_PROC);
    /* serval pal common init */
    if (RETCODE_OK == returnValue)
    {
        returnValue = ServalPal_Initialize(&CommandProcessorHandle);
    }
    if (RETCODE_OK == returnValue)
    {
        returnValue = ServalPalWiFi_Init();
    }
    if (RETCODE_OK == returnValue)
    {
        ServalPalWiFi_StateChangeInfo_T stateChangeInfo = { SERVALPALWIFI_OPEN, INT16_C(0) };
        returnValue = ServalPalWiFi_NotifyWiFiEvent(SERVALPALWIFI_STATE_CHANGE, &stateChangeInfo);
    }
    return returnValue;
}


void InitSdCard(void){

	Retcode_T retVal =RETCODE_FAILURE;
	FRESULT FileSystemResult = 0;
	static FATFS FatFileSystemObject;
	SDCardDriver_Initialize();
	if(SDCARD_INSERTED== SDCardDriver_GetDetectStatus()){
		retVal = SDCardDriver_DiskInitialize(DRIVE_ZERO);
		if(RETCODE_OK == retVal){
			printf("SD Card Disk initialize succeeded \n\r");
			FileSystemResult = f_mount(&FatFileSystemObject, DEFAULT_LOGICAL_DRIVE, FORCE_MOUNT);
			if(0 != FileSystemResult){
				printf("Mounting SD card failed \n\r");
			}
		}
	}else{
		printf("SD card failed \n\r");
	}

}

Retcode_T searchForFileOnSdCard(const char * filename, FILINFO * fileData){

	if(0  == f_stat(filename, fileData)){
		printf("File %s found on SD card. \n\r"	,filename);
		return RETCODE_OK;
	}
	else{
		printf(	"File %s does not exist. \n\r"	,filename);
		return	RETCODE_FAILURE;
	}
}

void readDataFromFileOnSdCard(const char* filename){
	FRESULT fileSystemResult;
	FILINFO fileInfo;
	char bufferRead[UINT16_C(512)];
	UINT bytesRead;
	int i=0;
	DEVICE_NAME = calloc(96, sizeof(char));
	WLAN_SSID = calloc(128, sizeof(char));
	WLAN_PSK = calloc(128, sizeof(char));
	DEST_SERVER_HOST = calloc(20, sizeof(char));
	DEST_SERVER_PORT = calloc(10, sizeof(char));
	INTER_REQUEST_INTERVAL = calloc(20, sizeof(char));

	if(RETCODE_OK == searchForFileOnSdCard(filename,&fileInfo)){
		f_open(&fileObject, filename, FA_OPEN_EXISTING | FA_READ);
		f_lseek(&fileObject, FIRST_LOCATION);
        fileSystemResult = f_read(&fileObject, bufferRead, fileInfo.fsize,&bytesRead);
        if((fileSystemResult !=0) || (fileInfo.fsize != bytesRead)){
        	printf("Error: Cannot read file %s \n\r",filename);
        }
        else{
        	bufferRead[bytesRead] ='\0';
        	printf("Read data from file %s \n\r",filename);
        	int j=0,tipo=1;
        	for(i=0;i<bytesRead;i++){
        		if(bufferRead[i]=='='){
        			i++;
        			j=0;
        			while(bufferRead[i+1]!='\n'){
						switch(tipo){
							case t_DEVICE_NAME:
								DEVICE_NAME[j] = bufferRead[i];
								break;
							case t_WLAN_SSID:
								WLAN_SSID[j] = bufferRead[i];
								break;
							case t_WLAN_PSK:
								WLAN_PSK[j] = bufferRead[i];
								break;
							case t_DEST_SERVER_HOST:
								DEST_SERVER_HOST[j] = bufferRead[i];
								break;
							case t_DEST_SERVER_PORT:
								DEST_SERVER_PORT[j] = bufferRead[i];
								break;
							case t_INTER_REQUEST_INTERVAL:
								INTER_REQUEST_INTERVAL[j] = bufferRead[i];
								break;
							default:
								if(bufferRead[i]=='Y' || bufferRead[i]=='E' || bufferRead[i]=='S')
									typesSensors[tipo-7]=true;
								else
									typesSensors[tipo-7]=false;
								break;
						}
        				j++;i++;
        			}
        			tipo++;

        		}
        	}

        	printf("%s-%s-%s-%s-%s-%s-%d-%d-%d-%d-%d-%d|\n\r",DEVICE_NAME,WLAN_SSID,WLAN_PSK,DEST_SERVER_HOST,DEST_SERVER_PORT,INTER_REQUEST_INTERVAL,
        			typesSensors[0],typesSensors[1],typesSensors[2],typesSensors[3],typesSensors[4],typesSensors[5]);

        }
        f_close(&fileObject);
	}else{
		printf("No file with name %s exists on the SD card \n\r",filename);
	}
}

static Retcode_T connectToWLAN(void)
{
    Retcode_T retcode;

    retcode = WlanConnect_Init();
    if (RETCODE_OK != retcode)
    {
        return retcode;
    }

    InitSdCard();

    readDataFromFileOnSdCard(FILE_NAME);

    retcode = NetworkConfig_SetIpDhcp(NULL);
    if (RETCODE_OK != retcode)
    {
        return retcode;
    }

    printf("Connecting to %s \r\n ", WLAN_SSID);

    retcode = WlanConnect_WPA((WlanConnect_SSID_T) WLAN_SSID, (WlanConnect_PassPhrase_T) WLAN_PSK, NULL);
    if (RETCODE_OK != retcode)
    {
        return retcode;
    }

    NetworkConfig_IpSettings_T currentIpSettings;
    retcode = NetworkConfig_GetIpSettings(&currentIpSettings);
    if (RETCODE_OK != retcode)
    {
        return retcode;
    }
    else
    {
        uint32_t ipAddress = Basics_htonl(currentIpSettings.ipV4);

        char humanReadbleIpAddress[SERVAL_IP_ADDR_LEN] = { 0 };
        int conversionStatus = Ip_convertAddrToString(&ipAddress, humanReadbleIpAddress);
        if (conversionStatus < 0)
        {
            printf("Couldn't convert the IP address to string format \r\n");
        }
        else
        {
            printf("Connected to WPA network successfully \r\n");
            printf(" Ip address of the device %s \r\n", humanReadbleIpAddress);
        }
    }

    return retcode;
}

static retcode_t httpRequestSentCallback(Callable_T* caller, retcode_t callerStatus)
{
    BCDS_UNUSED(caller);

    if (RC_OK == callerStatus)
    {
        printf("httpRequestSentCallback: HTTP request sent successfully.\r\n");
    }
    else
    {
        printf("httpRequestSentCallback: HTTP request failed to send. error=%d\r\n", callerStatus);
        printf("httpRequestSentCallback: Restarting request timer\r\n");
        xTimerStart(triggerHttpRequestTimerHandle, 10);
    }

    return RC_OK;
}

static retcode_t httpGetResponseCallback(HttpSession_T *httpSession, Msg_T *httpMessage, retcode_t status)
{
    BCDS_UNUSED(httpSession);

    xTimerStart(triggerHttpRequestTimerHandle, (const TickType_t)atoi(INTER_REQUEST_INTERVAL) / portTICK_PERIOD_MS);

    if (RC_OK != status)
    {
        printf("httpGetResponseCallback: error while receiving response to GET request. error=%d\r\n", status);
        return RC_OK;
    }
    if (NULL == httpMessage)
    {
        printf("httpGetResponseCallback: received NULL as HTTP message. This should not happen.\r\n");
        return RC_OK;
    }

    Http_StatusCode_T httpStatusCode = HttpMsg_getStatusCode(httpMessage);
    if (Http_StatusCode_OK != httpStatusCode)
    {
        printf("httpGetResponseCallback: received HTTP status other than 200 OK. status=%d\r\n", httpStatusCode);
    }
    else
    {
        retcode_t retcode;
        bool isLastPartOfMessage;
        uint32_t pageContentSize;
        retcode = HttpMsg_getRange(httpMessage, UINT32_C(0), &pageContentSize, &isLastPartOfMessage);
        if (RC_OK != retcode)
        {
            printf("httpGetResponseCallback: failed to get range from message. error=%d\r\n", retcode);
        }
        else
        {
            const char* responseContent;
            unsigned int responseContentLen;
            HttpMsg_getContent(httpMessage, &responseContent, &responseContentLen);
            printf("httpGetResponseCallback: successfully received a response: %.*s\r\n", responseContentLen, responseContent);

            if (isLastPartOfMessage)
            {
                /* We're done with the GET request. Let's make a POST request. */
                printf("httpGetResponseCallback: Server is up. Triggering the POST request.\r\n");
                xTaskNotifyGive(httpPostTaskHandle);
            }
            else
            {
                /* We're not done yet downloading the page - let's make another request. */
                printf("httpGetResponseCallback: there is still more to GET. Making another request.\r\n");
                httpGetPageOffset += responseContentLen;
                xTaskNotifyGive(httpGetTaskHandle);
            }
        }
    }
    return RC_OK;
}

static void httpGetTask(void* parameter)
{
    BCDS_UNUSED(parameter);
    retcode_t retcode;
    Retcode_T retVal;
    Msg_T* httpMessage;

    while (1)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);


        Ip_Address_T destServerAddress;
        retVal = NetworkConfig_GetIpAddress((uint8_t*) DEST_SERVER_HOST, &destServerAddress);
        if (RETCODE_OK != retVal)
        {
            printf("httpGetTask: unable to resolve hostname %s. error=%d.\r\n", DEST_SERVER_HOST,retcode);
        }
        if (RETCODE_OK == retVal)
        {

            retcode = HttpClient_initRequest(&destServerAddress, Ip_convertIntToPort(atoi(DEST_SERVER_PORT)), &httpMessage);

            if (RC_OK != retcode)
            {
                printf("httpGetTask: unable to create HTTP request. error=%d.\r\n", retcode);
                retVal = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_INIT_REQUEST_FAILED);
            }
        }
        if (RETCODE_OK == retVal)
        {
            HttpMsg_setReqMethod(httpMessage, Http_Method_Get);

            HttpMsg_setContentType(httpMessage, Http_ContentType_Text_Plain);

            retcode = HttpMsg_setReqUrl(httpMessage, DEST_GET_PATH);
            if (RC_OK != retcode)
            {
                printf("httpGetTask: unable to set request URL. error=%d.\r\n", retcode);
                retVal = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SET_REQURL_FAILED);
            }
        }

        if (RETCODE_OK == retVal)
        {
            retcode = HttpMsg_setHost(httpMessage, DEST_SERVER_HOST);
            if (RC_OK != retcode)
            {
                printf("httpGetTask: unable to set HOST header. error=%d.\r\n", retcode);
                retVal = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SET_HOST_FAILED);
            }
        }

        if (RETCODE_OK == retVal)
        {
            HttpMsg_setRange(httpMessage, httpGetPageOffset, REQUEST_MAX_DOWNLOAD_SIZE);

            Callable_T httpRequestSentCallable;
            (void) Callable_assign(&httpRequestSentCallable, httpRequestSentCallback);
            retcode = HttpClient_pushRequest(httpMessage, &httpRequestSentCallable, httpGetResponseCallback);
            if (RC_OK != retcode)
            {
                printf("httpGetTask: unable to push the HTTP request. error=%d.\r\n", retcode);
                retVal = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_PUSH_REQUEST_FAILED);
            }
        }
        if (RETCODE_OK != retVal)
        {
            Retcode_RaiseError(retVal);
            xTimerStart(triggerHttpRequestTimerHandle, (const TickType_t)atoi(INTER_REQUEST_INTERVAL) / portTICK_PERIOD_MS);
        }
    }
}


static retcode_t httpPostCustomHeaderSerializer(OutMsgSerializationHandover_T* serializationHandover)
{
    if (serializationHandover == NULL)
    {
        printf("httpPostCustomHeaderSerializer: serializationHandover is NULL. This should never happen.\r\n");
        return RC_APP_ERROR;
    }

    retcode_t result = RC_OK;
    switch (serializationHandover->position)
    {
    case 0:
        result = TcpMsg_copyStaticContent(serializationHandover, POST_REQUEST_CUSTOM_HEADER_0, strlen(POST_REQUEST_CUSTOM_HEADER_0));
        if (result != RC_OK)
            return result;
        serializationHandover->position = 1;
        break;
    case 1:
        result = TcpMsg_copyContentAtomic(serializationHandover, POST_REQUEST_CUSTOM_HEADER_1, strlen(POST_REQUEST_CUSTOM_HEADER_1));
        if (result != RC_OK)
            return result;
        serializationHandover->position = 2;
        break;
    default:
        result = RC_OK;
    }
    return result;

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



static retcode_t httpPostPayloadSerializer(OutMsgSerializationHandover_T* serializationHandover)
{
    char* httpBodyBuffer = receiveBufferFromSensors();

    uint32_t offset = serializationHandover->offset;
    uint32_t bytesLeft = strlen(httpBodyBuffer) - offset;
    uint32_t bytesToCopy = serializationHandover->bufLen > bytesLeft ? bytesLeft : serializationHandover->bufLen;

    memcpy(serializationHandover->buf_ptr, httpBodyBuffer + offset, bytesToCopy);
    serializationHandover->len = bytesToCopy;
    free(httpBodyBuffer);


    if (bytesToCopy < bytesLeft)
    {
        return RC_MSG_FACTORY_INCOMPLETE;
    }
    else
    {
        return RC_OK;
    }
}

static retcode_t httpPostResponseCallback(HttpSession_T *httpSession, Msg_T *httpMessage, retcode_t status)
{
    BCDS_UNUSED(httpSession);


    if (RC_OK != status)
    {
        return RC_APP_ERROR;
    }
    if (NULL == httpMessage)
    {
        printf("httpPostResponseCallback: received NULL as HTTP message. This should not happen.\r\n");
        return RC_APP_ERROR;
    }

    Http_StatusCode_T httpStatusCode = HttpMsg_getStatusCode(httpMessage);
    if (Http_StatusCode_OK != httpStatusCode)
    {
        printf("httpPostResponseCallback: received HTTP status other than 200 OK. status=%d\r\n", httpStatusCode);
    }
    else
    {
        retcode_t retcode;
        bool isLastPartOfMessage;
        uint32_t pageContentSize;
        retcode = HttpMsg_getRange(httpMessage, UINT32_C(0), &pageContentSize, &isLastPartOfMessage);
        if (RC_OK != retcode)
        {
            printf("httpPostResponseCallback: failed to get range from message. error=%d\r\n", retcode);
        }
        else
        {
            const char* responseContent;
            unsigned int responseContentLen;
            HttpMsg_getContent(httpMessage, &responseContent, &responseContentLen);
            printf("httpPostResponseCallback: successfully received a response: %.*s\r\n", responseContentLen, responseContent);

            if (!isLastPartOfMessage)
            {
                printf("httpPostResponseCallback: server response was too large. This example application does not support POST responses larger than %lu.\r\n", REQUEST_MAX_DOWNLOAD_SIZE);
            }

            printf("httpPostResponseCallback: POST request is done. Restarting request timer.\r\n");

        }
    }

    return RC_OK;
}



static void httpPostTask(void* parameter)
{
    BCDS_UNUSED(parameter);
    retcode_t retcode;
    Retcode_T retVal;
    Msg_T* httpMessage;
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

   while (1)
    {
	   	xSemaphoreTake(semPost,(const TickType_t)atoi(INTER_REQUEST_INTERVAL));

        Ip_Address_T destServerAddress;
        retVal = NetworkConfig_GetIpAddress((uint8_t*) DEST_SERVER_HOST, &destServerAddress);
        if (RETCODE_OK != retVal)
        {
            printf("httpPostTask: unable to resolve hostname %s. error=%d.\r\n", DEST_SERVER_HOST, (int) retVal);
        }
        if (RETCODE_OK == retVal)
        {
            retcode = HttpClient_initRequest(&destServerAddress, Ip_convertIntToPort(atoi(DEST_SERVER_PORT)), &httpMessage);

            if (RC_OK != retcode)
            {
                printf("httpPostTask: unable to create HTTP request. error=%d.\r\n", retcode);
                retVal = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_INIT_REQUEST_FAILED);
            }
        }
        if (RETCODE_OK == retVal)
        {
            HttpMsg_setReqMethod(httpMessage, Http_Method_Post);

            HttpMsg_setContentType(httpMessage, Http_ContentType_App_Json);

            retcode = HttpMsg_setReqUrl(httpMessage, DEST_POST_PATH);
            if (RC_OK != retcode)
            {
                printf("httpPostTask: unable to set request URL. error=%d.\r\n", retcode);
                retVal = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SET_REQURL_FAILED);
            }
        }
        if (RETCODE_OK == retVal)
        {
            retcode = HttpMsg_setHost(httpMessage, DEST_SERVER_HOST);
            if (RC_OK != retcode)
            {
                printf("httpPostTask: unable to set HOST header. error=%d.\r\n", retcode);
                retVal = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_SET_HOST_FAILED);
            }
        }
        if (RETCODE_OK == retVal)
        {
            HttpMsg_setRange(httpMessage, httpGetPageOffset, REQUEST_MAX_DOWNLOAD_SIZE);

            HttpMsg_serializeCustomHeaders(httpMessage, httpPostCustomHeaderSerializer);

            retcode = TcpMsg_prependPartFactory(httpMessage, httpPostPayloadSerializer);
            if (RC_OK != retcode)
            {
                printf("httpPostTask: unable to serialize request body. error=%d.\r\n", retcode);
                retVal = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_FAILURE);
            }
        }
        if (RETCODE_OK == retVal)
        {
            Callable_T httpRequestSentCallable;
            (void) Callable_assign(&httpRequestSentCallable, httpRequestSentCallback);
            retcode = HttpClient_pushRequest(httpMessage, &httpRequestSentCallable, httpPostResponseCallback);
            if (RC_OK != retcode)
            {
                printf("httpPostTask: unable to push the HTTP request. error=%d.\r\n", retcode);
                retVal = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_PUSH_REQUEST_FAILED);
            }
        }

        if (RETCODE_OK != retVal)
        {
            Retcode_RaiseError(retVal);
        }
    }
}

static void triggerHttpRequestTimerCallback(TimerHandle_t timer)
{
    BCDS_UNUSED(timer);
    httpGetPageOffset = 0;

    //todo

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
    /* convert 4 bytes starting at location 40 to a long integer */
    secsSince1900 = (unsigned long)payload_ptr[40] << 24;
    secsSince1900 |= (unsigned long)payload_ptr[41] << 16;
    secsSince1900 |= (unsigned long)payload_ptr[42] << 8;
    secsSince1900 |= (unsigned long)payload_ptr[43];
    /* subtract 70 years 2208988800UL; (Unix: starting from 1970) */
    uint64_t secsSince1970 = secsSince1900 - 2208988800UL; /* UTC else + timeZone*SECS_PER_HOUR; */
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
      SNTP_DEFAULT_PORT); // also used for local ntp client! */

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


void appInitSystem(void* cmdProcessorHandle, uint32_t param2)
{

    BCDS_UNUSED(param2);
    Retcode_T returnValue = RETCODE_OK;
    AppCmdProcessorHandle = (CmdProcessor_T *) cmdProcessorHandle;


    semPost = xSemaphoreCreateBinary();

    retcode_t rc = RC_OK;
    rc = connectToWLAN();
    if (RC_OK != rc)
    {
        printf("appInitSystem: network init/connection failed. error=%d\r\n", rc);
        return;
    }

    printf("ServalPal Setup\r\n");
    returnValue = ServalPalSetup();
    if (RETCODE_OK != returnValue)
    {
        Retcode_RaiseError(returnValue);
        printf("ServalPal Setup failed with %d \r\n", (int) returnValue);
        return;
    }

    rc = HttpClient_initialize();
    if (RC_OK != rc)
    {
        printf("Failed to initialize http client \r\n ");
        return;
    }

    InitSntpTime();

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

	BaseType_t taskCreated;

	taskCreated = xTaskCreate(httpPostTask, "XDK2MAM Post", TASK_STACK_SIZE_HTTP_REQ, NULL, TASK_PRIO_HTTP_REQ, &httpPostTaskHandle);
	if (taskCreated != pdTRUE)
	{
		printf("Failed to create the POST request task\r\n");
		return;
	}

	triggerHttpRequestTimerHandle = xTimerCreate("triggerXDK2MAMRequestTimer", (const TickType_t)atoi(INTER_REQUEST_INTERVAL) / portTICK_RATE_MS, pdFALSE, NULL, triggerHttpRequestTimerCallback);
	if (triggerHttpRequestTimerHandle == NULL)
	{
		printf("Failed to create the triggerRequestTimer\r\n");
		return;
	}
	BaseType_t requestTimerStarted = xTimerStart(triggerHttpRequestTimerHandle, (const TickType_t)atoi(INTER_REQUEST_INTERVAL) / portTICK_RATE_MS);
	if (requestTimerStarted == pdFALSE)
	{
		printf("Failed to start the triggerRequestTimer\r\n");
	}

	printf("Connected to network. First HTTP request will be made soon.\r\n");
	xTaskNotifyGive(httpPostTaskHandle);


}

CmdProcessor_T * GetAppCmdProcessorHandle(void)
{
    return AppCmdProcessorHandle;
}


