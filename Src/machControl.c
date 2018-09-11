#include "machControl.h"
#include "main.h"
#include "usart.h"
#include "stm32f4xx_hal.h"
#include "wxc_common.h"
#include "string.h"
stu_machControlInfo g_sendMachControlInfo;

stu_machControlInfo g_revMachControlInfo;
static char revDataBuff[10];


int initMachControl()
{
	memcpy(g_revMachControlInfo.pdata,revDataBuff,2);
	return 1;
}
int _crcAddData(unsigned char *buff,int len)
{
	int addData = 0;
	for(int i = 1; i < len -2 ; i ++)
	{
		addData += buff[i];
	}
	if((addData & 0xFF00) == buff[len - 1] && (addData & 0x00FF) == buff[len - 2])
	{
		return WXC_OK;
	}
	else
		return WXC_ERROR;
}
int _inputMachControl(uint8_t ch)
{
	static uint8_t s_revBuff[20];
	static int s_revbuffIndex = 0;
	static int s_revBeginFlag = 0;
	
	if(ch == 0x55 && s_revBeginFlag == 0)
	{
		s_revBeginFlag = 1;		
		memset(s_revBuff,0,20);
		s_revbuffIndex = 0;
	}
	else if(ch == 0x55 && s_revBeginFlag == 1)
	{
		memset(s_revBuff,0,20);
		s_revbuffIndex = 0;
	}
	if(ch == 0xAA && s_revBeginFlag == 1)
	{
		//end then to ansy
		int i = 0 ;
		g_revMachControlInfo.head = s_revBuff[i++];
		g_revMachControlInfo.address = s_revBuff[i++];
		g_revMachControlInfo.typeCode = s_revBuff[i++];		
		g_revMachControlInfo.dataLen = s_revBuff[i++];
		for(int j = 0 ; j < g_revMachControlInfo.dataLen ; j ++)
			g_revMachControlInfo.pdata[j] = s_revBuff[i++];	
		g_revMachControlInfo.crcAdd[0] = s_revBuff[i++];
		g_revMachControlInfo.crcAdd[1] = s_revBuff[i++];
		return _crcAddData(s_revBuff,s_revbuffIndex);
	}
	else if(ch == 0xAA && s_revBeginFlag == 0)
	{
		memset(s_revBuff,0,20);
		s_revbuffIndex = 0;		
	}
	s_revBuff[s_revbuffIndex++] = ch;
	return WXC_ERROR;
	
}
int getMachControlStatus()
{
	char deviceStatus = g_revMachControlInfo.pdata[0];
	switch (deviceStatus)
	{
		case MACH_STOP_MODE: 
			break;
		case MACH_RUN_MODE:
			break;
		case MACH_STREAM_OUT:
			break;
		case MACH_ERROR_MODE:
			break;
		default:break;
	}
	return deviceStatus;
	
}
void _getAddNumData(unsigned char *buff)
{
	int add = 0;
	for(int i = 1; i < 6 ; i ++)
	{
		add += buff[i];
	}
	*(buff+6) = add & 0x00FF;
	*(buff+7) = add & 0xFF00;	
}
void _sendDataToUsart(unsigned char* buff,int len)
{
	FIFO_UartSendBuffer(&control,buff,len);
	FIFO_UartSendBuffer(&debug,buff,len);
}

int sendMachDataToUsart(char status,char speed)
{
	char buff[2] = {status,speed};	
	unsigned char sendBuff[30];
	g_sendMachControlInfo.head = 0x55;
	g_sendMachControlInfo.address = WXC_ADDRESS;
	g_sendMachControlInfo.typeCode = MACH_COMMUN_SET;
	g_sendMachControlInfo.dataLen = 2;
	//g_sendMachControlInfo.pdata = buff;
	memcpy(g_sendMachControlInfo.pdata,buff,2);
	memcpy(sendBuff,(char *)&g_sendMachControlInfo,6);		
	_getAddNumData(sendBuff);
	*(sendBuff+8) = 0xAA;
	_sendDataToUsart(sendBuff,9);
	return WXC_OK;
}
int controlForKey(int index)
{
	static int status = 0;
	static int speed = 0;
	switch(index)
	{
		case 0:
			status = 0;			
			break;
		case 1:
			status = 1;
			break;
		case 2:
			speed += 10;
			break;
		case 3:
			speed -= 10;
			break;
		default:break;
	}
	if(speed < 0)
	{
		speed = 0;
	}
	if(speed > 120)
	{
		speed = 120;
	}
	//printf("status = %d,speed = %d\r\n",status,speed);
	sendMachDataToUsart(status,speed);
	return speed;
	
}
int getMachDataforUsart()
{
	uint8_t ch;
	while(FIFO_UartReadByte(&control,&ch) == HAL_OK)	
	{	
		if(_inputMachControl(ch) == WXC_OK)
			break;
	}
	return WXC_OK;
}





















































