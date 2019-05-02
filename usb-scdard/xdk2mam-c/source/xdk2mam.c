#include "XDKAppInfo.h"

#undef BCDS_MODULE_ID  /* Module ID define before including Basics package*/
#define BCDS_MODULE_ID XDK_APP_MODULE_ID_USB_XDK2MAM_CLIENT

/* own header files */
#include "xdk2mam.h"

/* system header files */
#include <stdio.h>

/* additional interface header files */


#include "BCDS_BSP_Board.h"
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

static xTaskHandle usbTaskHandle;
static xTimerHandle triggerUSBTimerHandle;
static uint32_t httpGetPageOffset = 0;
CmdProcessor_T *AppCmdProcessorHandle;
static CmdProcessor_T * AppCmdProcessor;
static uint32_t SysTime = UINT32_C(0);
static FIL fileObject;
static SemaphoreHandle_t semPost = NULL;

char* DEVICE_NAME = NULL;
char* INTER_REQUEST_INTERVAL = NULL;

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
							case t_INTER_REQUEST_INTERVAL:
								INTER_REQUEST_INTERVAL[j] = bufferRead[i];
								break;
							default:
								if(bufferRead[i]=='Y' || bufferRead[i]=='E' || bufferRead[i]=='S')
									typesSensors[tipo-2]=true;
								else
									typesSensors[tipo-2]=false;
								break;
						}
        				j++;i++;
        			}
        			tipo++;

        		}
        	}
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
	sprintf(deviceName,"\"device\": \"%s\"}",DEVICE_NAME);
	strcat(buffer,"],");
	strcat(buffer,deviceName);
	free(deviceName);

	return (char*)buffer;

}

static void usbTask(void* parameter)
{
    BCDS_UNUSED(parameter);
    retcode_t retcode;
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    char* usbBuffer;

   while (1)
    {
	   	xSemaphoreTake(semPost,(const TickType_t)atoi(INTER_REQUEST_INTERVAL));

	   	usbBuffer = receiveBufferFromSensors();

	   	printf("%s\n\r",usbBuffer);

        free(usbBuffer);
    }
}

static void triggerHttpRequestTimerCallback(TimerHandle_t timer)
{
    BCDS_UNUSED(timer);
    httpGetPageOffset = 0;
    return;
    //todo

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

    BCDS_UNUSED(param2);
    AppCmdProcessorHandle = (CmdProcessor_T *) cmdProcessorHandle;


    semPost = xSemaphoreCreateBinary();

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

	taskCreated = xTaskCreate(usbTask, "XDK2MAM USB", UINT16_C(700), NULL, UINT32_C(2), &usbTaskHandle);
	if (taskCreated != pdTRUE)
	{
		printf("Failed to create the POST request task\r\n");
		return;
	}

	triggerUSBTimerHandle = xTimerCreate("triggerXDK2MAMRequestTimer", (const TickType_t)atoi(INTER_REQUEST_INTERVAL) / portTICK_RATE_MS, pdFALSE, NULL, triggerHttpRequestTimerCallback);
	if (triggerUSBTimerHandle == NULL)
	{
		printf("Failed to create the triggerRequestTimer\r\n");
		return;
	}
	BaseType_t requestTimerStarted = xTimerStart(triggerUSBTimerHandle, (const TickType_t)atoi(INTER_REQUEST_INTERVAL) / portTICK_RATE_MS);
	if (requestTimerStarted == pdFALSE)
	{
		printf("Failed to start the triggerRequestTimer\r\n");
	}


	xTaskNotifyGive(usbTaskHandle);


}

CmdProcessor_T * GetAppCmdProcessorHandle(void)
{
    return AppCmdProcessorHandle;
}


