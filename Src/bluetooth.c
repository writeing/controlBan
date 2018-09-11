#include "bluetooth.h"
#include "string.h"
#include "FIFO_F4.h"
#include "wxc_common.h"
#include "kalman_filter.h"
#include "dac.h"
#include "math.h"
int g_blueToothFalg = 0;   // 0 device   1 Acquisition boards
stuBlueToothData g_flashBlueToothData;

kalman1_state state1;

void sendBuffer(uint8_t *buff)
{
	int len = strlen((char *)buff);	
	FIFO_UartSendBuffer(&bluetooth,buff,len);
	FIFO_UartSendBuffer(&debug,buff,len);
	
}
GPIO_PinState getGPIOBLE_CON()
{
	return HAL_GPIO_ReadPin(BLE_CON_GPIO_Port,BLE_CON_Pin);
}
void setGPIOBLE_AT(GPIO_PinState status)
{
	HAL_GPIO_WritePin(BLE_AT_GPIO_Port,BLE_AT_Pin,status);
}
int initBlueTooth()
{
	g_blueToothFalg = 0;

	if(getGPIOBLE_CON() == GPIO_PIN_SET)
	{
		// had connect
		printf("had connect\r\n");
		sendBuffer((uint8_t *)"hello world\r\n");
		setGPIOBLE_AT(GPIO_PIN_SET);
		g_blueToothFalg = 1;
		return WXC_OK;
	}
	//init bluetooth
	sendBuffer((uint8_t *)"AT+MAC");
	HAL_Delay(1000);
	sendBuffer((uint8_t *)"AT+HOSTEN1");
	HAL_Delay(1000);
	sendBuffer((uint8_t *)"AT+SCAN1");
	HAL_Delay(3000);
	if(getGPIOBLE_CON() == GPIO_PIN_SET)
	{
		printf("had connect\r\n");
		g_blueToothFalg = 1;
		return WXC_OK;
	}
	if(g_flashBlueToothData.arrayID[0] == 0)
	{
		sendBuffer((uint8_t *)"AT+RSLV");
		HAL_Delay(1000);
	}
	uint32_t time = HAL_GetTick();
	while(g_flashBlueToothData.arrayID[0] == 0 && HAL_GetTick() - time < 10000);
	if(g_flashBlueToothData.arrayID[0] == 0)
	{
		//connect error
		g_blueToothFalg = 1;
		return WXC_ERROR;
	}
	char buff[100];
	sprintf(buff,"AT+BAND%s\0",g_flashBlueToothData.arrayID);
	sendBuffer((uint8_t *)buff);
	HAL_Delay(1000);
	sprintf(buff,"AT+CONNET%s\0",g_flashBlueToothData.arrayID);
	sendBuffer((uint8_t *)buff);
	HAL_Delay(1000);
	time = HAL_GetTick();
	while(getGPIOBLE_CON() != GPIO_PIN_SET && HAL_GetTick() - time < 10000)
	{
		HAL_Delay(500);
		printf("wait connet\r\n");
	}
	if(getGPIOBLE_CON() != GPIO_PIN_SET)
	{
		return WXC_ERROR;
	}
	else
	{		
		printf("had connect\r\n");
		return WXC_OK;
	}
	g_blueToothFalg = 1;
	return WXC_OK;
	
}

static double frevBuff[50];
static int frevIndex = 0;

static uint8_t crevBuff[50];
static int crevIndex  = 0;
double proy = 0,prop = 0,pror = 0;
double nowy = 0,nowp = 0,nowr = 0;


static int runspeed = 0;
static int rightspeed = 0;

void getRunSpeedforbluetooth()
{
	int value;
	value = (int)((2048/10)*runspeed);
	HAL_DAC_SetValue(&hdac,DAC_CHANNEL_2,DAC_ALIGN_12B_R,value);
	HAL_DAC_Start(&hdac,DAC_CHANNEL_2);	
}

