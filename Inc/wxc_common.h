#ifndef WXC_COMMON_H_

#define WXC_COMMON_H_
#include "stm32f4xx_hal.h"

/****************************begin define***************************************/
#define WXC_OK       1
#define WXC_ERROR    0

#define WXC_ADDRESS    0x01
/****************************end define *****************************/


/***************************begin variable*******************************************/
typedef enum
{
	BLUE_CMD_RESET = 0,
	BLUE_CMD_INIT = 1,
}BlueToothcmdFlag;



/**************************end variable ****************************************/










/*********************************begin func*************************************/
extern void setArrFlag(int index);


extern void clearArrFlag(int index);


extern int getArrFlag(int index);


/*********************************end func*********************************/
















#endif

