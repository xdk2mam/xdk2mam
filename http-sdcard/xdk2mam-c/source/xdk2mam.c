#include "XDKAppInfo.h"

#undef BCDS_MODULE_ID  /* Module ID define before including Basics package*/
#define BCDS_MODULE_ID XDK_APP_MODULE_ID_HTTP_XDK2MAM_CLIENT

/* own header files */
#include "xdk2mam.h"

/* system header files */
#include <stdio.h>

/* additional interface header files */

#include "XDK_WLAN.h"
#include "XDK_ServalPAL.h"
#include "XDK_HTTPRestClient.h"
#include "XDK_SNTP.h"
#include "BCDS_BSP_Board.h"
#include "BCDS_NetworkConfig.h"
#include "BCDS_CmdProcessor.h"
#include "BCDS_Assert.h"
#include "XDK_Utils.h"
#include "FreeRTOS.h"
#include "task.h"


#include <Serval_Clock.h>
#include <Serval_Log.h>
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

static xTaskHandle httpPostTaskHandle;
static xTimerHandle triggerHttpRequestTimerHandle;
static uint32_t httpGetPageOffset = 0;
static CmdProcessor_T CommandProcessorHandle;
CmdProcessor_T *AppCmdProcessorHandle;
static CmdProcessor_T * AppCmdProcessor;
static uint32_t SysTime = UINT32_C(0);
static FIL fileObject;
static SemaphoreHandle_t semPost = NULL;

char* DEVICE_NAME = NULL;
char* WLAN_SSID = NULL;
char* WLAN_PSK = NULL;
char* DEST_SERVER_HOST = NULL;
char* DEST_SERVER_PORT = NULL;
char* INTER_REQUEST_INTERVAL = NULL;
char* DEST_POST_PATH = NULL;
HTTPRestClient_Config_T HTTPRestClientConfigInfo;


// Global array of all sensors => true : enable -- false : disable
bool typesSensors[7] = {
						true, //ENVIROMENTAL
						true, //ACCELEROMETER
						true, //GYROSCOPE
						true, //INERTIAL
						true, //LIGHT
						true, //MAGNETOMETER
						true  //ACOUSTIC
					};




Retcode_T InitSdCard(void){

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
	return retVal;

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
	DEST_POST_PATH = calloc(20, sizeof(char));

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
							case t_DEST_POST_PATH:
								DEST_POST_PATH[j] = bufferRead[i];
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
				case ACOUSTIC:
					aux = processAcousticData(null,0);
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



static void AppControllerValidateWLANConnectivity(void)
{
    Retcode_T retcode = RETCODE_OK;
    NetworkConfig_IpStatus_T ipStatus = NETWORKCONFIG_IP_NOT_ACQUIRED;
    NetworkConfig_IpSettings_T ipAddressOnGetStatus;

    ipStatus = NetworkConfig_GetIpStatus();
    if (ipStatus == NETWORKCONFIG_IPV4_ACQUIRED)
    {
        retcode = NetworkConfig_GetIpSettings(&ipAddressOnGetStatus);
        if ((RETCODE_OK == retcode) && (UINT32_C(0) == (ipAddressOnGetStatus.ipV4)))
        {
            /* Our IP configuration is corrupted somehow in this case. No use in proceeding further. */
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NODE_IPV4_IS_CORRUPTED);
        }
    }
    else
    {
        /* Our network connection is lost. No use in proceeding further. */
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NODE_WLAN_CONNECTION_IS_LOST);
    }
    if (RETCODE_OK != retcode)
    {
        Retcode_RaiseError(retcode);
        printf("AppControllerValidateWLANConnectivity : Resetting the device. Check if network is available. Node will do a soft reset in 10 seconds.\r\n\r\n");
        vTaskDelay(pdMS_TO_TICKS(10000));
        BSP_Board_SoftReset();
        assert(false); /* Code must not reach here */
    }
}