void getRightSpeedforbluetooth()
{
	int value;
	//rightspeed = 10- rightspeed;
	//value = (int)((2048/10)*rightspeed);
	value = 2048 + (int)((2048/10)*rightspeed);
	HAL_DAC_SetValue(&hdac,DAC_CHANNEL_1,DAC_ALIGN_12B_R,value);
	HAL_DAC_Start(&hdac,DAC_CHANNEL_1);	
}


int getNowRunSpeed()
{
	return runspeed;
}
int getNowRightSpeed()
{
	return rightspeed;
}
int CalcRunSpeed(double value1,double value2)
{
	//different plus or minus
	double value = value1 - value2;
	float out1 = 0;
//	value = abs(value);
	//out1 = kalman1_filter(&state1, value1);

	int temp = (int)((out1*10/90));
	return temp;
}
int CalcRightSpeed(double value1,double value2)
{
	double value = value1 - value2;
	//value = abs(value);
	int temp = (int)((value*10/90));
	return temp;
}

// set aquireboard cmd
int sendCmdToAquireBoards(char *buff,int cmdIndex)
{
	setArrFlag(cmdIndex);
	g_flashBlueToothData.cmdFlag = 0;	
	sendBuffer((uint8_t *)buff);
	unsigned int time = HAL_GetTick();
	while(g_flashBlueToothData.cmdFlag == 0 && HAL_GetTick() - time < 10000)
	{
		sendBuffer((uint8_t *)buff);
	}
	if(g_flashBlueToothData.cmdFlag == 0)
		return WXC_ERROR;
	else
		return WXC_OK;
}

//set loaca data redefine 
void setLocalStatus()
{
	if(g_flashBlueToothData.bluetoothDataStatus == WXC_ERROR)
	{
		// haven`t start yet
		return;
	}
	if(proy == 0 && prop == 0&& pror == 0)
	{
		//init
		proy = frevBuff[0];
		prop = frevBuff[1];
		pror = frevBuff[2];
		//kalman1_init(&state1, nowp, 5e2);
		return;
	}
	nowy = frevBuff[0];
	nowp = frevBuff[1];
	nowr = frevBuff[2];
	runspeed = CalcRunSpeed(nowp,prop);
	rightspeed = CalcRightSpeed(nowr,pror);
	printf("%d\r\n",abs(((int)(nowp - prop))));	
}

void _inputAnsy(uint8_t ch)
{
	crevBuff[crevIndex] = ch;
	if(crevIndex > 5 && crevBuff[crevIndex - 1] == '\r' && crevBuff[crevIndex ] == '\n')
	{
		uint8_t temp[10] = {0};
		int j = 0;
		memset(temp,0,10);
		for(int i = 0 ; i < crevIndex - 1 ; i ++)
		{
			if(crevBuff[i] == ',')
			{
				frevBuff[frevIndex++] = atof((char *)temp);				
				j = 0;
				memset(temp,'\0',10);
				continue;
			}
			temp[j++] = crevBuff[i];
			
		}
		frevBuff[frevIndex++] = atof((char *)temp);
		frevIndex = 0;
		crevIndex  = 0;
		setLocalStatus();
		return;
	}	
	crevIndex ++;
}
static char g_arrCmdbuff[100];

void _inputAnsyblueTooth(char ch)
{
	//get bluetooth id 
	printf("%c",ch);
	static int s_index = 0;
	g_arrCmdbuff[s_index++] = ch;
	if(strstr(g_arrCmdbuff,"reset") != 0 && getArrFlag(BLUE_CMD_RESET) == WXC_OK)
	{
		//findof reset 
		g_flashBlueToothData.cmdFlag = 1;
		memset(g_arrCmdbuff,0,s_index);
		s_index = 0;
		clearArrFlag(BLUE_CMD_RESET);
	}
}
void getblueTooth()
{
	uint8_t ch;
	while(FIFO_UartReadByte(&bluetooth,&ch) == HAL_OK)	
	{
		if(g_blueToothFalg == 1)
		{
			_inputAnsy(ch);
		}
		else
		{
			_inputAnsyblueTooth(ch);
		}
	}
}








