#ifndef _MACH_CONTROL_H_
#define _MACH_CONTROL_H_


#define MACH_COMMUN_GET   0x02
#define MACH_COMMUN_SET   0x01









typedef enum
{
	MACH_STOP_MODE = 0x00,
	MACH_RUN_MODE,
	MACH_ERROR_MODE,
}enum_machControlStatus;


typedef enum
{
	MACH_STREAM_OUT = 0x10,
	MACH_VALUE_OUT = 0x20,
	MACH_CONTROL_ERROR = 0x40,
	MACH_COMMUN_ERROR = 0x80,
}enum_machControlError;


typedef struct machControlInfo
{
	char head;
	char address;
	char typeCode;
	char dataLen;
	char pdata[2];
	char crcAdd[2];
	char tail;
}stu_machControlInfo;



/*********************begin extern func***************************/
extern int controlForKey(int index);
extern int getMachDataforUsart(void);
/*********************end extern func*****************************/


























#endif


