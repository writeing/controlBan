#ifndef __BLUETOOTH__
#define __BLUETOOTH__

#include "FIFO_F4.h"
#include "usart.h"
#include "stm32f4xx_hal.h"

extern int initBlueTooth(void);
extern void getblueTooth(void);

extern int CalcRunSpeed(double value1,double value2);

extern int CalcRightSpeed(double value1,double value2);

extern int setaquireboradsInit(void);
typedef struct 
{
	char arrayID[100];
	char bluetoothConnected;
	char bluetoothDataStatus;	
	char cmdFlag;
}stuBlueToothData;

#endif

