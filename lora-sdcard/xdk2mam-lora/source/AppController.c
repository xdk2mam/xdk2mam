/* module includes ********************************************************** */

#include "XdkAppInfo.h"
#undef BCDS_MODULE_ID  /* Module ID define before including Basics package*/
#define BCDS_MODULE_ID XDK_APP_MODULE_ID_APP_CONTROLLER

/* own header files */
#include "AppController.h"

#include <Serval_Clock.h>
#include <Serval_Log.h>
#include "FreeRTOS.h"
#include "timers.h"
#include "Serval_Http.h"
#include "semphr.h"
#include "BCDS_SDCard_Driver.h"
#include "ff.h"
#include "fs.h"
#include "ctype.h"


/* system header files */
#include <stdio.h>

/* additional interface header files */
#include "BCDS_Assert.h"
#include "BCDS_CmdProcessor.h"
#include "XDK_LED.h"
#include "BSP_BoardType.h"
#include "XDK_CayenneLPPSerializer.h"
#include "XDK_Sensor.h"
#include "XDK_LoRa.h"
#include "FreeRTOS.h"
#include "XDK_Utils.h"
#include "task.h"
#include "semphr.h"
#include "BCDS_SDCard_Driver.h"
#include "ff.h"
#include "fs.h"


/* constant definitions ***************************************************** */
#define APP_TEMPERATURE_OFFSET_CORRECTION        (-3459)/**< Macro for static temperature offset correction. Self heating, temperature correction factor */

#define TEMPERATURE_DATA_CH      0x01
#define HUMIDITY_DATA_CH         0x02
#define PRESSURE_DATA_CH         0x03
#define ILLUMINANCE_DATA_CH      0x04
#define ACCELEROMETER_DATA_CH    0x05
#define GYROSCOPE_DATA_CH    	 0x06
#define MAGNETOMETER_DATA_CH     0x07

static FIL fileObject;

char* APP_LORA_RX_WINDOW_FREQ = NULL;
char* APP_LORA_FREQUENCY	 = NULL;
char* APP_CONTROLLER_LORA_TX_DELAY = NULL;
char* APP_LORA_APP_EUI = NULL;
char* APP_LORA_APP_KEY = NULL;


// Global array of all sensors => true : enable -- false : disable
bool typesSensors[4] = {
						true, //ENVIROMENTAL
						true, //ACCELEROMETER
						true, //GYROSCOPE
						true, //LIGHT
					};

/* local variables ********************************************************** */
/**< AppKey is the data encryption key used to "encode" the messages between the end nodes and the Application Server */
uint8_t AppKey[16];


uint64_t  appEUI;

