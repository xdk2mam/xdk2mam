
#ifndef XDK110_ACCELEROMETER_H_
#define XDK110_ACCELEROMETER_H_

#include "BCDS_Retcode.h"

char* processAccelData(void * param1, uint32_t param2);

extern Retcode_T accelerometerSensorInit(void);

extern Retcode_T accelerometerSensorDeinit(void);

#endif


