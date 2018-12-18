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
#include "BCDS_BlePeripheral.h"
#include "BCDS_BidirectionalService.h"



static xTaskHandle bleTaskHandle;
static xTimerHandle triggerRequestTimerHandle;
static uint32_t httpGetPageOffset = 0;
static CmdProcessor_T CommandProcessorHandle;
CmdProcessor_T *AppCmdProcessorHandle;

static FIL fileObject;
static SemaphoreHandle_t semPost = NULL;


//BLE
static SemaphoreHandle_t BleStartSyncSemphr = NULL;
static SemaphoreHandle_t BleWakeUpSyncSemphr = NULL;


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
char* INTER_REQUEST_INTERVAL;
char* INTERVAL_STREAM_DIVIDER_BLE;


static void dataReceived(uint8_t *rxBuffer,uint8_t rxDataLenght){
	uint8_t receiveBuffer[UINT8_C(24)];
	memset(receiveBuffer,0,sizeof(receiveBuffer));
	memcpy(receiveBuffer,rxBuffer,rxDataLenght);
	printf("Received data: %s \n\r",receiveBuffer);
}


static void dataSent(Retcode_T sendStatus){
	BCDS_UNUSED(sendStatus);
	//xSemaphoreGive(SendCompleteSyncSemphr);
}

static Retcode_T initializeAndRegisterService(void){
	BidirectionalService_Init(dataReceived,dataSent);
	BidirectionalService_Register();

	return RETCODE_OK;
}

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
	INTER_REQUEST_INTERVAL = calloc(20, sizeof(char));
	INTERVAL_STREAM_DIVIDER_BLE = calloc(20, sizeof(char));

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
        	for(i=0;i<(int)bytesRead;i++){
        		if(bufferRead[i]=='='){
        			i++;
        			j=0;
        			while(bufferRead[i+1]!='\n'){
						switch(tipo){
							case t_DEVICE_NAME:
								DEVICE_NAME[j] = bufferRead[i];
								break;
							case t_INTER_REQUEST_INTERVAL:
								INTER_REQUEST_INTERVAL[j] = bufferRead[i];
								break;
							case t_INTERVAL_STREAM_DIVIDER_BLE:
								INTERVAL_STREAM_DIVIDER_BLE[j] = bufferRead[i];
								break;
							default:
								if(bufferRead[i]=='Y' || bufferRead[i]=='E' || bufferRead[i]=='S')
									typesSensors[tipo-4]=true;
								else
									typesSensors[tipo-4]=false;
								break;
						}
        				j++;i++;
        			}
        			tipo++;

        		}
        	}

        	printf("%s-%s-%d-%d-%d-%d-%d-%d|\n\r",DEVICE_NAME,INTER_REQUEST_INTERVAL,
        			typesSensors[0],typesSensors[1],typesSensors[2],typesSensors[3],typesSensors[4],typesSensors[5]);

        }
        f_close(&fileObject);
	}else{
		printf("No file with name %s exists on the SD card \n\r",filename);
	}
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
	sprintf(deviceName,"\"device\": \"%s\",\"timestamp\": ",DEVICE_NAME);
	strcat(buffer,"],");
	strcat(buffer,deviceName);
	free(deviceName);

	return (char*)buffer;

}


static void bleSendData(void* parameter)
{
    BCDS_UNUSED(parameter);
    Retcode_T retVal;
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    int i=0,c=0;
    char *aux = calloc(23, sizeof(char));
    while (1)
    {
	   	xSemaphoreTake(semPost,(const TickType_t)atoi(INTER_REQUEST_INTERVAL));

	   	char *buff = receiveBufferFromSensors();

	   	printf("Datos a enviar: %s \n\r",buff);
	   	printf("Sizeof: %d \n\r",strlen(buff));
	   	printf("Interval: %d \n\r",atoi(INTERVAL_STREAM_DIVIDER_BLE));

	   	for(i=0;i<(int)strlen(buff);i++){
	   		aux[c] = buff[i];
	   		c++;
	   		if(c==20 || (int)strlen(buff)-1==i){
	   			retVal = BidirectionalService_SendData((uint8_t *) aux, 20);
	   			free(aux);
	   			aux= calloc(23, sizeof(char));
	   			c=0;
	   			vTaskDelay((const TickType_t)atoi(INTERVAL_STREAM_DIVIDER_BLE));
	   		}
	   	}
	   	retVal = BidirectionalService_SendData((uint8_t *) "@@@_finish_data_@@@", 19);

        if (RETCODE_OK != retVal)
        {
            Retcode_RaiseError(retVal);
        }else
        {
        	//xSemaphoreTake(SendCompleteSyncSemphr,BLE_SEND_TIMEOUT);
        }



        free(buff);
    }
}