Retcode_T InitSdCard(void){

	Retcode_T retVal =RETCODE_FAILURE;
	FRESULT FileSystemResult = 0;
	static FATFS FatFileSystemObject;
	SDCardDriver_Initialize();
	if(SDCARD_INSERTED== SDCardDriver_GetDetectStatus()){
		retVal = SDCardDriver_DiskInitialize(DRIVE_ZERO);
		if(RETCODE_OK == retVal){
			printf("SD Card Disk initialize succeeded \r\n");
			FileSystemResult = f_mount(&FatFileSystemObject, DEFAULT_LOGICAL_DRIVE, FORCE_MOUNT);
			if(0 != FileSystemResult){
				printf("Mounting SD card failed \r\n");
			}
		}
	}else{
		printf("SD card failed \r\n");
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

	APP_LORA_RX_WINDOW_FREQ = calloc(10, sizeof(char));
	APP_LORA_FREQUENCY	 = calloc(10, sizeof(char));
	APP_CONTROLLER_LORA_TX_DELAY = calloc(20, sizeof(char));
	APP_LORA_APP_EUI = calloc(30, sizeof(char));
	APP_LORA_APP_KEY = calloc(96, sizeof(char));

	char * aux_lora_key = calloc(10, sizeof(char));
	int lo = 0,aux_lo=0;

	if(RETCODE_OK == searchForFileOnSdCard(filename,&fileInfo)){
		f_open(&fileObject, filename, FA_OPEN_EXISTING | FA_READ);
		f_lseek(&fileObject, FIRST_LOCATION);
        fileSystemResult = f_read(&fileObject, bufferRead, fileInfo.fsize,&bytesRead);
        if((fileSystemResult !=0) || (fileInfo.fsize != bytesRead)){
        	printf("Error: Cannot read file %s \r\n",filename);
        }
        else{
        	bufferRead[bytesRead] ='\0';
        	printf("Read data from file %s \r\n",filename);
        	int j=0,tipo=1;
        	for(i=0;i<bytesRead;i++){
        		if(bufferRead[i]=='='){
        			i++;
        			j=0;
        			while(bufferRead[i+1]!='\n'){
						switch(tipo){
							case t_APP_LORA_RX_WINDOW_FREQ:
								APP_LORA_RX_WINDOW_FREQ[j] = bufferRead[i];
								break;
							case t_APP_LORA_FREQUENCY:
								APP_LORA_FREQUENCY[j] = bufferRead[i];
								break;
							case t_APP_CONTROLLER_LORA_TX_DELAY:
								APP_CONTROLLER_LORA_TX_DELAY[j] = bufferRead[i];
								break;
							case t_APP_LORA_APP_EUI:
								APP_LORA_APP_EUI[j] = bufferRead[i];
								break;
							case t_APP_LORA_APP_KEY:
								APP_LORA_APP_KEY[j] = bufferRead[i];
								if(bufferRead[i]!=','){
									aux_lora_key[aux_lo] = bufferRead[i];
									aux_lo++;
								}else{
									AppKey[lo] = strtol(aux_lora_key, NULL, 16);
									lo++;
									aux_lo=0;
									memset(aux_lora_key, 0x00, 10);
								}

								if(bufferRead[i+2]=='\n')
									AppKey[lo] = strtol(aux_lora_key, NULL, 16);

								break;
							default:
								if(bufferRead[i]=='Y' || bufferRead[i]=='E' || bufferRead[i]=='S')
									typesSensors[tipo-6]=true;
								else
									typesSensors[tipo-6]=false;
								break;
						}
        				j++;i++;
        			}
        			tipo++;

        		}
        	}

        	printf("%s-%s-%s-%s-%s-%d-%d-%d-%d|\r\n",
        			APP_LORA_RX_WINDOW_FREQ,
        			APP_LORA_FREQUENCY,
					APP_CONTROLLER_LORA_TX_DELAY,
					APP_LORA_APP_EUI,
					APP_LORA_APP_KEY,
        			typesSensors[0],typesSensors[1],typesSensors[2],typesSensors[3]);

        }
        f_close(&fileObject);
	}else{
		printf("No file with name %s exists on the SD card \r\n",filename);
	}
}







/**
 * @brief This is the LoRa event Notification callback function.
 *
 * @param[in]   event event type that is received.
 */
static void LoRaEventNotificationCB(LoRa_Event_T event);

static CmdProcessor_T * AppCmdProcessor;/**< Handle to store the main Command processor handle to be used by run-time event driven threads */
static xTaskHandle AppControllerHandle = NULL;/**< OS thread handle for Application controller to be used by run-time blocking threads */

/* inline functions ********************************************************* */

/* local functions ********************************************************** */
static void LoRaEventNotificationCB(LoRa_Event_T event)
{
    Retcode_T retcode = RETCODE_OK;

    switch (event)
    {
    // LoRa network succesfully received the packet, turn on all LEDs
    case LORA_EVENT_RECEIVED_PACKET:
        printf("Lora Network Received Packet \r\n");
        retcode = LED_On(LED_INBUILT_RED | LED_INBUILT_ORANGE | LED_INBUILT_YELLOW);
        if (RETCODE_OK != retcode)
        {
            printf("LED's failed to ON \r\n");
        }
        break;
        // Failed to send packet on the LoRa network, turn the red LED on
    case LORA_EVENT_SEND_FAILED:
        printf("Lora Network Send Failed \r\n");
        retcode = LED_Off(LED_INBUILT_RED | LED_INBUILT_ORANGE | LED_INBUILT_YELLOW);
        if (RETCODE_OK == retcode)
        {
            retcode = LED_On(LED_INBUILT_RED);
        }
        if (RETCODE_OK != retcode)
        {
            printf("LED's failed to ON \r\n");
        }
        break;
    default:
        printf("Lora Join Failed State \r\n");
        break;
    }

}

/**
 * @brief Responsible for controlling the LORA Example application control flow.
 *
 * - LoRa join is done
 * - Read the Sensor data
 * - Send sensor data via. LoRa.
 * - Wait for APP_CONTROLLER_LORA_TX_DELAY before proceeding to redoing the above steps except first step
 *
 * @param[in] pvParameters
 * Unused
 */
static void AppControllerFire(void * pvParameters)
{
    BCDS_UNUSED(pvParameters);

    Retcode_T retcode = RETCODE_OK, ledReturn = RETCODE_OK;
    Sensor_Value_T sensorValue;
    memset(&sensorValue, 0x00, sizeof(sensorValue));
    uint8_t dataBuffer[APP_LORA_BUFFER_MAX_SIZE];
    uint32_t bufferIndex = 0;
    memset(dataBuffer, 0, APP_LORA_BUFFER_MAX_SIZE);
    CayenneLPPSerializer_Input_T cayenneLPPSerializerInput;
    CayenneLPPSerializer_Output_T cayenneLPPSerializerOutput;
    cayenneLPPSerializerOutput.BufferLength = APP_LORA_BUFFER_MAX_SIZE;

    retcode = LoRa_Join();
    if (RETCODE_OK == retcode)
    {
        printf("AppControllerFire : LoRa Join Success...\r\n");
        ledReturn = LED_On(LED_INBUILT_YELLOW);
        do
        {
            retcode = LoRa_SetDataRate(0);
            if (RETCODE_OK == retcode)
            {
                /*to avoid losing the first frame sent to the gateway: gateway duplicate error message */
                retcode = LoRa_SendUnconfirmed(UINT8_C(1), (void*) 'X', UINT16_C(1));
            }
            if (RETCODE_OK == retcode)
            {
                // Set Data Rate to 3 (increase amount of data to send) and send the data via LoRa
                retcode = LoRa_SetDataRate(3);
            }
            if (RETCODE_OK != retcode)
            {

                printf("AppControllerFire :Sending first frame to gateway failed Retrying  ...\r\n");
            }
        } while (RETCODE_OK != retcode);
        while (1)
        {
            retcode = Sensor_GetData(&sensorValue);

            if (RETCODE_OK == retcode)
			{
				cayenneLPPSerializerOutput.BufferPointer = dataBuffer;
				cayenneLPPSerializerInput.DataType = CAYENNE_LLP_SERIALIZER_ACCELEROMETER;
				cayenneLPPSerializerInput.DataChannel = ACCELEROMETER_DATA_CH;
				cayenneLPPSerializerInput.Data.Accelerometer.AccelerometerXValue = (int16_t) (sensorValue.Accel.X);
				cayenneLPPSerializerInput.Data.Accelerometer.AccelerometerYValue = (int16_t) (sensorValue.Accel.Y);
				cayenneLPPSerializerInput.Data.Accelerometer.AccelerometerZValue = (int16_t) (sensorValue.Accel.Z);
				retcode = CayenneLPPSerializer_SingleInstance(&cayenneLPPSerializerInput, &cayenneLPPSerializerOutput);
				bufferIndex += cayenneLPPSerializerOutput.BufferFilledLength;
			}

            if (RETCODE_OK == retcode)
            {
                cayenneLPPSerializerOutput.BufferPointer = &dataBuffer[bufferIndex];
                cayenneLPPSerializerOutput.BufferLength = (APP_LORA_BUFFER_MAX_SIZE - bufferIndex);
                cayenneLPPSerializerInput.DataType = CAYENNE_LLP_SERIALIZER_TEMPERATURE_SENSOR;
                cayenneLPPSerializerInput.DataChannel = TEMPERATURE_DATA_CH;
                cayenneLPPSerializerInput.Data.TemperatureSensor.TemperatureSensorValue = (int16_t) (sensorValue.Temp / 100);
                retcode = CayenneLPPSerializer_SingleInstance(&cayenneLPPSerializerInput, &cayenneLPPSerializerOutput);
                bufferIndex += cayenneLPPSerializerOutput.BufferFilledLength;
            }
            if (RETCODE_OK == retcode)
			{
				cayenneLPPSerializerOutput.BufferPointer = &dataBuffer[bufferIndex];
				cayenneLPPSerializerOutput.BufferLength = (APP_LORA_BUFFER_MAX_SIZE - bufferIndex);
				cayenneLPPSerializerInput.DataType = CAYENNE_LLP_SERIALIZER_GYROMETER;
				cayenneLPPSerializerInput.DataChannel = GYROSCOPE_DATA_CH;
				cayenneLPPSerializerInput.Data.Gyrometer.GyrometerXValue = (int16_t) sensorValue.Gyro.X;
				cayenneLPPSerializerInput.Data.Gyrometer.GyrometerYValue = (int16_t) sensorValue.Gyro.Y;
				cayenneLPPSerializerInput.Data.Gyrometer.GyrometerZValue = (int16_t) sensorValue.Gyro.Z;
				retcode = CayenneLPPSerializer_SingleInstance(&cayenneLPPSerializerInput, &cayenneLPPSerializerOutput);
				bufferIndex += cayenneLPPSerializerOutput.BufferFilledLength;
			}
            if (RETCODE_OK == retcode)
            {
                cayenneLPPSerializerOutput.BufferPointer = &dataBuffer[bufferIndex];
                cayenneLPPSerializerOutput.BufferLength = (APP_LORA_BUFFER_MAX_SIZE - bufferIndex);
                cayenneLPPSerializerInput.DataType = CAYENNE_LLP_SERIALIZER_HUMIDITY_SENSOR;
                cayenneLPPSerializerInput.DataChannel = HUMIDITY_DATA_CH;
                cayenneLPPSerializerInput.Data.HumiditySensor.HumiditySensorValue = (uint8_t) (sensorValue.RH * 2);
                retcode = CayenneLPPSerializer_SingleInstance(&cayenneLPPSerializerInput, &cayenneLPPSerializerOutput);
                bufferIndex += cayenneLPPSerializerOutput.BufferFilledLength;
            }
            if (RETCODE_OK == retcode)
            {
                cayenneLPPSerializerOutput.BufferPointer = &dataBuffer[bufferIndex];
                cayenneLPPSerializerOutput.BufferLength = (APP_LORA_BUFFER_MAX_SIZE - bufferIndex);
                cayenneLPPSerializerInput.DataType = CAYENNE_LLP_SERIALIZER_BAROMETER;
                cayenneLPPSerializerInput.DataChannel = PRESSURE_DATA_CH;
                cayenneLPPSerializerInput.Data.Barometer.BarometerValue = (uint16_t) (sensorValue.Pressure / 10);
                retcode = CayenneLPPSerializer_SingleInstance(&cayenneLPPSerializerInput, &cayenneLPPSerializerOutput);
                bufferIndex += cayenneLPPSerializerOutput.BufferFilledLength;
            }
            if (RETCODE_OK == retcode)
            {
                cayenneLPPSerializerOutput.BufferPointer = &dataBuffer[bufferIndex];
                cayenneLPPSerializerOutput.BufferLength = (APP_LORA_BUFFER_MAX_SIZE - bufferIndex);
                cayenneLPPSerializerInput.DataType = CAYENNE_LLP_SERIALIZER_ILLUMINANCE_SENSOR;
                cayenneLPPSerializerInput.DataChannel = ILLUMINANCE_DATA_CH;
                cayenneLPPSerializerInput.Data.IlluminanceSensor.IlluminanceSensorValue = (uint16_t) (sensorValue.Light / 1000);
                retcode = CayenneLPPSerializer_SingleInstance(&cayenneLPPSerializerInput, &cayenneLPPSerializerOutput);
                bufferIndex += cayenneLPPSerializerOutput.BufferFilledLength;
            }
            if (RETCODE_OK == retcode)
            {
                retcode = LoRa_SendUnconfirmed(APP_LORA_PORT, &dataBuffer, bufferIndex);
                if (RETCODE_OK != retcode)
                {
                    printf("AppControllerFire:Failed to Transmit data on Port %d\r\n", APP_LORA_PORT);
                }
                bufferIndex = 0;
                memset(dataBuffer, 0, APP_LORA_BUFFER_MAX_SIZE);
            }
            if (RETCODE_OK != retcode)
            {
                //Retcode_RaiseError(retcode);
            }
            vTaskDelay(pdMS_TO_TICKS(APP_CONTROLLER_LORA_TX_DELAY));
        }
    }
    else
    {
        printf("AppControllerFire : LoRa Join Failed hence suspended the task...\r\n");
        //printf("%02X%02X%02X%02X%02X%02X%02X%02X\r\n",appEUI[0],appEUI[1],appEUI[2],appEUI[3],appEUI[4],appEUI[5],appEUI[6],appEUI[7]);
        printf("appEUI: %X \r\n",appEUI);

        uint64_t hwDevEUI;
        uint8_t* hwEUI;
        retcode = LoRa_GetHwEUI(&hwDevEUI);
        printf("%X \r\n",hwDevEUI);
        if (retcode == RETCODE_OK)
        {
            hwEUI = (uint8_t*) &hwDevEUI;
            printf("[XDK][LORA] Hardware DevEUI: %02x%02x%02x%02x%02x%02x%02x%02x configure the same in the application server\r\n",
                    hwEUI[7], hwEUI[6], hwEUI[5], hwEUI[4], hwEUI[3], hwEUI[2], hwEUI[1], hwEUI[0]);
        }
        else
        {
            printf("AppControllerFire : Failed to get Dev EUI \r\n");
        }
        ledReturn = LED_On(LED_INBUILT_RED);
        if (RETCODE_OK != ledReturn)
        {
            printf("AppControllerFire : Failed to ON LED \r\n");
        }
        vTaskSuspend(AppControllerHandle);
    }
}

/**
 * @brief To enable the necessary modules for the application
 * - LoRa
 * - Sensor
 *
 * @param[in] param1
 * Unused
 *
 * @param[in] param2
 * Unused
 */
static void AppControllerEnable(void * param1, uint32_t param2)
{
    BCDS_UNUSED(param1);
    BCDS_UNUSED(param2);
    Retcode_T retcode = RETCODE_OK;
    uint8_t* hwEUI;
    uint64_t hwDevEUI;

    retcode = LoRa_Enable();
    if (RETCODE_OK == retcode)
    {
        retcode = LoRa_GetHwEUI(&hwDevEUI);
        if (RETCODE_OK == retcode)
        {
            hwEUI = (uint8_t*) &hwDevEUI;
            printf("[XDK][LORA] Hardware DevEUI: %02x%02x%02x%02x%02x%02x%02x%02x configure the same in the application server\r\n",
                    hwEUI[7], hwEUI[6], hwEUI[5], hwEUI[4], hwEUI[3], hwEUI[2], hwEUI[1], hwEUI[0]);


        }
    }
    if (RETCODE_OK == retcode)
    {
        retcode = Sensor_Enable();
    }
    if (RETCODE_OK == retcode)
    {
        retcode = LED_Enable();
    }
    if (RETCODE_OK == retcode)
    {
        if (pdPASS != xTaskCreate(AppControllerFire, (const char * const ) "AppController", TASK_STACK_SIZE_APP_CONTROLLER, NULL, TASK_PRIO_APP_CONTROLLER, &AppControllerHandle))
        {
            retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_OUT_OF_RESOURCES);
        }
    }
    if (RETCODE_OK != retcode)
    {
        printf("AppControllerEnable : Failed \r\n");
        printf("%s-%s-%s-%s-%s-%d-%d-%d-%d|\r\n",
                			APP_LORA_RX_WINDOW_FREQ,
                			APP_LORA_FREQUENCY,
        					APP_CONTROLLER_LORA_TX_DELAY,
        					APP_LORA_APP_EUI,
        					APP_LORA_APP_KEY,
                			typesSensors[0],typesSensors[1],typesSensors[2],typesSensors[3]);
        //int i=0;
    	//for(i=0;i<16;i++)
		//	printf("%02x\r\n",AppKey[i]);


        Retcode_RaiseError(retcode);
        assert(0); /* To provide LED indication for the user */
    }
}