static void httpPostTask(void* parameter)
{
    BCDS_UNUSED(parameter);
    retcode_t retcode;
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    char* httpBodyBuffer;

   while (1)
    {
	   	xSemaphoreTake(semPost,(const TickType_t)atoi(INTER_REQUEST_INTERVAL));

	   	AppControllerValidateWLANConnectivity();

	   	httpBodyBuffer = receiveBufferFromSensors();

	   	printf("--%s\n\r",httpBodyBuffer);

	   	HTTPRestClient_Post_T HTTPRestClientPostInfo =
        {
                .Payload = httpBodyBuffer,
                .PayloadLength = strlen(httpBodyBuffer),
                .Url = DEST_POST_PATH,
                .RequestCustomHeader0 = POST_REQUEST_CUSTOM_HEADER_0,
				.RequestCustomHeader1 = POST_REQUEST_CUSTOM_HEADER_1
        }; //< HTTP rest client POST parameters


	   	retcode = HTTPRestClient_Post(&HTTPRestClientConfigInfo, &HTTPRestClientPostInfo, APP_RESPONSE_FROM_HTTP_SERVER_POST_TIMEOUT);

        if (RETCODE_OK != retcode)
        {
            Retcode_RaiseError(retcode);
        }
        free(httpBodyBuffer);
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

void AppController_Init(void * cmdProcessorHandle, uint32_t param2)
{
    BCDS_UNUSED(param2);
    Retcode_T retcode = RETCODE_OK;

    if (cmdProcessorHandle == NULL)
    {
        printf("AppController_Init : Command processor handle is NULL \r\n");
        retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER);
    }
    else
    {
        AppCmdProcessor = (CmdProcessor_T *) cmdProcessorHandle;
        retcode = CmdProcessor_Enqueue(AppCmdProcessor, appInitSystem, NULL, UINT32_C(0));
    }

    if (RETCODE_OK != retcode)
    {
        Retcode_RaiseError(retcode);
        assert(0); /* To provide LED indication for the user */
    }
}


void appInitSystem(void* cmdProcessorHandle, uint32_t param2)
{
	Retcode_T rt = RETCODE_FAILURE;

	rt = InitSdCard();

	if(rt != RETCODE_FAILURE)
		readDataFromFileOnSdCard(FILE_NAME);
	

	WLAN_Setup_T WLANSetupInfo =
	        {
	                .IsEnterprise = false,
	                .IsHostPgmEnabled = false,
	                .SSID = WLAN_SSID,
	                .Username = WLAN_PSK,
	                .Password = WLAN_PSK,
	                .IsStatic = 0,
	                .IpAddr = XDK_NETWORK_IPV4(0, 0, 0, 0),
	                .GwAddr = XDK_NETWORK_IPV4(0, 0, 0, 0),
	                .DnsAddr = XDK_NETWORK_IPV4(0, 0, 0, 0),
	                .Mask = XDK_NETWORK_IPV4(0, 0, 0, 0),
	        };

	HTTPRestClient_Setup_T HTTPRestClientSetupInfo =
	        {
	                .IsSecure = HTTP_SECURE_ENABLE,
	        };

	HTTPRestClientConfigInfo.IsSecure = HTTP_SECURE_ENABLE;
	HTTPRestClientConfigInfo.DestinationServerUrl = DEST_SERVER_HOST;
	HTTPRestClientConfigInfo.DestinationServerPort = atoi(DEST_SERVER_PORT);
	HTTPRestClientConfigInfo.RequestMaxDownloadSize = REQUEST_MAX_DOWNLOAD_SIZE;

    BCDS_UNUSED(param2);
    Retcode_T returnValue = RETCODE_OK;
    AppCmdProcessorHandle = (CmdProcessor_T *) cmdProcessorHandle;


    semPost = xSemaphoreCreateBinary();

    retcode_t rc = RC_OK;
    rc = WLAN_Setup(&WLANSetupInfo);
    if (RC_OK != rc)
    {
        printf("appInitSystem: network init/connection failed. error=%d\r\n", rc);
        return;
    }
    printf("ServalPal Setup\r\n");
    returnValue = ServalPAL_Setup(AppCmdProcessorHandle);

	if (RETCODE_OK == returnValue)
	{
		returnValue = HTTPRestClient_Setup(&HTTPRestClientSetupInfo);
	}

    if (RETCODE_OK != returnValue)
    {
        Retcode_RaiseError(returnValue);
        printf("ServalPal Setup failed with %d \r\n", (int) returnValue);
        return;
    }

    Retcode_T retcode = WLAN_Enable();
    if (RETCODE_OK == retcode)
    {
        retcode = ServalPAL_Enable();
    }
	if (RETCODE_OK == retcode)
	{
	   retcode = HTTPRestClient_Enable();
	}
    if (RETCODE_OK != retcode){
    	if(rt!= RETCODE_OK){
    		printf("SD card failed!!! \n\r");
    		return;
    	}
    	Retcode_RaiseError(retcode);
		printf("WLAN_Enable failed with %d \r\n", (int) retcode);
		return;
    }
    printf("Connecting to %s \r\n ", WLAN_SSID);
    rc = HTTPRestClient_Setup(&HTTPRestClientSetupInfo);
    if (RETCODE_OK != rc){
       	Retcode_RaiseError(rc);
   		printf("HTTPRestClient_Setup failed with %d \r\n", (int) rc);
   		return;
    }
    rc = HTTPRestClient_Enable();
    if (RETCODE_OK != rc){
		Retcode_RaiseError(rc);
		printf("HTTPRestClient_Enable failed with %d \r\n", (int) rc);
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
				case ACOUSTIC:
					acousticSensorInit();
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