static void triggerRequestTimerCallback(TimerHandle_t timer)
{
    BCDS_UNUSED(timer);
    httpGetPageOffset = 0;
    //ToDo
}

static void handleEvent(BlePeripheral_Event_T event, void *data) {
	switch(event) {
	case BLE_PERIPHERAL_STARTED:
		printf("handleEvent : BLE powered ON successfully \r\n");
		xSemaphoreGive( BleStartSyncSemphr );
		break;
    case BLE_PERIPHERAL_WAKEUP_SUCCEEDED:
		xSemaphoreGive( BleWakeUpSyncSemphr );
		break;
    case BLE_PERIPHERAL_CONNECTED :
    	printf("handleEvent : Device connected \r\n");
    	Ble_RemoteDeviceAddress_T *remoteAddress;
        remoteAddress = (Ble_RemoteDeviceAddress_T*) data;
        printf("Device connected: %02x:%02x:%02x:%02x:%02x:%02x\n\r",
                    remoteAddress->Addr[0], remoteAddress->Addr[1],
                    remoteAddress->Addr[2], remoteAddress->Addr[3],
                    remoteAddress->Addr[4], remoteAddress->Addr[5]);

        break;
    case BLE_PERIPHERAL_DISCONNECTED:
    	printf("handleEvent: Device Disconnected\n\r");
    	break;
    default:
    	break;
    }
}


void appInitSystem(void* cmdProcessorHandle, uint32_t param2)
{

    BCDS_UNUSED(param2);
    Retcode_T returnValue = RETCODE_OK;
    AppCmdProcessorHandle = (CmdProcessor_T *) cmdProcessorHandle;


    semPost = xSemaphoreCreateBinary();

    BleStartSyncSemphr = xSemaphoreCreateBinary();
    BleWakeUpSyncSemphr = xSemaphoreCreateBinary();
    BleStartSyncSemphr = xSemaphoreCreateBinary();

    retcode_t rc = RC_OK;

    InitSdCard();

    readDataFromFileOnSdCard(FILE_NAME);

    printf("ServalPal Setup\r\n");
    returnValue = ServalPalSetup();
    if (RETCODE_OK != returnValue)
    {
        Retcode_RaiseError(returnValue);
        printf("ServalPal Setup failed with %d \r\n", (int) returnValue);
        return;
    }

    if (RETCODE_OK == rc)
	{
    	rc = BlePeripheral_Initialize(handleEvent, initializeAndRegisterService);
	}
	if (RETCODE_OK == rc)
	{
		rc = BlePeripheral_SetDeviceName((uint8_t*) DEVICE_NAME);
	}

	if(rc == BlePeripheral_Start()){
		xSemaphoreTake(BleStartSyncSemphr,BLE_START_SYNC_TIMEOUT);
	}
	if(rc == BlePeripheral_Wakeup()){
		xSemaphoreTake(BleWakeUpSyncSemphr,BLE_WAKEUP_SYNC_TIMEOUT);
	}

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

	taskCreated = xTaskCreate(bleSendData, "XDK2MAM BLE", TASK_STACK_SIZE_REQ, NULL, TASK_PRIO_REQ, &bleTaskHandle);
	if (taskCreated != pdTRUE)
	{
		printf("Failed to create the BLE request task\r\n");
		return;
	}

	triggerRequestTimerHandle = xTimerCreate("triggerXDK2MAMRequestTimer", (const TickType_t)atoi(INTER_REQUEST_INTERVAL) / portTICK_RATE_MS, pdFALSE, NULL, triggerRequestTimerCallback);
	if (triggerRequestTimerHandle == NULL)
	{
		printf("Failed to create the triggerRequestTimer\r\n");
		return;
	}
	BaseType_t requestTimerStarted = xTimerStart(triggerRequestTimerHandle, (const TickType_t)atoi(INTER_REQUEST_INTERVAL) / portTICK_RATE_MS);
	if (requestTimerStarted == pdFALSE)
	{
		printf("Failed to start the triggerRequestTimer\r\n");
	}

	printf("Connected to network.\r\n");
	xTaskNotifyGive(bleTaskHandle);


}

CmdProcessor_T * GetAppCmdProcessorHandle(void)
{
    return AppCmdProcessorHandle;
}