uint64_t stringHexToUint64_t(char const *str)
{
    uint64_t accumulator = 0;
    for (size_t i = 0 ; isxdigit((unsigned char)str[i]) ; ++i)
    {
        char c = str[i];
        accumulator *= 16;
        if (isdigit(c)) /* '0' .. '9'*/
            accumulator += c - '0';
        else if (isupper(c)) /* 'A' .. 'F'*/
            accumulator += c - 'A' + 10;
        else /* 'a' .. 'f'*/
            accumulator += c - 'a' + 10;

    }

    return accumulator;
}

static void AppControllerSetup(void * param1, uint32_t param2)
{
    BCDS_UNUSED(param1);
    BCDS_UNUSED(param2);

    Retcode_T retcode = RETCODE_OK;

	Retcode_T rt = RETCODE_FAILURE;
	rt = InitSdCard();
	if(rt != RETCODE_FAILURE)
		readDataFromFileOnSdCard(FILE_NAME);

	appEUI = stringHexToUint64_t(APP_LORA_APP_EUI);

    LoRa_Setup_T LoRaSetupInfo =
            {
                    .CodingRate = APP_LORA_CODING_RATE,
                    .DevEUI = NULL,
                    .AppEUI = appEUI,
                    .AppKey = (uint8_t*) AppKey,
                    .RxFreq = atol(APP_LORA_RX_WINDOW_FREQ),
                    .Freq = atol(APP_LORA_FREQUENCY),
                    .EventCallback = LoRaEventNotificationCB,
                    .JoinType = LORA_JOINTYPE_OTAA,
            };

    Sensor_Setup_T SensorSetup =
            {
                    .CmdProcessorHandle = NULL,
                    .Enable =
                            {
                                    .Accel = typesSensors[1],
                                    .Mag = false,
                                    .Gyro = typesSensors[2],
                                    .Humidity = typesSensors[0],
                                    .Temp = typesSensors[0],
                                    .Pressure = typesSensors[0],
                                    .Light = typesSensors[3],
                                    .Noise = false,
                            },
                    .Config =
                            {
                                    .Accel =
                                            {
                                                    .Type = SENSOR_ACCEL_BMA280,
                                                    .IsRawData = false,
                                                    .IsInteruptEnabled = false,
                                                    .Callback = NULL,
                                            },
                                    .Gyro =
                                            {
                                                    .Type = SENSOR_GYRO_BMG160,
                                                    .IsRawData = false,
                                            },
                                    .Mag =
                                            {
                                                    .IsRawData = false,
                                            },
                                    .Light =
                                            {
                                                    .IsInteruptEnabled = false,
                                                    .Callback = NULL,
                                            },
                                    .Temp =
                                            {
                                                    .OffsetCorrection = APP_TEMPERATURE_OFFSET_CORRECTION,
                                            },
                            },
            };



    retcode = LoRa_Setup(&LoRaSetupInfo);
    if (RETCODE_OK == retcode)
    {
        SensorSetup.CmdProcessorHandle = AppCmdProcessor;
        retcode = Sensor_Setup(&SensorSetup);
    }
    if (RETCODE_OK == retcode)
    {
        retcode = LED_Setup();
    }
    if (RETCODE_OK == retcode)
    {
        retcode = CmdProcessor_Enqueue(AppCmdProcessor, AppControllerEnable, NULL, UINT32_C(0));
    }
    if (RETCODE_OK != retcode)
    {
        printf("AppControllerSetup : Failed \r\n");
        Retcode_RaiseError(retcode);
        assert(0); /* To provide LED indication for the user */
    }
}
/* global functions ********************************************************** */
/** Refer interface header for description */
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
        retcode = CmdProcessor_Enqueue(AppCmdProcessor, AppControllerSetup, NULL, UINT32_C(0));
    }

    if (RETCODE_OK != retcode)
    {
        Retcode_RaiseError(retcode);
        assert(0); /* To provide LED indication for the user */
    }
}
/**@} */
/** ************************************************************************* */
