/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "stdio.h"
#include "stm32f1_rc522.h"
#include "stdio.h"
#include "string.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "ecard.h"
#include "iso14443ab.h"
#include "softuart.h"
#include "define.h"
#include "Config.h"
#include "eeConfig.h"
#include "ee.h"
#include "string.h"
#include "stdlib.h"
#include "flash.h"
#include "heart_beat.h"
#ifdef PN532_Routine
#include "pn532_stm32f1.h"
#endif

#ifdef Max30102
#include "MAX30102.h"
#endif
#if (deviceType==BD) || (deviceType==BCU)
#include "crc16.h"
#endif
//#if defined(UHF)
//#include "UHF.h"
//#endif

/* USER CODE END Includes */
/* 
bootloader : 0x8005800 , 0x1A800  , define withBoot in symbolss 
moteghayeri k ba volatile sakhte mishe, agar vared INT beshe taghiri nmikonad. 
*/

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */
//.\Project\AndroidF1.axf: Error: L6218E: Undefined symbol SaveTransactionRecord (referred from main.o).
/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
IWDG_HandleTypeDef hiwdg;

#if defined (Max30102) || defined(PN532_Routine)
I2C_HandleTypeDef hi2c1;
#endif
//I2C_HandleTypeDef *i2c_max30102;

#if defined (Max30102)
max30102_t max30102;
#endif

#if defined LCD
#include "FontIcon.h"
#include "LCD.h"
void showDateTime(void);

#endif
RTC_HandleTypeDef hrtc;

SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;


#ifdef remote 
TIM_HandleTypeDef htim2;
#endif

#ifdef virtualRemote 
TIM_HandleTypeDef htim1;
#endif

TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;
unsigned char uart_no = 2;

/* USER CODE BEGIN PV */
RTC_TimeTypeDef time;
RTC_DateTypeDef date;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_RTC_Init(void);
static void MX_SPI1_Init(void);
static void MX_SPI2_Init(void);
static void MX_TIM3_Init(void);
#ifndef atlas
static void MX_USART1_UART_Init(void);
#endif
static void MX_USART2_UART_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_IWDG_Init(void);
unsigned char decFlag = 0, cardReadStep = 0;
unsigned int 
	now 								= 0, 
	requestID  			 		= 123456,
	preRequestID        = 0,
	lastTransactionSent = 0;

void sendCRC(const char* buffer);
unsigned char getCRC(const char* buffer);
unsigned char useGift = 1, dontSend = 0;

#ifdef remote  
static void MX_TIM2_Init(void);
#endif

#ifdef virtualRemote 
static void MX_TIM1_Init(void);
#endif

#if defined (Max30102) || defined(PN532_Routine)
static void MX_I2C1_Init(void);
#endif
/* USER CODE BEGIN PFP */
#ifdef    NoDispenser
int NOdispenser;
#endif
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define led(x)        HAL_GPIO_WritePin(GPIOA,led_Pin,x);
#define ToggleLED     HAL_GPIO_TogglePin(GPIOA,led_Pin); 		
#if defined AutoStart
#define POWERTAB(x)    HAL_GPIO_WritePin(PowerKey_GPIO_Port ,PowerKey_Pin,x);
#endif

#if (deviceType==BCU) || (deviceType == BD)
#define SetTX485();      HAL_GPIO_WritePin(DERE_GPIO_Port,DERE_Pin,1);
#define SetRX485();      HAL_GPIO_WritePin(DERE_GPIO_Port,DERE_Pin,0);
#endif

#if (deviceType == AutoPark)
#define READKey   HAL_GPIO_ReadPin(Key_GPIO_Port,Key_Pin)
#endif

#if defined GATEROADBLOCK
#define RELAYSTATE   HAL_GPIO_ReadPin(Relay_State_GPIO_Port, Relay_State_Pin) 

#define IR1   2 // cheshmi 1 , vorod
#define IR3   2 // cheshmi 2 , khoroj 

#define IR2       					  HAL_GPIO_ReadPin(IR2_GPIO_Port, IR2_Pin)
#define SNSUP     	 					HAL_GPIO_ReadPin(SNSUP_GPIO_Port,SNSUP_Pin)
#define SNSDOWN   	 					HAL_GPIO_ReadPin(SNSDOWN_GPIO_Port, SNSDOWN_Pin)

#define RELAY_OUT(x) 	  	    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,x);
#define RELAY_DIR(x)  	      HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,x);   //k2 , p



in paini  

#define RELAY_IN(x)        		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_10,x);  //k1 , (pin vasati phonix 1*3, va akhari gnd) 
#define RELAY_PWR(x)        	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_10,x);

#endif

#define NotFoundMem 			send("{\"Mem\":{\"Status\":-1}}\n");
#define DeleteMem 				send("{\"Mem\":{\"Status\":0}}\n");
#define MemoryFull        send("{\"Mem\":{\"Status\":1}}\n");
#define MemoryComeingFull send("{\"Mem\":{\"Status\":2}}\n");
#define EmptyMem   				send("{\"Mem\":{\"Status\":3}}\n");

#ifdef __GNUC__
/* With GCC, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
PUTCHAR_PROTOTYPE
{	
  CDC_Transmit_FS((uint8_t *)&ch,1);
	HAL_Delay(1);
  return ch;
}
//*************************************************************send
unsigned char getCRC(const char* buffer) {
	unsigned char 
		crc = 0,
		len = strlen(buffer);
	for (unsigned short i=0; i<len; i++)
		crc ^= buffer[i];
	return crc;
}
void sendCRC(const char* buffer) {
	unsigned char crc = getCRC(buffer);
	CDC_Transmit_FS((uint8_t*)&crc, 1);
}

void sendDebug(unsigned char* data) {
	#ifdef DEBUG_HNA
	HAL_UART_Transmit(&huart3, (unsigned char*)data ,strlen(data), 1000);
	#endif
}

int send(unsigned char *buf) {
	unsigned char 
		len = strlen(buf),
		count = 0;
	int result = 1000;
	
	#ifdef DEBUG_HNA
	if (!dontSend)
		sendDebug(buf);
	#endif
	result = CDC_Transmit_FS((unsigned char*)buf, len);
	
	#ifdef serialport 
			 HAL_UART_Transmit(&huart2, buf, strlen(buf), 100);	//posTest Terminal
	#endif
	led(0) 
	HAL_Delay(1);
	led(1)
	return result;
}
//*************************************************************
unsigned char SaveFromRamToDF(unsigned int memoryOffset, unsigned int sizeToCopy, unsigned char *pDest) {
	WDTR;
	return(SPI_Flash_Write2(pDest, memoryOffset, sizeToCopy));
}
unsigned char LoadFromDFToRam(unsigned int memoryOffset, unsigned int sizeToCopy, unsigned char *pDest)
{
	WDTR;
	SPI_Flash_Read(pDest, memoryOffset, sizeToCopy);
}
void SaveRingDetail(void) {
	unsigned char i;
	unsigned char buf[12];
	IntToBytes(trHead, &buf[0]);
	IntToBytes(trTail, &buf[4]);
  buf[8] = deviceType;
	
  crc=0;
  for (i=0; i<9; i++)
    crc+=buf[i];
	
  buf[9]= crc & 0xFF;
  buf[10]=(crc>>8) & 0xFF;
  if (SaveFromRamToDF(addIndicators, 11, buf))
		SaveFromRamToDF(addIndicators, 11, buf);
}
//ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss
unsigned char LoadRecordsInfo(void)
{
	unsigned int i;
	unsigned short crc=0;
	unsigned int OfflineStart, Address, Idx, Idx2;
	unsigned char Found=0, buf[12];
  
  LoadFromDFToRam(addIndicators, 11, buf);
	BytesToInt(&trHead, &buf[0]);
	BytesToInt(&trTail, &buf[4]);

	
	crc=0;
  for (i=0; i<9; i++)
    crc+=buf[i];
	
  if ((buf[9]+(buf[10]*256))!=crc)
  {
    trHead=0;
    trTail=0;
  }
	
  if (buf[8] != deviceType)
  {
    trHead=0;
    trTail=0;
  }

  if ((trHead>MAX_TRANSACTIONS))
    trHead = 0;
	
  if (trTail>=2)
		trTail = 0;
  
	/*
  Idx=trHead; Idx2=trTail;

  for (i=0; i<MAX_TRANSACTIONS; i++) 
  {
    WDTR;
    Address=addTransactions+((Idx+1)*LEN_TRANSACTIONS)-2; 
    LoadFromDFToRam(Address, 2, buf);

    if ((buf[0]==0xff) && (buf[1]==0xff))  //3c 14
    {
      trHead=Idx;
      trTail=Idx2;
      break;
    }
    Idx++;
    if (Idx>=MAX_TRANSACTIONS) Idx=0;
    if (Idx==Idx2)
      if (++Idx2>=MAX_TRANSACTIONS)
        Idx2=0;
  }
	
  if (i>=MAX_TRANSACTIONS)
  {
    trHead=0;
    trTail=0;
  }
  
  //-----------------------------
  if (trHead!=trTail)
  {
    Idx=trHead;
    OfflineStart=trTail;
    if (Idx>0)
      Idx--;
    else if (trTail!=0)
      Idx=MAX_TRANSACTIONS-1;
    
    //printf("\n\rCheck Offline ");
    for (i=0; i<MAX_TRANSACTIONS; i++)
    {
      if (Idx==trTail)
        break;
			WDTR;

      Address=addTransactions+(Idx*LEN_TRANSACTIONS); 
      LoadFromDFToRam(Address+LEN_TRANSACTIONS-1, 1, buf);
      //printf("- %d, buf[0] =%x ",Idx,buf[0]);		
		
      if (buf[0]==0x90)
      {
        //printf("\n\rOff Found");
        OfflineStart=Idx+1;
        if (OfflineStart>=MAX_TRANSACTIONS) OfflineStart=0;
        break;
      }

      if (Idx==0)
        Idx=MAX_TRANSACTIONS-1;
      else 
        Idx--;
    }
  }  
  */
  SaveRingDetail();
}

void IncTrHead(void)
{
	trHead++;
	if (trHead == MAX_TRANSACTIONS - 10)
		MemoryComeingFull
	
  if (trHead>MAX_TRANSACTIONS)
	{
    trHead=0;
		trTail++;
		MemoryFull	
	}
  if (trTail>=2)
     trTail=0; 
}
//************************************
unsigned char SaveTransactionRecord(unsigned char *Buf,unsigned char len)
{
	unsigned int address;	
	unsigned short i;
  address=addTransactions+(trHead*len);
  if (SaveFromRamToDF(address, len, Buf))
		if (SaveFromRamToDF(address, len, Buf))
		{
			IncTrHead();
      address=addTransactions+(trHead*len);
		  SaveFromRamToDF(address, len, Buf);	
		}	
	IncTrHead();
	
}
//************************************
void loadTransactionRecord(void) {
	unsigned char readRecord[LEN_TRANSACTIONS] = {0};
	unsigned char sendToAndroid[400];
	unsigned int ucRam = 0,cardIdRam = 0;
	
	#if (deviceType==AutoPark)
	unsigned char gpRam,yRam,MRam,dRam,hRam,mRam,sRam,eemRam ; 
	#endif	
	#if (deviceType==AmusementPark)
	unsigned int etebarRam,preEtebarRam,hPaymentRam,paymentRam,hNPaymentRam ;
  unsigned char statusRam,opRam,yRam,MRam,dRam,hRam,mRam;	
	#endif
	#if (deviceType==BusEtebar)
	unsigned int etebarRam,preEtebarRam,paymentRam;
  unsigned char statusRam,opRam,yRam,MRam,dRam,hRam,mRam;	
	#endif
	#if (deviceType==BCU)
	unsigned int etebarRam,preEtebarRam,paymentRam;
  unsigned char statusRam,opRam,yRam,MRam,dRam,hRam,mRam,gpRam;	
	#endif
	
	unsigned int row = 0;
	unsigned int trHeadBuf = trHead - 1  ;
	unsigned char Return = 0;
	unsigned int cntr = 0;
	send("{\"Receive\":{\"Status\":26},\"Mem\":{\"Type\":\"RaedRecord\"}}\n");
	if (memID != 0xEF16)
	{
		NotFoundMem
		return;
	}
	if ( (trHead == 0) && (trTail == 0)) 
	{
		EmptyMem 
		return;
	}
	
	cntr = 0;
	if (trTail)
	{
		if (trHead)
		{
			cntr = trHead;
			trHeadBuf = MAX_TRANSACTIONS;
		}
		else
		{
			cntr = 0;
			trHeadBuf = MAX_TRANSACTIONS;			
		}
	}
	for (; cntr <= trHeadBuf ; cntr++)
	{
		WDTR
		LoadFromDFToRam(addTransactions+(cntr*LEN_TRANSACTIONS),LEN_TRANSACTIONS,readRecord);
		if (readRecord[0] == deviceType)
		{
			row++;
			#if (deviceType==AutoPark)
			cardIdRam = readRecord[1] + ( readRecord[2] << 8) + ( readRecord[3] << 16) + ( readRecord[4] << 24);   
			ucRam     = readRecord[5] + ( readRecord[6] << 8) + ( readRecord[7] << 16) + ( readRecord[8] << 24);
			gpRam     = readRecord[9];
			yRam      = readRecord[10];
			MRam      = readRecord[11];
			dRam      = readRecord[12];
			hRam      = readRecord[13];
			mRam      = readRecord[14];
			sRam      = readRecord[15];
			eemRam    = readRecord[16];
			sprintf(sendToAndroid,"{\"C\":{\"Row\":%d,\"ID\":%d,\"UC\":%d,\"Gp\":%d,\"Y\":%d,\"M\":%d,\"D\":%d,\"H\":%d,\"m\":%d,\"S\":%d,\"EEM\":%d}}\n",row,cardIdRam,ucRam,gpRam,yRam,MRam,dRam,hRam,mRam,sRam,eemRam);
			#endif
			
			#if (deviceType==AmusementPark)
			cardIdRam     = readRecord[1] + ( readRecord[2] << 8) + ( readRecord[3] << 16) + ( readRecord[4] << 24);   
			ucRam         = readRecord[5] + ( readRecord[6] << 8) + ( readRecord[7] << 16) + ( readRecord[8] << 24);
			statusRam     = readRecord[9];	
      opRam         =	readRecord[10];
			etebarRam     = readRecord[11] + ( readRecord[12] << 8) + ( readRecord[13] << 16) + ( readRecord[14] << 24);
			preEtebarRam  = readRecord[15] + ( readRecord[16] << 8) + ( readRecord[17] << 16) + ( readRecord[18] << 24);
			hPaymentRam   = readRecord[19] + ( readRecord[20] << 8) + ( readRecord[21] << 16) + ( readRecord[22] << 24);
			paymentRam    = readRecord[23] + ( readRecord[24] << 8) + ( readRecord[25] << 16) + ( readRecord[26] << 24);
			hNPaymentRam  = readRecord[27] + ( readRecord[28] << 8) + ( readRecord[29] << 16) + ( readRecord[30] << 24);
			yRam          = readRecord[31];
			MRam          = readRecord[32];
			dRam          = readRecord[33];
			hRam          = readRecord[34];
			mRam          = readRecord[35];
		  sprintf(sendToAndroid,"{\"C\":{\"Row\":%d,\"Status\":%d,\"UC\":%u,\"ID\":%u,\"Etebar\":%u,\"PreEtebar\":%u,\"Gt\":%u,\"Price\":%u,\"Hedie\":%u,\"Op\":%d,\"Y\":%d,\"m\":%d,\"D\":%d,\"H\":%d,\"M\":%d}}\n",
			row,statusRam,ucRam,cardIdRam,etebarRam,preEtebarRam,hPaymentRam,paymentRam,hNPaymentRam,opRam,yRam,MRam,dRam,hRam,mRam);
			#endif

			#if (deviceType==BusEtebar)
			cardIdRam     = readRecord[1] + ( readRecord[2] << 8) + ( readRecord[3] << 16) + ( readRecord[4] << 24);   
			ucRam         = readRecord[5] + ( readRecord[6] << 8) + ( readRecord[7] << 16) + ( readRecord[8] << 24);
			statusRam     = readRecord[9];	
      opRam         =	readRecord[10];
			etebarRam     = readRecord[11] + ( readRecord[12] << 8) + ( readRecord[13] << 16) + ( readRecord[14] << 24);
			preEtebarRam  = readRecord[15] + ( readRecord[16] << 8) + ( readRecord[17] << 16) + ( readRecord[18] << 24);
			paymentRam    = readRecord[19] + ( readRecord[20] << 8) + ( readRecord[21] << 16) + ( readRecord[22] << 24);
			yRam          = readRecord[23];
			MRam          = readRecord[24];
			dRam          = readRecord[25];
			hRam          = readRecord[26];
			mRam          = readRecord[27];
		  sprintf(sendToAndroid,"{\"C\":{\"Row\":%d,\"Status\":%d,\"UC\":%u,\"ID\":%u,\"Etebar\":%u,\"PreEtebar\":%u,\"Price\":%u,\"Op\":%d,\"Y\":%d,\"m\":%d,\"D\":%d,\"H\":%d,\"M\":%d}}\n",
			row,statusRam,ucRam,cardIdRam,etebarRam,preEtebarRam,paymentRam,opRam,yRam,MRam,dRam,hRam,mRam);
			#endif
			
			#if (deviceType==BCU)
			cardIdRam     = readRecord[1] + ( readRecord[2] << 8) + ( readRecord[3] << 16) + ( readRecord[4] << 24);   
			ucRam         = readRecord[5] + ( readRecord[6] << 8) + ( readRecord[7] << 16) + ( readRecord[8] << 24);
			statusRam     = readRecord[9];	
      opRam         =	readRecord[10];
			etebarRam     = readRecord[11] + ( readRecord[12] << 8) + ( readRecord[13] << 16) + ( readRecord[14] << 24);
			preEtebarRam  = readRecord[15] + ( readRecord[16] << 8) + ( readRecord[17] << 16) + ( readRecord[18] << 24);
			paymentRam    = readRecord[19] + ( readRecord[20] << 8) + ( readRecord[21] << 16) + ( readRecord[22] << 24);
			yRam          = readRecord[23];
			MRam          = readRecord[24];
			dRam          = readRecord[25];
			hRam          = readRecord[26];
			mRam          = readRecord[27];
			gpRam         = readRecord[28];
		  sprintf(sendToAndroid,"{\"C\":{\"Row\":%d,\"Status\":%d,\"UC\":%u,\"ID\":%u,\"Etebar\":%u,\"PreEtebar\":%u,\"Price\":%u,\"Gp\":%d,\"Op\":%d,\"Y\":%d,\"m\":%d,\"D\":%d,\"H\":%d,\"M\":%d}}\n",
			row,statusRam,ucRam,cardIdRam,etebarRam,preEtebarRam,paymentRam,gpRam,opRam,yRam,MRam,dRam,hRam,mRam);
			#endif
		  send(sendToAndroid); 
			HAL_Delay(100);
		}
		if ((trTail) && (trHead))
		{
			if (cntr == trHeadBuf)
			{
				if (Return)
					return;
				cntr = 0 - 1 ;
				trHeadBuf = trHead - 1;
				Return = 1;
			}
		}				
	}
}
//{"Receive":{"Status":19}}
//sssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss
#define INVALIDJson send("{\"Receive\":{\"Status\":190}}\n");
#define TIMEOUTJSON send("{\"C\":{\"Status\":1}}\n");
#define TIMEDATEERRORJSON send("{\"T\":{\"Status\":38}}\n");
//void Log(unsigned char *buf,unsigned int i)
//{
//	unsigned int len = strlen(buf);
//	unsigned char buff[len+sizeof(i)];
//	CDC_Transmit_FS((unsigned char*)buf,len);
//	HAL_Delay(1);
//	sprintf(buff,":%u\n",i);
//	CDC_Transmit_FS((unsigned char*)buff,strlen(buff));
//	led(0) 
//	HAL_Delay(1);
//	led(1)
//}

void sendVersion(void) {
	char buff[100]={0};

//	#define  id  0x1FFFF7E8
//	uint8_t  my_id[12], i;
//	for (i=0;i<12;i++)
//	{
//		 my_id[i]=*((uint8_t*) id+i);
//		 printf("%0X" , my_id[i] );
//	}
//	printf("\n");
	
	#if (deviceType==IO)
	sprintf(buff,"{\"Firmware\":{\"F\":1,\"DeviceType\":\"IO\",\"Release\":%d",Release);
	
	#elif (deviceType==BCU)
		#ifdef ExecTransactionBCU
		sprintf(buff,"{\"Firmware\":{\"F\":1,\"DeviceType\":\"BCU\",\"Release\":%d",Release);
		#else
		sprintf(buff,"{\"Firmware\":{\"F\":1,\"DeviceType\":\"APARK\",\"Release\":%d",Release);
		#endif
	
	#elif (deviceType==BD)
	sprintf(buff,"{\"Firmware\":{\"F\":1,\"DeviceType\":\"BD\",\"Release\":%d",Release);
	
	#elif (deviceType==Shop)
	sprintf(buff,"{\"Firmware\":{\"F\":1,\"DeviceType\":\"Shop\",\"Release\":%d",Release);
	
	#elif (deviceType==Sport)
  sprintf(buff,"{\"Firmware\":{\"F\":1,\"DeviceType\":\"Sport\",\"Release\":%d",Release);
	
	#elif (deviceType==AmusementPark)
	sprintf(buff,"{\"Firmware\":{\"F\":1,\"DeviceType\":\"AmusementPark\",\"Release\":%d",Release);
	
	#elif (deviceType==Parking)
	sprintf(buff,"{\"Firmware\":{\"F\":1,\"DeviceType\":\"Parking\",\"Release\":%d",Release);
	
	#elif (deviceType==BusEtebar)
	sprintf(buff,"{\"Firmware\":{\"F\":1,\"DeviceType\":\"BusEtebar\",\"Release\":%d",Release);
	
	#elif (deviceType==AutoPark)
	sprintf(buff,"{\"Firmware\":{\"F\":1,\"DeviceType\":\"AutoParking\",\"Release\":%d",Release);
	
	#elif (deviceType==Appointment)
	sprintf(buff,"{\"Firmware\":{\"F\":1,\"DeviceType\":\"Appointment\",\"Release\":%d",Release);
	
	#elif (deviceType==gateRoadblock)
	sprintf(buff,"{\"Firmware\":{\"F\":1,\"DeviceType\":\"GateRoadBlock\",\"Release\":%d",Release);

#else
#error "Wrong to choice deviceType [IO, Shop, Sport, Amusement, Parking or BusEtebar]"
#endif	
	send(buff);

	//***********************************************************************************withBoot
	#ifdef withBoot
	if ((*(__IO uint32_t *) 0x08000000) == 0x20004190) 
		send(",\"BootVer\":1");
	else if ((*(__IO uint32_t *) 0x08000000) == 0x20001a00)
		send(",\"BootVer\":2");
	else
		send(",\"BootVer\":3");
	#else
	send(",\"Boot\":0");
	#endif
	#ifdef mediaTechTab
	HAL_Delay(1);
	send(",\"mediaTechTab\":1");
	#endif	
	#ifdef AutoStart
	send(",\"AutoStart\":1");
	#else
	send(",\"AutoStart\":0");
	#endif	
	#ifdef PN532_Routine 
	HAL_Delay(1);
	//send("Reader Type : PN532\n");
	send(",\"ReaderType\":PN532");
	#endif	
	#ifdef RC522_Routine
	HAL_Delay(1);
	send(",\"ReaderType\":RC522");
	#endif		
	#ifdef finger 
	HAL_Delay(1);	
	//send(",\"Finger\":1");
	printf(",\"Finger\":%d",huart2.Init.BaudRate);
	#endif	
	#ifdef atlas
	HAL_Delay(1);	
	send(",\"Atlas\":1");
	#endif
	#ifdef Max30102
	HAL_Delay(1);	
	send(",\"HeartBeat\":1"); 
	#endif
	#ifdef heightHum
	HAL_Delay(1);	
	send(",\"HeightHum\":1"); 
	#endif
	#ifdef pos
		HAL_Delay(1);	
		#if (deviceType == Shop)
		printf(",\"Pos\":%d",huart3.Init.BaudRate);	
		#else
		printf(",\"Pos\":%d",huart2.Init.BaudRate);
		#endif
	#endif
	#ifdef UHF
	HAL_Delay(1);	
	send(",\"UHF\":1"); 
	#endif
	#ifdef DS7
	HAL_Delay(1);	
	//send(",\"DS7\":1"); 
	printf(",\"DS7\":%d",bound);
	#endif
	#ifdef remote
	HAL_Delay(1);	
	send(",\"Remote\":1"); 
	#endif
	#ifdef tarazo
	HAL_Delay(1);	
	//send(",\"Tarazo\":1");
	printf(",\"Tarazo\":%d",huart1.Init.BaudRate);	
	#endif
	
	#ifdef cardAccDis
	HAL_Delay(1);	
	//send(",\"cardAccDis\":1");
		printf(",\"cardAccDis\":%d",huart3.Init.BaudRate);
	#endif

	#ifdef ONEREADCARD
	HAL_Delay(1);	
	send(",\"oneReadCardInField\":1"); 
	#endif
	if (memID == 0XEF16) {
		HAL_Delay(1);	
		send(",\"MEM\":1");
	}	
	else {
		HAL_Delay(1);	
		send(",\"MEM\":0");
	}
	
  #ifdef LCD
	HAL_Delay(1);	
	send(",\"LCD\":1"); 
	#endif

  #ifdef NFC
	HAL_Delay(1);	
	send(",\"NFC\":1"); 
	#endif
	
	HAL_Delay(1);	
	send("}}\n");
//
//  sprintf(buff,"%s %s", __DATE__ ,__TIME__ );
//	printf("%s %s\n" , __DATE__ ,__TIME__ );	
  memset(fromAndroid,'\0',sizeof(fromAndroid));
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance==TIM3)
		{
			SoftUartHandler();
		}
	
	#ifdef remote 
		  if (htim->Instance==TIM2){
			//TIM2->SR = ~0x0001;
			TIM2->SR = ~TIM_IT_UPDATE;
			Time+=1000;	
		}
	#endif
}


//#ifndef atlas
//#ifdef Max30102
//void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
//{
//	if (max30102Flag) 
//		if (GPIO_Pin == INT_Pin)
//		{
//			Max30102_InterruptCallback();
//		}
//}
//#endif
//#endif
void ledBlik(char cnt,unsigned char delay) {
	for (char i=0;i<cnt;i++) {
		led(1);
		HAL_Delay(delay);
		led(0);
		HAL_Delay(delay);
		led(1);
	}
}

void serialSendCheck(void)
{
	if (!serialSendFlag) send("{\"Receive\":{\"Status\":26},\"C\":{\"Type\":\"UnSendSerial\"}}\n");
	if (serialSendFlag)  send("{\"Receive\":{\"Status\":26},\"C\":{\"Type\":\"SendSerial\"}}\n");
	serialSendFlag =!serialSendFlag;
	memset(fromAndroid,'\0',sizeof(fromAndroid));
}

#if ((deviceType==BusEtebar) || (deviceType==AmusementPark) || (deviceType==BCU) || (deviceType==BD))
void etebarCheck(void) {
	send("{\"Receive\":{\"Status\":26},\"C\":{\"Type\":\"Etebar\"}}\n");
	etebarFlag = 0;
	memset(fromAndroid,'\0',sizeof(fromAndroid));
}

void ziroEtebarCheck(void)
{
	
	#if (deviceType==AmusementPark)
	crc = 0;
	for (unsigned char cnt = 0;cnt<19;cnt++)
		crc ^= fromAndroid[cnt];
	if (crc != fromAndroid[19])
	{
		INVALIDJson;
		rxAndroid = 1;
	  memset(fromAndroid,'\0',sizeof(fromAndroid));
		return;
	}
	#endif
	send("{\"Receive\":{\"Status\":26},\"C\":{\"Type\":\"ZiroEtebar\"}}\n");
	Year = fromAndroid[1];
	Month = fromAndroid[2];
	Day = fromAndroid[3];
	Hour = fromAndroid[4];
	Min = fromAndroid[5];
	Sec = fromAndroid[6];
	
	deviceID = (fromAndroid[7]<<24) + (fromAndroid[8]<<16) + (fromAndroid[9]<<8) + fromAndroid[10];
	zeroEtebarFlag = 0;		
	#if (deviceType==AmusementPark)
	ziroEtebarID = (fromAndroid[11]<<24) + (fromAndroid[12]<<16) + (fromAndroid[13]<<8) + fromAndroid[14];
	ziroEtebarUC = (fromAndroid[15]<<24) + (fromAndroid[16]<<16) + (fromAndroid[17]<<8) + fromAndroid[18];
	serialSendFlag = 1;	
	flagRc522 = 0;
	#else
	serialSendFlag = 0;	
	#endif
	
	rxAndroid = 1;
	memset(fromAndroid,'\0',sizeof(fromAndroid));
}

void canselCheck(void) {
	HAL_Delay(1);
	send("{\"Receive\":{\"Status\":26},\"C\":{\"Type\":\"Cansel\"}}\n"); 
	type 					 = 0;
	etebarFlag 		 = 1;
	zeroEtebarFlag = 1;
	rxAndroid 		 = 1; 
	#if (deviceType == AmusementPark)
	decreaseFlag 	 = 0;
	#endif
	serialSendFlag = 0;
	memset(fromAndroid, '\0', sizeof(fromAndroid));
}
#endif


void relayState(char state, char num) {
	if (num == '1') {
		if (state == 'n') {
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, 1);
			HAL_Delay(1);
			send("{\"Receive\":{\"Status\":26},\"R1\":{\"Status\":23}}\n");
			rxAndroid = 1;
			memset(fromAndroid, '\0', sizeof(fromAndroid));
		}
		if (state == 'f') {
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, 0);
			HAL_Delay(1);
			send("{\"Receive\":{\"Status\":26},\"R1\":{\"Status\":24}}\n");
			rxAndroid = 1;
			memset(fromAndroid,'\0',sizeof(fromAndroid));
		}		
	}
	if (num == '2') {
		if (state == 'n') {
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, 1);
			HAL_Delay(1);
			send("{\"Receive\":{\"Status\":26},\"R2\":{\"Status\":23}}\n");
			rxAndroid = 1;
			memset(fromAndroid, '\0', sizeof(fromAndroid));
		}
		if (state == 'f') {
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, 0);
			HAL_Delay(1);
			send("{\"Receive\":{\"Status\":26},\"R2\":{\"Status\":24}}\n");
			rxAndroid = 1;
			memset(fromAndroid, '\0', sizeof(fromAndroid));
		}		
	}			
}

#if ((deviceType==BusEtebar) || (deviceType==AmusementPark))
void RC522ExecTransactionCheck (void) {
	//2A 01 00004f30 00000080 00000000 64 3B9AC9FF 00000fff 000A060d3937 d8
	unsigned long int IDD = 0;
	crc = 0;
	for (char cnt=0; cnt<41; cnt++)
		crc ^= fromAndroid[cnt];
	
	if ((crc != fromAndroid[41]) ||  (fromAndroid[14] != 0x64)) { 
		send("{\"Receive\":{\"Status\":19}}\n");  
		reciveDataFlag 			= 0;
		flagRc522 		 			= 0;
		crc 								= 0;
		execTransactionFlag = 0;
		memset(fromAndroid, '\0', sizeof(fromAndroid));
		return;
	}
	else if ((crc == fromAndroid[41]) || (fromAndroid[14] == 0x64)) {
		memset(HediyeBuf, '\0', sizeof(HediyeBuf));
		type    = (int)fromAndroid[1]; //01
		payMent = (fromAndroid[2] << 24) + (fromAndroid[3] << 16) + (fromAndroid[4] << 8) + fromAndroid[5];  // 00 02 71 00
		Hetebar = (fromAndroid[6] << 24) + (fromAndroid[7] << 16) + (fromAndroid[8] << 8) + fromAndroid[9];
		
		//Written by HNA;
		HediyeBuf[0] = fromAndroid[9];
		HediyeBuf[1] = fromAndroid[8];
		HediyeBuf[2] = fromAndroid[7];
		HediyeBuf[3] = fromAndroid[6];
		//HNA
		
		MINEtebar = (fromAndroid[10] << 24) + (fromAndroid[11] << 16) + (fromAndroid[12] << 8) + fromAndroid[13]; // 00 00 00 00
		MAXEtebar = (fromAndroid[15] << 24) + (fromAndroid[16] << 16) + (fromAndroid[17] << 8) + fromAndroid[18]; // 00 89 54 40
		deviceID  = (fromAndroid[19] << 24) + (fromAndroid[20] << 16) + (fromAndroid[21] << 8) + fromAndroid[22]; // 00 01 01 01
		Year  = fromAndroid[23];
		Month = fromAndroid[24];
		Day   = fromAndroid[25];
		Hour  = fromAndroid[26];
		Min   = fromAndroid[27];
		Sec   = fromAndroid[28];
		configUc  = (fromAndroid[29] << 24) + (fromAndroid[30] << 16) + (fromAndroid[31] << 8) + fromAndroid[32];   // 00 01 01 01
		IDD 		  = (fromAndroid[33] << 24) + (fromAndroid[34] << 16) + (fromAndroid[35] << 8) + fromAndroid[36];
		requestID = (fromAndroid[37] << 24) + (fromAndroid[38] << 16) + (fromAndroid[39] << 8) + fromAndroid[40];
		
		if (requestID == preRequestID)
			send(sendingResult);
		else
			IncrementCreditToCard(IDD, payMent, Hetebar, requestID);
		memset(fromAndroid, '\0', sizeof(fromAndroid));
		flagRc522 = 0;
		crc 			= 0;
	}	
}

//================================================================================================
#if defined ExecTransaction || defined(ExecTransactionBus)
void RC522ExecDecreaseTransactionCheck(void) {
	//23 01 00004f30 00000080 00000000 64 3B9AC9FF 00000fff 000A060d3937 d8
	#define CANSELLJSON send("{\"Receive\":{\"Status\":26},\"C\":{\"Type\":\"Cansel\"}}\n");
	#define VALIDJSON   send("{\"Receive\":{\"Status\":26},\"C\":{\"Type\":\"DecreaseTransaction\"}}\n");
	unsigned char buf[50],snr[6];
	unsigned long int IDD = 0, ti = 0, preHetebar;
	unsigned char idx = 0;
	unsigned int Gift = 0;
	crc = 0;
	for (char cnt=0; cnt<41; cnt++)
		crc ^= fromAndroid[cnt];
	
	if ((crc != fromAndroid[41]) || (fromAndroid[14] != 0x64)) { 
		INVALIDJson 
		decreaseFlag = 0;  
		crc = 0; 
		memset(fromAndroid,'\0',sizeof(fromAndroid));
		return;
	}
	else if ((crc == fromAndroid[41]) || (fromAndroid[14] == 0x64)){
		type 			 = (int)fromAndroid[1]; //01
		payMent 	 = (fromAndroid[2]<<24) + (fromAndroid[3]<<16) + (fromAndroid[4]<<8) + fromAndroid[5];        // 00 02 71 00
		//if (payMent == prePayment)
			//prePayment = payMent;
		preHetebar = (fromAndroid[6]<<24) + (fromAndroid[7]<<16) + (fromAndroid[8]<<8) + fromAndroid[9];
		
		//Written by HNA;
		HediyeBuf[0] = fromAndroid[9];
		HediyeBuf[1] = fromAndroid[8];
		HediyeBuf[2] = fromAndroid[7];
		HediyeBuf[3] = fromAndroid[6];
		//HNA
		
		MINEtebar = (fromAndroid[10]<<24) + (fromAndroid[11]<<16) + (fromAndroid[12]<<8) + fromAndroid[13];  // 00 00 00 00
		MAXEtebar = (fromAndroid[15]<<24) + (fromAndroid[16]<<16) + (fromAndroid[17]<<8) + fromAndroid[18];  // 00 89 54 40
		deviceID  = (fromAndroid[19]<<24) + (fromAndroid[20]<<16) + (fromAndroid[21]<<8) + fromAndroid[22];   // 00 01 01 01
		Year  		= fromAndroid[23];
		Month 		= fromAndroid[24];
		Day 			= fromAndroid[25];
		Hour  		= fromAndroid[26];
		Min 			= fromAndroid[27];
		Sec 			= fromAndroid[28];
		configUc  = (fromAndroid[29] << 24) + (fromAndroid[30] << 16) + (fromAndroid[31] << 8) + fromAndroid[32];   // 00 01 01 01
		IDD 		  = (fromAndroid[33] << 24) + (fromAndroid[34] << 16) + (fromAndroid[35] << 8) + fromAndroid[36];
		requestID = (fromAndroid[37] << 24) + (fromAndroid[38] << 16) + (fromAndroid[39] << 8) + fromAndroid[40];
		
		char doTransaction = 1;
		if (requestID == preRequestID) {
			send(sendingResult);
			doTransaction = 0; //Do nothing until new request is arrived.
		}
		
		CDCReceiveLen = 0;
		//VALIDJSON
		ti = HAL_GetTick();
		while (doTransaction) {
			WDTR
			if (HAL_GetTick() - ti > 16000) {
				TIMEOUTJSON
				break;
			}
			if (CDCReceiveLen) {
				if ((fromAndroid[0] == 'c') || (fromAndroid[0] == 'C')) {
					CANSELLJSON;
					break;
				}
				else
					CDCReceiveLen = 0;	
			}
			
			if (ISO14443_SingleTagSelect(snr)) {
				if (ISO14443_SingleTagSelect(snr))
					continue;
			}

			state = Old_ExecTransaction(19,&IDD,&configUc,payMent,MINEtebar,MAXEtebar,deviceID,&etebar,&preetebar,&lastDevice,&LastoP,&oPIndex);

			/*if ((state == 9) || (state == 1)) 
				state = Old_ExecTransaction(19,&IDD,&configUc,payMent,MINEtebar,MAXEtebar,deviceID,&etebar,&preetebar,&lastDevice,&LastoP,&oPIndex);
			if ((state == 9) || (state == 1)) 
				state = Old_ExecTransaction(19,&IDD,&configUc,payMent,MINEtebar,MAXEtebar,deviceID,&etebar,&preetebar,&lastDevice,&LastoP,&oPIndex);*/
			
			Hetebar  =  HediyeBuf[0];
			Hetebar += (HediyeBuf[1] << 8);
			Hetebar += (HediyeBuf[2] << 16);
			Hetebar += (HediyeBuf[3] << 24);
			
			sprintf(sendingResult, "{\"C\":{\"Status\":%d,\"UC\":%u,\"ID\":%u,\"" \
				"Etebar\":%d,\"PreHedie\":%d,\"Hedie\":%d,\"PreEtebar\":%d,\"Price\"" \
				":%d,\"EEM\":%u}}\n", state, configUc, IDD, etebar, 
				preHetebar, Hetebar, preetebar, payMent, requestID);

			send(sendingResult);
			lastTransactionSent = 0; //HAL_GetTick();
			
			if (state == 0) {// اگر تمام مراحل کارت بدرستي انجام شد
				cardReadStep = 1;
				//lastSuccessfulID = HAL_GetTick();
			}
			preRequestID = requestID;
			
			buf[idx++] = deviceType;			
			buf[idx++] = IDD & 0xFF;
			buf[idx++] = ( IDD >> 8) & 0xFF;
			buf[idx++] = ( IDD >> 16) & 0xFF;
			buf[idx++] = ( IDD >> 24) & 0xFF;
			
			buf[idx++] =  configUc & 0xFF;
			buf[idx++] = ( configUc >> 8) & 0xFF;
			buf[idx++] = ( configUc >> 16) & 0xFF;
			buf[idx++] = ( configUc >> 24) & 0xFF;
			buf[idx++] = state;
			
			buf[idx++] = 2; // type
			
			buf[idx++] =  etebar & 0xFF;
			buf[idx++] = ( etebar >> 8) & 0xFF;
			buf[idx++] = ( etebar >> 16) & 0xFF;
			buf[idx++] = ( etebar >> 24) & 0xFF;

			buf[idx++] =  preetebar & 0xFF;
			buf[idx++] = ( preetebar >> 8) & 0xFF;
			buf[idx++] = ( preetebar >> 16) & 0xFF;
			buf[idx++] = ( preetebar >> 24) & 0xFF;
			
			#if defined ExecTransaction
			buf[idx++] = 0;
			buf[idx++] = 0;
			buf[idx++] = 0;
			buf[idx++] = 0;
			#endif
			
			buf[idx++] =  payMent & 0xFF;
			buf[idx++] = ( payMent >> 8) & 0xFF;
			buf[idx++] = ( payMent >> 16) & 0xFF;
			buf[idx++] = ( payMent >> 24) & 0xFF;
			
			#if defined ExecTransaction
			buf[idx++] =  0;
			buf[idx++] =  0;
			buf[idx++] =  0;
			buf[idx++] =  0;
			#endif
			
			buf[idx++] = Year;
			buf[idx++] = Month;
			buf[idx++] = Day;
			buf[idx++] = Hour;
			buf[idx++] = Min;
		  if (SPI_Flash_ReadID() == 0XEF16) {
//				SaveTransactionRecord(buf,idx);
				SaveRingDetail();
			}
			break;
		}
		decreaseFlag  = 0;
		crc 					= 0;
		CDCReceiveLen = 0;
		return;
	}
}
#endif
//********************************************************************************
#if defined ExecTransaction
void RC522WriteGpCardCheck(void)
{
	crc=0;
//	68 subgp darsad 
//	subgp == 0  { 68 subgp Y M D H M S crc} 
//	subgp == 22 { 68 subgp darsad Ye Me De Hs Ms He Me Y M D H M S crc } 
//	subgp == 23 { 68 subgp Y M D H M S crc}
//	subgp == 24 { 68 subgp Ye Me De Hs Ms He Me Y M D H M S crc } 
//	subgp == 25 { 68 subgp Ye Me De Hs Ms He Me Y M D H M S crc }
	
	switch (fromAndroid[1])
	{
		case 0: 
			Year = fromAndroid[2];
			Month = fromAndroid[3];
			Day = fromAndroid[4];
			Hour = fromAndroid[5];
			Min = fromAndroid[6];
			Sec = fromAndroid[7];
			for (char cnt=0;cnt<8;cnt++)
				crc ^= fromAndroid[cnt];
			if (crc == fromAndroid[8]) reciveAck = 1;
			break;
		
		case 22: 	
			Year = fromAndroid[10];  // example packet: 68 16 32 0a 0b 0f 12 14 17 16 00 0a 06 11 26 00 7e 
			Month = fromAndroid[11];
			Day = fromAndroid[12];
			Hour = fromAndroid[13];
			Min = fromAndroid[14];
			Sec = fromAndroid[15];
			for (char cnt=0;cnt<16;cnt++)
				crc ^= fromAndroid[cnt];
			if (crc == fromAndroid[16]) reciveAck = 1;			
			break;
		
		case 23: 
			Year = fromAndroid[2];
			Month = fromAndroid[3];
			Day = fromAndroid[4];
			Hour = fromAndroid[5];
			Min = fromAndroid[6];
			Sec = fromAndroid[7];
			for (char cnt=0;cnt<8;cnt++)
				crc ^= fromAndroid[cnt];
			if (crc == fromAndroid[8]) reciveAck = 1;
			break;			
		
		case 24: 
			Year = fromAndroid[9];
			Month = fromAndroid[10];
			Day = fromAndroid[11];
			Hour = fromAndroid[12];
			Min = fromAndroid[13];
			Sec = fromAndroid[14];
			for (char cnt=0;cnt<15;cnt++)
				crc ^= fromAndroid[cnt];
			if (crc == fromAndroid[15]) reciveAck = 1;	
			break;
		
		case 25: 
			Year = fromAndroid[9];
			Month = fromAndroid[10];
			Day = fromAndroid[11];
			Hour = fromAndroid[12];
			Min = fromAndroid[13];
			Sec = fromAndroid[14];
			for (char cnt=0;cnt<15;cnt++)
				crc ^= fromAndroid[cnt];
			if (crc == fromAndroid[15]) reciveAck = 1;				
			break;
		default: reciveAck = 0; break;
	}
	if (reciveAck) 
	{
		MINEtebar = 0;
		send("{\"Receive\":{\"Status\":26,\"Type\":\"ChangeOfGroup\"}}\n"); 
		Write_GroupOfCard(fromAndroid[1]);
		crc = 0;		
		reciveAck = 0;
		memset(fromAndroid,'\0',sizeof(fromAndroid));
	}
	else
	{
		send("{\"Receive\":{\"Status\":19}}\n");
		reciveAck = 0;
		crc = 0;
		memset(fromAndroid,'\0',sizeof(fromAndroid));
	}
}
#endif
#endif

#ifdef atlas
void atlasSendIDProcess(int state)
{
	if (state)
	{
		send("{\"Receive\":{\"Status\":26},\"A\":{\"Type\":\"SendID\"}}\n");  
		atlasSendFlag = 1;
		HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
		memset(fromAndroid,'\0',sizeof(fromAndroid));
	}
	else 
	{
		send("{\"Receive\":{\"Status\":26},\"A\":{\"Type\":\"UnSendID\"}}\n");  
		atlasSendFlag = 0;
		HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
		memset(fromAndroid,'\0',sizeof(fromAndroid));		
	}
}
#endif
//************************************************************************************************
#ifdef finger
void DeleteAllTemplateProcess(void)// 'D'  Android send
{
			FPRN_DeleteAllTemplate();
			IdentifyFlag=0; 
			memset(fromAndroid,'\0',sizeof(fromAndroid));
}

//************************************************************************************************
void enrollProcess(void) {// 'e'  Android send 
	char z = 0;
	unsigned char ResBuff[2]   = {0};
	unsigned char ID_Number[5] = {0};	
	
	for (char i=0; i<10; i++) {
		if (fromAndroid[i] == 'L') {
			z = 2;
			break;
		}
		ID_Number[i] = fromAndroid[i + 1];
	}

	if (z >= 2) {	
//										sscanf(fromAndroid,"e%dL",&idFR);											
		#if (deviceType == Shop)
		ResBuff[0]=idFR%256;
		ResBuff[1]=idFR/256;
		bufferLen = 0;
		FPRNCancelCmd();
		for (z = 0 ; z < 3 ; z++)
		{
		FPRN_Send_Command(0x0105,2,ResBuff); // delet
		HAL_Delay(150);		
		if ((GetResponse[0] == 0xAA) && (GetResponse[2] == 0x05) && (GetResponse[8]))
		break;
		WDTR
		HAL_Delay(900);
		if ((GetResponse[0] == 0xAA) && (GetResponse[2] == 0x05) && (GetResponse[8]))
		break;	
		}
		#endif
		idFR = atoi(ID_Number);		
		FPRN_AddNewUser(idFR);

		//		#if defined tarazo
		//		__HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
		//		#endif

		memset(fromAndroid, '\0', sizeof(fromAndroid)); 
		CDCReceiveLen = rxBufferHeadPos = 0;
		enrollFlag 		= 0;	
	}
	else {
		state = 19; 
		send("{\"Receive\":{\"Status\":19}}"); 
		enrollFlag = 0;	
		idFR 			 = 0; 
		z 				 = 0; 
		memset(fromAndroid, '\0', sizeof(fromAndroid));
		CDCReceiveLen = rxBufferHeadPos = 0;
	}
}

//*****************************************************************************************************
void deleteTemplateProcess(void) {  // 'd'  Android send  delete Template Process
	char z = 0;
	unsigned char ID_Number[5] = {0};	
	for (char i=0; i<10; i++) {							
		if (fromAndroid[i] == 't') {
			z = 2; 
			break;
		}
		ID_Number[i] = fromAndroid[i + 1];														 							
	}
	
	if (z == 2) {  
//							sscanf(fromAndroid,"d%dt",&idFR);
		idFR = atoi(ID_Number);
		FPRN_DeleteTemplate(idFR);
		memset(fromAndroid, '\0', sizeof(fromAndroid)); 
		deleteTemplateFlag = 0;	
	}
	else {
		state = 19;  
		send("{\"Receive\":{\"Status\":19}}\n");  
		deleteTemplateFlag = 0;	
		idFR = 0; 
		z 	 = 0; 
		memset(fromAndroid, '\0', sizeof(fromAndroid));
	}
}
//*****************************************************************************************************
void identifyProcess(void) {//i Android send identify Process
	send("{\"Receive\":{\"Status\":26},\"F\":{\"Type\":\"Identify\"}}\n");  
	HAL_Delay(1);
	IdentifyFlag=1;
	memset(fromAndroid,'\0',sizeof(fromAndroid));
}

//*****************************************************************************************************
void identifyOffProcess(void)  //I Android send identify Off Process
{
						send("{\"Receive\":{\"Status\":26},\"F\":{\"Type\":\"IdentifyOff\"}}\n");  
						FPRNCancelCmd();
						IdentifyFlag=0;
						memset(fromAndroid,'\0',sizeof(fromAndroid));
}
//*****************************************************************************************************
void cancelProcess(void) //  'C'   Android send   cancel Process
{
						HAL_Delay(1);
						send("{\"Receive\":{\"Status\":26},\"F\":{\"Type\":\"Cansel\"}}\n");  
						FPRN_CancelCmd();
						memset(fromAndroid,'\0',sizeof(fromAndroid));
}

//******************************************************************************************************
void readTempalteProcess(void) {// 'R'  Android send  read Tempalte Process
	char z = 0, h = 0;
	unsigned char ID_Number[5] = {0};	
	for (char i=0; i<10; i++) {
		if (fromAndroid[i] == 'd') {
			z = 2;
			break;
		}
		else if (fromAndroid[i] == 'h') {
			h = 2;
			break;
		}
		ID_Number[i] = fromAndroid[i + 1];
	}

	if (z == 2) {  
	//							sscanf(fromAndroid,"R%dd",&idFR);	
		idFR = atoi(ID_Number);
		FPRN_GetTemplateByID(idFR, 0); 
	}
	else if (h == 2) {
		idFR = atoi(ID_Number);
		FPRN_GetTemplateByID(idFR, 1); 
	}
	else 
		send("{\"Receive\":{\"Status\":19}}\n");
	memset(fromAndroid,'\0',sizeof(fromAndroid));
	readTemplateFlag = 0;
	idFR						 = 0; 
	z 							 = 0;
}
#endif
//*********************************************************************************************
//*********************************************************************************************sendResponsePos
#ifdef pos
//**************************************************************
void sendResponsePos(unsigned char *buf , unsigned int len) { //  UART TO Android
	#define successful 						 0
	#define timeOut 							 1
	#define invalidData 					 2
	#define canselByCustomer 			 3
	#define adameErtebatBaMarkaz   5
	#define repeatTransaction 		19
	#define mojodiKafiNis 				51
	#define invalidPass 					55
	#define saghfTrakonesh 				61
	#define notresponseFromServer 68
	#define ramzBishazHad 				75
	#define diactiveCard 					78  
	
	unsigned int responseCode = 0;
	unsigned char y,m,d,h,M;
	unsigned int i = 0,j = 0;
	unsigned char buff[100];
	if (posType == 0) {
		//CDC_Transmit_FS(buf,len);
		HAL_Delay(10);
		return;
	}
	//*********************************************************************************pasargard
	else if (posType == 3)	{
		if ((buf[4] == 9) && (buf[5] == 0x62) && (buf[6] == 0x45) && 
			(buf[7] == 0x70) && (buf[16] == 0x31) && (buf[17] == 0x32)
			&& (buf[18] == 0x34) && (buf[19] == 0x35) && (buf[23] == 0x1c))
				responseCode = ((buf[20] - 0x30) * 100 ) + ((buf[21] - 0x30) * 10) + (buf[22] - 0x30);
		else {			
			send("{\"P\":{\"responseCode\":-1}}\n");
			return;
		}
		
		if (responseCode == 0) {// successful
			j = 0;
			for (i = 0 ; i <= len ; i++)
				if (buf[i] == 0x1c) {
					j++;
					if (j == 4)
						break;
				}
			y = ((buf[i+1] - 0x30) * 10) + ((buf[i+2]  - 0x30));
			m = ((buf[i+3] - 0x30) * 10) + ((buf[i+4]  - 0x30));
			d = ((buf[i+5] - 0x30) * 10) + ((buf[i+6]  - 0x30));
			h = ((buf[i+7] - 0x30) * 10) + ((buf[i+8]  - 0x30));
			M = ((buf[i+9] - 0x30) * 10) + ((buf[i+10] - 0x30));	
			if (buf[i+11] != 0x1c) {
				y = m = d = h = m = 0;
			}
			sprintf(buff,"{\"P\":{\"resCode\":%d,\"Amount\":%ld,\"H\":%d,\"M\":%d,\"Y\":%d,\"m\":%d,\"D\":%d}}\n",responseCode,amount,h,M,y,m,d);
		}
		else if (responseCode == 55) // invalidPass
			sprintf(buff,"{\"P\":{\"resCode\":%d,\"Amount\":%ld,\"H\":%d,\"M\":%d,\"Y\":%d,\"m\":%d,\"D\":%d}}\n",responseCode,amount,0,0,0,0,0);
		else if (responseCode == 78) // diactiveCard
			sprintf(buff,"{\"P\":{\"resCode\":%d,\"Amount\":%ld,\"H\":%d,\"M\":%d,\"Y\":%d,\"m\":%d,\"D\":%d}}\n",responseCode,amount,0,0,0,0,0);
		else if (responseCode == 128) // canselByCustomer
			sprintf(buff,"{\"P\":{\"resCode\":%d,\"Amount\":%ld,\"H\":%d,\"M\":%d,\"Y\":%d,\"m\":%d,\"D\":%d}}\n",canselByCustomer,amount,0,0,0,0,0);
		else if (responseCode == 75) // ramzBishazHad
			sprintf(buff,"{\"P\":{\"resCode\":%d,\"Amount\":%ld,\"H\":%d,\"M\":%d,\"Y\":%d,\"m\":%d,\"D\":%d}}\n",responseCode,amount,0,0,0,0,0);
		else if (responseCode == 61) // saghfTrakonesh
			sprintf(buff,"{\"P\":{\"resCode\":%d,\"Amount\":%ld,\"H\":%d,\"M\":%d,\"Y\":%d,\"m\":%d,\"D\":%d}}\n",responseCode,amount,0,0,0,0,0);
		else if (responseCode == 51) // mojodiKafiNis
			sprintf(buff,"{\"P\":{\"resCode\":%d,\"Amount\":%ld,\"H\":%d,\"M\":%d,\"Y\":%d,\"m\":%d,\"D\":%d}}\n",responseCode,amount,0,0,0,0,0);
		else if (responseCode == 19) // repeatTransaction
			sprintf(buff,"{\"P\":{\"resCode\":%d,\"Amount\":%ld,\"H\":%d,\"M\":%d,\"Y\":%d,\"m\":%d,\"D\":%d}}\n",responseCode,amount,0,0,0,0,0);
		else if (responseCode == 127 ) // timeOut
			sprintf(buff,"{\"P\":{\"resCode\":%d,\"Amount\":%ld,\"H\":%d,\"M\":%d,\"Y\":%d,\"m\":%d,\"D\":%d}}\n",timeOut,amount,0,0,0,0,0);
		else //Cancel
			sprintf(buff,"{\"P\":{\"resCode\":%d,\"Amount\":%ld,\"H\":%d,\"M\":%d,\"Y\":%d,\"m\":%d,\"D\":%d}}\n",responseCode,amount,0,0,0,0,0);
		send(buff);
		HAL_Delay(1);
		return;		
	}
	//****************************************************************************************saman
	else if (posType == 4) {
		/*send("\n=============================================\n");
		for (uint16_t i=0; i<len; i++) {
			sprintf(buff, "%02X ", buf[i]);
			send(buff);
		}
		send("\n=============================================\n");*/
		
		if (buf[6] == 0x72) {
			for (i=0; i<=len; i++) {
				if ((buf[i] == 0x88) && (buf[i+1] == 0x02) && (buf[i + 2] == 0x01))
					break;
			}
			if (i >= len) {
				send("{\"P\":{\"responseCode\":-1}}\n");
				return ;
			}

			switch (buf[i + 3]) {
				case 0: {
					for (j=0; j<=len; j++) {
						if ((buf[j]==0x87) && (buf[j+1]==0x13))
							break;
					}
					if (j>=len)
						j = 0;
					
					if (j == 0)
						sprintf(buff,"{\"P\":{\"resCode\":%d,\"Amount\":%ld,\"H\":%d,\"M\":%d,\"Y\":%d,\"m\":%d,\"D\":%d}}\n",buf[i+3],amount,0,0,0,0,0);
					else {
						y = ( ( buf[j+4] -  0x30) * 10 ) + ( ( buf[j+5]  - 0x30 ) );
						m = ( ( buf[j+7] -  0x30) * 10 ) + ( ( buf[j+8]  - 0x30 ) );
						d = ( ( buf[j+10] - 0x30) * 10 ) + ( ( buf[j+11] - 0x30 ) );
						h = ( ( buf[j+13] - 0x30) * 10 ) + ( ( buf[j+14] - 0x30 ) );
						M = ( ( buf[j+16] - 0x30) * 10 ) + ( ( buf[j+17] - 0x30 ) );
						sprintf(buff,"{\"P\":{\"resCode\":%d,\"Amount\":%ld,\"H\":%d,\"M\":%d,\"Y\":%d,\"m\":%d,\"D\":%d}}\n",buf[i+3],amount,h,M,y,m,d);
					}
					send(buff);
					break;
				}
				default:
					sprintf(buff,"{\"P\":{\"resCode\":%d,\"Amount\":%ld,\"H\":%d,\"M\":%d,\"Y\":%d,\"m\":%d,\"D\":%d}}\n",buf[i+3],amount,0,0,0,0,0);
					send(buff);
					break;
			}
			HAL_Delay(1);
			return;
		}
		else if (len > 1)
			send("{\"P\":{\"responseCode\":-1}}\n");
	}
}
//*************************************************************makePosNumberArray
char makePosNumberArray(unsigned int Num,unsigned char *NumStr) {
	unsigned short i = 0, j = 0;
	unsigned int Temp, Pow = 1;
	unsigned char NumCount = 0;	
		
  Temp = Num; 
  i = 9;
  while (i) {
	  Pow = 1;
		for (j=0; j<i; j++) 
			Pow *= 10;  
	  if (Temp/Pow) {
			i++; 
			break;
		}
		i--;
  }
	
  NumCount = i;
  i = 0;
  while (i<NumCount) {
    NumStr[i++] = (Temp/Pow) + 0x30; 
		Temp %= Pow;
	  Pow /=  10;
	}
  return NumCount;
}	
//************************************************************pos_SendSerialData
void pos_SendSerialData(unsigned char *buff) {
	#if (deviceType == Sport)   
	HAL_UART_Transmit(&huart3,(unsigned char*)buff ,1,1);
	#else
	HAL_UART_Transmit(uart_no == 2 ? &huart2 : &huart3,(unsigned char*)buff ,1,1);//2
	#endif
	return;
}
//***********************************************************pos
char Pos()  {//Android TO UART 
	/*
	if ((fromAndroid[1] < 0x31) ||  (fromAndroid[1] > 0x36))
	{
		send("{\"Receive\":{\"Status\":19}}\n"); 
		memset(fromAndroid,'\0',sizeof(fromAndroid));
		return 0; 
	}
	*/
	unsigned short idx = 0;
	//unsigned char globalBuff[100]={0};
	unsigned char NumStr[10],NumCount=0,sha[20],checkSum=0;
	
	/*unsigned char pasargadTest[] = {
		0x00, 0x00 ,0x00 ,0x0E ,0x02 ,0xF9 ,0x45 ,0x70 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00, 
		0x00 ,0x31 ,0x32 ,0x34 ,0x35 ,0x1C ,0x32 ,0x30 ,0x30 ,0x30 ,0x30 ,0x1C ,0x1C ,0x1C ,0x1C,
		0x16 ,0x56 ,0x80 ,0x0f ,0x73 ,0x93 ,0xfc ,0xce ,0xe6 ,0xaa ,0xbf ,0x7a ,0x45 ,0x3c ,0x6a,
		0x6e ,0xa8 ,0x31 ,0x44 ,0xfd
	};
		
	unsigned char samanKishTest[] = {
	  	0x02, 0x63, 0x00, 0x00, 0x00, 0x01, 0x72, 0x5D, 0xB1, 0x13, 0x81, 0x04, 0x31, 0x30, 0x30, 
		  0x30, 0x82, 0x02, 0x3A, 0x0A, 0x83, 0x01, 0x01, 0x84, 0x01, 0x03, 0x86, 0x01, 0x00, 0xB2,
		  0x46, 0xA1, 0x1B, 0x81, 0x04, 0x31, 0x30, 0x30, 0x30, 0x82, 0x01, 0x30, 0x83, 0x01, 0x30,
		  0x84, 0x01, 0x30, 0x85, 0x01, 0x30, 0x86, 0x01, 0x30, 0x87, 0x01, 0x30, 0x88, 0x01, 0x30,
		  0xA2, 0x27, 0xA1, 0x05, 0x81, 0x80, 0x82, 0x01, 0x5F, 0xA1, 0x11, 0x81, 0x06, 0x44, 0x6C,
		  0x6C, 0x56, 0x65, 0x72, 0x82, 0x07, 0x32, 0x2E, 0x39, 0x2E, 0x30, 0x2E, 0x30, 0xA1, 0x0B,
		  0x81, 0x06, 0x50, 0x72, 0x67, 0x56, 0x65, 0x72, 0x82, 0x01, 0x5F, 0x8E, 0x53, 0x4A, 0x66,
	};*/
	
 	if (fromAndroid[1] == 1) {//------test pos pasargad 
		posType = 3;
		//for (unsigned int cnt=0; cnt<= sizeof(pasargadTest); cnt++)
			//pos_SendSerialData(&pasargadTest[cnt]);
	}
	else if (fromAndroid[1] == 6) {//-test pos parsean
		posType = 5;
		sprintf(globalBuff, "0068{\"cmd\":10,\"amount\":%d,\"servicM\":\"000000\"" \
			",\"sign\":\"899|123456789\"}\n", amount);
		for (unsigned int cnt = 0 ; cnt < sizeof(globalBuff) ; cnt++)
		pos_SendSerialData(&globalBuff[cnt]);
	}

	else if (fromAndroid[1] == 2) {//-test pos saman kish 
		posType = 4;
		//for (unsigned int cnt=0; cnt<=sizeof(samanKishTest); cnt++)
			//pos_SendSerialData(&samanKishTest[cnt]);
	}
	else if (fromAndroid[1] == 5) {//-send to pasargad V3
		posType = 3;
		idx = 0;
		if (fromAndroid[2] < 8) {			
			INVALIDJson;
			return 1;
		}
		checkSum = fromAndroid[3] ^ fromAndroid[4] ^ fromAndroid[5] ^ fromAndroid[6];
		if (fromAndroid[7] != checkSum) {
			INVALIDJson;
			return 1;
		}
		amount = ((fromAndroid[3] << 24) + (fromAndroid[4] << 16) + 
							(fromAndroid[5] << 8) + (fromAndroid[6] & 0xff));
		
		NumCount = makePosNumberArray(amount, NumStr);
		
		globalBuff[0] = globalBuff[1] = globalBuff[2] = 0;
		idx = 3;
		
		globalBuff[idx++] = NumCount + 9;
		globalBuff[idx++] = 2;
		globalBuff[idx++] = 0xf9;
		globalBuff[idx++] = 0x45;
		globalBuff[idx++] = 0x70;
		globalBuff[idx++] = 
		globalBuff[idx++] = 
		globalBuff[idx++] = 
		globalBuff[idx++] = 
		globalBuff[idx++] = 
		globalBuff[idx++] = 
		globalBuff[idx++] = 
		globalBuff[idx++] = 0;
    globalBuff[idx++] = 0x31; 
		globalBuff[idx++] = 0x32; 
		globalBuff[idx++] = 0x34; 
		globalBuff[idx++] = 0x35; 
		globalBuff[idx++] = 0x1c;
		for (unsigned char i=0; i<NumCount; i++) 
			globalBuff[idx++] = NumStr[i];
		globalBuff[idx++] = 
		globalBuff[idx++] = 
		globalBuff[idx++] = 
		globalBuff[idx++] = 0x1c;
		
		SHA1(globalBuff, idx, sha);
		
		for (unsigned char i = 0; i<20; i++)
			globalBuff[idx++] = sha[i];
		
		for (unsigned int cnt = 0; cnt<idx; cnt++)
			pos_SendSerialData(&globalBuff[cnt]);	
	}
	else if (fromAndroid[1] == 3) {//-send to pasargad V5
		posType = 3;
		idx = 0;
		if (fromAndroid[2] < 8) {			
			INVALIDJson;
			return 1;
		}
		checkSum = fromAndroid[3] ^ fromAndroid[4] ^ fromAndroid[5] ^ fromAndroid[6];
		if (fromAndroid[7] != checkSum) {
			INVALIDJson;
			return 1;
		}
		amount = ((fromAndroid[3] << 24) + (fromAndroid[4] << 16) + 
							(fromAndroid[5] << 8) + (fromAndroid[6] & 0xff));
		NumCount = makePosNumberArray(amount, NumStr);
		globalBuff[0] = globalBuff[1] = globalBuff[2] = 0;
		idx = 3;
		globalBuff[idx++] = NumCount + 9;
		globalBuff[idx++] = 2;
		globalBuff[idx++] = 0xf9;
		globalBuff[idx++] = 0x45;
		globalBuff[idx++] = 0x70;
		globalBuff[idx++] = 
		globalBuff[idx++] = 
		globalBuff[idx++] = 
		globalBuff[idx++] = 
		globalBuff[idx++] = 
		globalBuff[idx++] = 
		globalBuff[idx++] = 
		globalBuff[idx++] = 0;
    globalBuff[idx++] = 0x31; 
		globalBuff[idx++] = 0x32; 
		globalBuff[idx++] = 0x34; 
		globalBuff[idx++] = 0x35; 
		globalBuff[idx++] = 0x1c;
		for (unsigned char i=0; i<NumCount; i++) 
			globalBuff[idx++] = NumStr[i];
		globalBuff[idx++] = 
		globalBuff[idx++] = 
		globalBuff[idx++] = 
		globalBuff[idx++] = 0x1c;
		
		SHA1(globalBuff, idx, sha);
		
		for (unsigned char i = 0; i<20; i++)
			globalBuff[idx++] = sha[i];
		
		for (unsigned int cnt = 0; cnt<idx; cnt++)
			pos_SendSerialData(&globalBuff[cnt]);	
//	send("{\"Receive\":{\"Status\":26,\"Type\":\"PosTransaction\"}}\n"); 	
	}
	else if (fromAndroid[1] == 4) {//-send to saman kish pos with hub
		posType = 4;
		idx = 0;
		if (fromAndroid[2] < 8) {
			INVALIDJson;
			return 1;
		}
		checkSum = fromAndroid[3] ^ fromAndroid[4] ^ fromAndroid[5] ^ fromAndroid[6];
		if (fromAndroid[7] != checkSum) {
			INVALIDJson;
			return 1;
		}
		amount = ((fromAndroid[3] << 24) + (fromAndroid[4] << 16) + 
							(fromAndroid[5] <<  8) + (fromAndroid[6] &  0xff));
		NumCount = makePosNumberArray(amount, NumStr); 
		globalBuff[0] = 0x02;
		globalBuff[1] = 0x75;//Len
		globalBuff[2] = 0x00;
		globalBuff[3] = 0x00;
		globalBuff[4] = 0x00;	
		globalBuff[5] = 0x01;	
		globalBuff[6] = 0x72;
		globalBuff[7] = 0x6F; //Len
		idx = 8;
		globalBuff[idx++] = 0xB1;	
		globalBuff[idx++] = 0x1A;	//Len		
		globalBuff[idx++] = 0x81;
		globalBuff[idx++] = NumCount;	//Len	
		for (unsigned char i=0; i<NumCount; i++) 
			globalBuff[idx++] = NumStr[i];
	 
		globalBuff[idx++] = 0x82;
		globalBuff[idx++] = 0x09;
		globalBuff[idx++] = 0xE4;
		globalBuff[idx++] = 0xC7;
		globalBuff[idx++] = 0xE3;
		globalBuff[idx++] = 0x3A;
		globalBuff[idx++] = 0xCD;
		globalBuff[idx++] = 0xD3;
		globalBuff[idx++] = 0xED;
		globalBuff[idx++] = 0xE4;
		globalBuff[idx++] = 0x0A; 
		globalBuff[idx++] = 0x83;
		globalBuff[idx++] = 0x01;
		globalBuff[idx++] = 0x01;
		globalBuff[idx++] = 0x84;
		globalBuff[idx++] = 0x01;
		globalBuff[idx++] = 0x03;
		globalBuff[idx++] = 0x85;
		globalBuff[idx++] = 0x01;
		globalBuff[idx++] = 0x00;
		globalBuff[idx++] = 0xB2;
		globalBuff[idx++] = 0x51;//Len
		globalBuff[idx++] = 0xA1;
		globalBuff[idx++] = 2 + NumCount + 21;	 
		globalBuff[idx++] = 0x81;
		globalBuff[idx++] = NumCount;	//Len	
		for (unsigned char i=0; i<NumCount; i++) 
			globalBuff[idx++] = NumStr[i];		
	 
		globalBuff[idx++] = 0x82;
		globalBuff[idx++] = 0x01;
		globalBuff[idx++] = 0x30;
		globalBuff[idx++] = 0x83;
		globalBuff[idx++] = 0x01;
		globalBuff[idx++] = 0x30;
		globalBuff[idx++] = 0x84;
		globalBuff[idx++] = 0x01;
		globalBuff[idx++] = 0x30;
		globalBuff[idx++] = 0x85;
		globalBuff[idx++] = 0x01;
		globalBuff[idx++] = 0x30;
		globalBuff[idx++] = 0x86;
		globalBuff[idx++] = 0x01;
		globalBuff[idx++] = 0x30;
		globalBuff[idx++] = 0x87;
		globalBuff[idx++] = 0x01;
		globalBuff[idx++] = 0x30;
		globalBuff[idx++] = 0x88;
		globalBuff[idx++] = 0x01;
		globalBuff[idx++] = 0x30;
		
		//.......................................
		globalBuff[idx++] = 0xA2;
		globalBuff[idx++] = 0x32;
		globalBuff[idx++] = 0xA1;
		globalBuff[idx++] = 0x0B;
		globalBuff[idx++] = 0x81;
		globalBuff[idx++] = 0x03;
		globalBuff[idx++] = 0xE4;
		globalBuff[idx++] = 0xC7;
		globalBuff[idx++] = 0xE3;
		globalBuff[idx++] = 0x82;
		globalBuff[idx++] = 0x04;
		globalBuff[idx++] = 0xCD;
		globalBuff[idx++] = 0xD3;
		globalBuff[idx++] = 0xED;
		globalBuff[idx++] = 0xE4;
		globalBuff[idx++] = 0xA1;
		globalBuff[idx++] = 0x11;
		globalBuff[idx++] = 0x81;
		globalBuff[idx++] = 0x06;
		globalBuff[idx++] = 0x44;
		globalBuff[idx++] = 0x6C;
		globalBuff[idx++] = 0x6C;
		globalBuff[idx++] = 0x56;
		globalBuff[idx++] = 0x65;
		globalBuff[idx++] = 0x72;
		globalBuff[idx++] = 0x82;
		globalBuff[idx++] = 0x07;
		globalBuff[idx++] = 0x32;
		globalBuff[idx++] = 0x2E;
		globalBuff[idx++] = 0x33;
		globalBuff[idx++] = 0x2E;
		globalBuff[idx++] = 0x30;
		globalBuff[idx++] = 0x2E;
		globalBuff[idx++] = 0x31;
		globalBuff[idx++] = 0xA1;
		globalBuff[idx++] = 0x10;
		globalBuff[idx++] = 0x81;
		globalBuff[idx++] = 0x06;
		globalBuff[idx++] = 0x50;
		globalBuff[idx++] = 0x72;
		globalBuff[idx++] = 0x67;
		globalBuff[idx++] = 0x56;
		globalBuff[idx++] = 0x65;
		globalBuff[idx++] = 0x72;
		globalBuff[idx++] = 0x82;
		globalBuff[idx++] = 0x06;
		globalBuff[idx++] = 0x70;
		globalBuff[idx++] = 0x72;
		globalBuff[idx++] = 0x67;
		globalBuff[idx++] = 0x76;
		globalBuff[idx++] = 0x65;
		globalBuff[idx++] = 0x72;
		globalBuff[idx++] = 0xF2;
		globalBuff[idx++] = 0x1D;
		globalBuff[idx++] = 0x1D;
		globalBuff[idx++] = 0xEB;
		
		for (unsigned int cnt = 0; cnt<idx; cnt++)
			pos_SendSerialData(&globalBuff[cnt]);
	}
	else {//--------------------------to each pos with android 
		posType = 0; 
		for (unsigned int cnt=2; cnt<=(fromAndroid[1]+1); cnt++)
			pos_SendSerialData(&fromAndroid[cnt]);
	}
	
	send("{\"Receive\":{\"Status\":26,\"Type\":\"PosTransaction\"}}\n"); 
	memset(fromAndroid,'\0', sizeof(fromAndroid));
	return 0;
}
#endif
//**********************************************************************************************

void sendWithLen(unsigned char *buff,char len) {
	for (unsigned char cnt=0;cnt<len;cnt++)	{
		SoftUartPuts(0,(unsigned char *)buff,1);	
		buff++;
		HAL_Delay(10);
	}
}
void setTimeDate(void)
{
	time.Hours=fromAndroid[1];        //H
	time.Minutes=fromAndroid[2];      //M
	time.Seconds=fromAndroid[3];      //S
	date.Date=fromAndroid[4]; //{17}  //D
	date.Month=fromAndroid[5]; //{02} //MOUNTH
	date.WeekDay=fromAndroid[6];//{03}//WEEKDAY 
	date.Year=fromAndroid[7]; //{16}  //Y
	HAL_Delay(1);
	if ( (time.Hours > 23) || (time.Minutes > 59) || (time.Seconds > 59) || (date.Date > 32) || (date.Month > 13) || (date.Year > 42) || (date.WeekDay > 8))
	{
		INVALIDJson;
		return ;
	}
	if (HAL_RTC_SetTime(&hrtc, &time, RTC_FORMAT_BIN)!= HAL_OK)
	{
		send("{\"T\":{\"Status\":128}}\n");
		return ;
	}
	HAL_Delay(1);
	if (HAL_RTC_SetDate(&hrtc, &date, RTC_FORMAT_BIN)!= HAL_OK)
	{
		return ;
		send("{\"T\":{\"Status\":129}}\n");
	}
	send("{\"Receive\":{\"Status\":26},\"T\":{\"Type\":\"SetTimeDate\"}}\n");
	#if defined LCD
	showDateTime();
	#endif
	memset(fromAndroid,'\0',sizeof(fromAndroid));
}

void getTimeDate(void)
{
	HAL_Delay(1);
	HAL_RTC_GetTime(&hrtc,&time,RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc,&date,RTC_FORMAT_BIN);
	sprintf(sendTimeDate,"{\"T\":{\"H\":%d,\"M\":%d,\"S\":%d,\"Y\":%d,\"m\":%d,\"D\":%d,\"W\":%d}}\n",time.Hours,time.Minutes,time.Seconds,date.Year,date.Month,date.Date,date.WeekDay); 
	send(sendTimeDate);
	HAL_Delay(1);	
	memset(fromAndroid,'\0',sizeof(fromAndroid));
}
void sendWithSoftUart(unsigned char *buff,char len)
{
	for (char cnt=0;cnt<len;cnt++)
	{
		SoftUartPuts(0,(unsigned char *)buff,1);	
		buff++;
		WDTR
		HAL_Delay(5);
	}
}
//**************************************************************************************tarazo
#ifdef tarazo
void WeightProccess (void)
{
	float m = 0;
	if ((uart1Buff[0] == '!') && (uart1Buff[1] == '$'))
	{
		send("{\"W\":{\"Calib\":1200g}}\n");
		HAL_Delay(5);
	}
	//-----------------------
	if ((uart1Buff[0] == '%') && (uart1Buff[1] == '+'))
	{
		send("{\"W\":{\"Calib\":3740g}}\n");
		HAL_Delay(5);
	}
	//-----------------------------
	if ((uart1Buff[0] == 'e') && (uart1Buff[1] == '1'))
	{
		send("{\"W\":{\"Error\":1}}\n");
		HAL_Delay(5);
	}
	//-----------------------------
	if ((uart1Buff[0] == 'r') && (uart1Buff[1] == '2'))
	{
		send("{\"W\":{\"Error\":2}}\n");
		HAL_Delay(5);
	}
	//------------------------------
	if ((uart1Buff[0] == 't') && (uart1Buff[1] == '3'))
	{
		send("{\"W\":{\"Error\":3}}\n");
		HAL_Delay(5);
	}
	//------------------------------	
	if ((uart1Buff[0] == 'o') && (uart1Buff[1] == 'k'))
	{
		send("{\"W\":{\"Calib\":0}}\n");
		HAL_Delay(5);
	}
  //---------------------------------	
	if ((uart1Buff[0] == 'y') && (uart1Buff[1] == 'e'))
	{
		send("{\"W\":{\"Calib\":1}}\n");
		HAL_Delay(5);
	}
  //------------------------------------------------
	if ((uart1Buff[0] == '[') && (uart1Buff[5] == ']'))
	{  
		weight = ((uart1Buff[1] << 24)) + ((uart1Buff[2] << 16)) + ((uart1Buff[3] << 8)) + (uart1Buff[4]&0xff); 
		HAL_Delay(10);
		sprintf(globalBuff,"{\"W\":{\"IntTemp\":%d}}\n",weight);
		send(globalBuff);
		memset(globalBuff,'\0',sizeof(globalBuff));
	}	
	//---------------------------------------------------
	if ((uart1Buff[0] == '{') && (uart1Buff[5] == '}'))
	{  
		weight = ((uart1Buff[1] << 24)) + ((uart1Buff[2] << 16)) + ((uart1Buff[3] << 8)) + (uart1Buff[4]&0xff); 
		HAL_Delay(10);
		sprintf(globalBuff,"{\"W\":{\"B\":%d}}\n",weight);
		send(globalBuff);
		memset(globalBuff,'\0',sizeof(globalBuff));
	}
  //----------------------------------------------------
	if ((uart1Buff[0] == '(') && (uart1Buff[5] == ')'))
	{  
		weight = ((uart1Buff[1] << 24)) + ((uart1Buff[2] << 16)) + ((uart1Buff[3] << 8)) + (uart1Buff[4]&0xff); 
		HAL_Delay(10);
		sprintf(globalBuff,"{\"W\":{\"LCT\":%d}}\n",weight);
		send(globalBuff);
		memset(globalBuff,'\0',sizeof(globalBuff));
	}
	//------------------------------------------------------
	if (uart1Buff[0] == ':') 
	{  
		sscanf(uart1Buff,":%f:",&m);
		HAL_Delay(10);
		sprintf(globalBuff,"{\"W\":{\"m\":%f}}\n",m);
		send(globalBuff);
		memset(globalBuff,'\0',sizeof(globalBuff));
	}	
	//--------------------------------------------
//	if ((uart1Buff[0] == '@') && (uart1Buff[5] == '#'))
//	{  
//		weight = ((uart1Buff[1] << 24)) + ((uart1Buff[2] << 16)) + ((uart1Buff[3] << 8)) + (uart1Buff[4]&0xff); 
//		sprintf(globalBuff,"{\"W\":{\"CharTemp\":%c%c%c%c}}\n",uart1Buff[1],uart1Buff[2],uart1Buff[3],uart1Buff[4]);
//		CDC_Transmit_FS(globalBuff,25);
//		HAL_Delay(10);
//		sprintf(globalBuff,"{\"W\":{\"IntTemp\":%d}}\n",weight);
//		CDC_Transmit_FS(globalBuff,28);  
//		memset(globalBuff,'\0',sizeof(globalBuff));
//	}
   memset(uart1Buff,'\0',sizeof(uart1Buff));	
}
#endif
//*************************************************************************************DS7
#ifdef DS7

void ds7Proccess(void)
{
	send("{\"Receive\":{\"Status\":26}}\n");
	sendWithSoftUart(fromAndroid,8);
	memset(fromAndroid,'\0',sizeof(fromAndroid));
}
#endif
//**************************************************************************************defined ReadWrite
#if defined ReadWrite
//void DeviceEnteringExitingMode(unsigned char mode)
//{
//	if (mode > 2) send("{\"Receive\":{\"Status\":19},\"C\":{\"Type\":\"DeviceMode\"}}\n");
//	else
//	{
//		EnteringExitingMode = mode;
//		send("{\"Receive\":{\"Status\":26},\"C\":{\"Type\":\"DeviceMode\"}}\n");
//	}
//	memset(fromAndroid,'\0',sizeof(fromAndroid));
//}
//==============================================================
unsigned char restoreRGroupParking(unsigned char *snr,unsigned char *bufRestore)
{
	unsigned short Read_Write_First_Arg_Restore=21;
	unsigned char Source_Buffer[16];
	unsigned char Readed_BCC = 0;
	unsigned int tick = HAL_GetTick();
	while (1)
	{
		if (HAL_GetTick() - tick > 500 ) {return 255;}
		if (!ISO14443_ReadBlock(Read_Write_First_Arg_Restore, Source_Buffer))
		{
			for (unsigned char index=0; index<15; index++){Readed_BCC^=Source_Buffer[index];}
			if (Readed_BCC == Source_Buffer[15])
			{
				if (Source_Buffer[14]>10)
				{
					return 255; 
				}
				if (Source_Buffer[7]>24 || Source_Buffer[8]>59 || Source_Buffer[9]>59 ||  Source_Buffer[6] >31 ||   Source_Buffer[5] > 12 || Source_Buffer[4]> 32)
				{
					return 255; 
				}	
				for (unsigned char index=0; index<16; index++){bufRestore[index]=Source_Buffer[index];}
				return 0;
			}
			else 
				return 255;
		}
		else continue;
	} //while (1)
}
//==============================================================ChangeGroup
unsigned char rwGroupParking(unsigned char rw,unsigned gp,unsigned char *snr)
{
  unsigned short Login_First_Arg = 5,Login_Second_Arg = 0x37,Read_Write_First_Arg=22,Login_Successfully=1;
  unsigned int tick;
  volatile unsigned char Source_Buffer[16];
	unsigned char Readed_BCC=0,Read_Write_First_Arg_Restore = 21;
	#define PARKING_EXIT_MODE  1
	#define PARKING_ENTER_MODE 0
	unsigned int User_Group;
	unsigned char GroupOfCard=0;
	unsigned char Group_Secure,index=0;
  unsigned char once = 1;	
	//unsigned char buff[200];
	unsigned char buff[20];
	unsigned short idx = 0;
	switch (rw)
	{
	case 0://******************************************
	  tick = HAL_GetTick();
	  while (Login_Successfully)
	  {
									if (HAL_GetTick() - tick > 5000 ) {send("{\"C\":{\"Status\":1}}\n");  return 255;}
									WDTR
									MFRC522_Init();
									if (ISO14443_SingleTagSelect(snr))
									{
															if (ISO14443_SingleTagSelect(snr))
																continue;
									}
									if (ISO14443_LoginE2(Login_First_Arg,Login_Second_Arg))
									{
															if (ISO14443_SingleTagSelect(snr))
															{
																if (ISO14443_SingleTagSelect(snr))
																	continue;
															} 
															if (ISO14443_LoginE2(Login_First_Arg,Login_Second_Arg)) continue;
									}			
									if (!ISO14443_ReadBlock(Read_Write_First_Arg, Source_Buffer))
									{//****************
																tick = HAL_GetTick();
																while (1)
																{//*************
																						if (HAL_GetTick() - tick > 5000 ) {send("{\"C\":{\"Status\":1}}\n");  return 255;}
																						WDTR;
																						Readed_BCC = 0;
																						for (index=0; index<15; index++){Readed_BCC^=Source_Buffer[index];}
																						if (Readed_BCC == Source_Buffer[15])
																						{
																							User_Group   = Source_Buffer[1];  User_Group<<=8;  // ??????? ?????
																							User_Group  += Source_Buffer[0];
																							GroupOfCard  = Source_Buffer[14];                   // ??????? ????????
																							Group_Secure = Source_Buffer[14];						
																							if (Source_Buffer[14]>10)
																							{
																											if (restoreRGroupParking(snr,Source_Buffer))
																											{
																												send("{\"C\":{\"Status\":8}}\n");
																												return 255; 
																											}
																											else 
																												continue;								
																							}
																							if (Source_Buffer[7]>24 || Source_Buffer[8]>59 || Source_Buffer[9]>59 ||  Source_Buffer[6] >31 ||   Source_Buffer[5] > 12 || Source_Buffer[4]> 32)
																							{
																											if (restoreRGroupParking(snr,Source_Buffer))
																											{
																												send("{\"C\":{\"Status\":6}}\n");
																												return 255; 
																											}
																											else
																												continue;
																							}					
																							if (!((Source_Buffer[10] == PARKING_EXIT_MODE) ||  (Source_Buffer[10] == PARKING_ENTER_MODE)))
																							{
																											//Source_Buffer[10] = PARKING_ENTER_MODE; 	
																											if (restoreRGroupParking(snr,Source_Buffer))
																											{
																												send("{\"C\":{\"Status\":7}}\n");
																												return 255; 
																											}
																											else
																											{
																												once = 1;
																												continue;
																											}
																							}
																						//****************
																							if (once)
																							{
																											once = 0;
																											if (EnteringExitingModeMember == 0x20)
																											{								
																												if (Source_Buffer[10] == PARKING_EXIT_MODE) 
																												{
																													ParkTime.Entering_Exiting_Mode = PARKING_EXIT_MODE;
																													Source_Buffer[10] = PARKING_ENTER_MODE; 	
																												}	
																												else
																												{
																													ParkTime.Entering_Exiting_Mode = PARKING_ENTER_MODE;
																													Source_Buffer[10] = PARKING_EXIT_MODE; 
																												}
																											}
																							}
																						
																							if (EnteringExitingModeMember == 0x00) 
																							{
																												if ( Source_Buffer[10] == PARKING_ENTER_MODE)
																												{
																													sprintf(buff,"{\"C\":{\"ID\":%d,\"UC\":%d,\"Status\":3}}\n",cardId,uc,etebar);	
																													send(buff); 
																													return 0;
																												}
																												Source_Buffer[10] = PARKING_ENTER_MODE;
																												ParkTime.Entering_Exiting_Mode = PARKING_ENTER_MODE;
																							}
																						
																							if (EnteringExitingModeMember == 0x10) 
																							{
																												if (Source_Buffer[10] == PARKING_EXIT_MODE)
																												{
																													sprintf(buff,"{\"C\":{\"ID\":%d,\"UC\":%d,\"Status\":3}}\n",cardId,uc,etebar);	
																													send(buff); 
																													return 0;
																												}
																												Source_Buffer[10] = PARKING_EXIT_MODE;
																												ParkTime.Entering_Exiting_Mode = PARKING_EXIT_MODE;
																							}
																						
																							Readed_BCC=0;					
																							ParkTime.year  = Source_Buffer[4];
																							ParkTime.month = Source_Buffer[5];
																							ParkTime.day   = Source_Buffer[6];
																							ParkTime.hour  = Source_Buffer[7];
																							ParkTime.min   = Source_Buffer[8];
																							ParkTime.sec   = Source_Buffer[9];
																						
																							HAL_RTC_GetTime(&hrtc,&time,RTC_FORMAT_BIN);
																							HAL_RTC_GetDate(&hrtc,&date,RTC_FORMAT_BIN);	
																							Source_Buffer[4]=date.Year;
																							Source_Buffer[5]=date.Month;
																							Source_Buffer[6]=date.Date;
																							Source_Buffer[7]=time.Hours;
																							Source_Buffer[8]=time.Minutes;
																				
																							Source_Buffer[9] = time.Seconds;     
																							Source_Buffer[11] = 0;
																							Source_Buffer[12] = 0;
																							Source_Buffer[13] = Source_Buffer[10];
																							Source_Buffer[14] = GroupOfCard;   
																							Readed_BCC=0;
																							
																							for (index=0; index<15; index++){Readed_BCC^=Source_Buffer[index];}
																							Source_Buffer[15]=Readed_BCC;
																							
																							if (!ISO14443_LoginE2(Login_First_Arg,Login_Second_Arg))
																							{
																										if (!ISO14443_WriteBlock(Read_Write_First_Arg, Source_Buffer))
																										{
																														if (!ISO14443_WriteBlock(Read_Write_First_Arg_Restore, Source_Buffer))
																														{
																																				HAL_Delay(1);
																																				sprintf(buffTransaction,"{\"C\":{\"ID\":%d,\"UC\":%d,\"Gp\":%d,\"Y\":%d,\"M\":%d,\"D\":%d,\"H\":%d,\"m\":%d,\"S\":%d,\"EEM\":%d}}\n",cardId,uc,GroupOfCard,ParkTime.year,ParkTime.month,ParkTime.day,ParkTime.hour,ParkTime.min,ParkTime.sec,ParkTime.Entering_Exiting_Mode); // ID card , ersal UC card
																																				send(buffTransaction);  //ersal ID card
																																				HAL_Delay(50);
																																				buff[idx++] = deviceType;
																																				buff[idx++] = cardId & 0xFF;
																																				buff[idx++] = ( cardId >> 8) & 0xFF;
																																				buff[idx++] = ( cardId >> 16) & 0xFF;
																																				buff[idx++] = ( cardId >> 24) & 0xFF;
																																				buff[idx++] =  uc & 0xFF;
																																				buff[idx++] = ( uc >> 8) & 0xFF;
																																				buff[idx++] = ( uc >> 16) & 0xFF;
																																				buff[idx++] = ( uc >> 24) & 0xFF;
																																				buff[idx++] = GroupOfCard;
																																				buff[idx++] = ParkTime.year;
																																				buff[idx++] = ParkTime.month;
																																				buff[idx++] = ParkTime.day;
																																				buff[idx++] = ParkTime.hour;
																																				buff[idx++] = ParkTime.min;
																																				buff[idx++] = ParkTime.sec;
																																				buff[idx++] = ParkTime.Entering_Exiting_Mode;
																																				if (	memID == 0XEF16)
																																				{
																																					SaveTransactionRecord(buff,idx);
																																					SaveRingDetail();
																																				}
																																				HAL_Delay(10);
																																				return 0;
																														}
																														else continue;
																										}
																										else {continue;} //send("{\"C\":{\"Status\":2}}\n"); return 255;}						
																							}
																							else {continue;}//send("{\"C\":{\"Status\":3}}\n"); return 255;}				
																						}
																						else 
																						{
																							if (restoreRGroupParking(snr,Source_Buffer))
																							{
																								send("{\"C\":{\"Status\":4}}\n"); 
																								return 255;
																							}
																							else
																								continue;
																						}
																}						
									}
									else {continue;}//send("{\"C\":{\"Status\":5}}\n");return 255;}
		}
//	}
	
//	else if (rw==1) //write 
//	{
	case 1:	//**************************************
	  send("{\"Receive\":{\"Status\":26},\"C\":{\"Type\":\"ChangeGroup\"}}\n");//put card 
	  tick = HAL_GetTick();
	  while (Login_Successfully)
	  {
												WDTR;
												if (HAL_GetTick() - tick > 5000 ) {send("{\"C\":{\"Status\":1}}\n");  return 255;}
												Readed_BCC=0;																																				
												WDTR
												
									MFRC522_Init();
									if (ISO14443_SingleTagSelect(snr))
									{
															if (ISO14443_SingleTagSelect(snr))
																continue;
									}
									if (ISO14443_LoginE2(Login_First_Arg,Login_Second_Arg))
									{
															if (ISO14443_SingleTagSelect(snr))
															{
																if (ISO14443_SingleTagSelect(snr))
																	continue;
															} 
															if (ISO14443_LoginE2(Login_First_Arg,Login_Second_Arg)) continue;
									}		
						
						if (!ISO14443_ReadBlock(Read_Write_First_Arg, Source_Buffer)){
//							sprintf(buffTransaction,"{\"C\":{\"ID\":%d,\"UC\":%d,\"Gp\":%d,\"Y\":%d,\"M\":%d,\"D\":%d,\"H\":%d,\"m\":%d,\"S\":%d,\"EEM\":%d}}\n",cardId,uc,GroupOfCard,ParkTime.year,ParkTime.month,ParkTime.day,ParkTime.hour,ParkTime.min,ParkTime.sec,ParkTime.Entering_Exiting_Mode); // ID card , ersal UC card
//							send(buffTransaction);  //ersal ID card
												Source_Buffer[2]=0;//DeviceID%256; 
												Source_Buffer[3]=0;//DeviceID/256;
												Source_Buffer[0] = Source_Buffer[1] = 0;	
												
												HAL_RTC_GetTime(&hrtc,&time,RTC_FORMAT_BIN);
												HAL_RTC_GetDate(&hrtc,&date,RTC_FORMAT_BIN);	
												Source_Buffer[4]= date.Year;
												Source_Buffer[5]= date.Month;
												Source_Buffer[6]= date.Date;
												Source_Buffer[7]= time.Hours;
												Source_Buffer[8] = time.Minutes;
												Source_Buffer[9] = time.Seconds;
												Source_Buffer[9] = 0;//(FirstNumber[0]-'0')<<4 | (FirstNumber[1]-'0')&0x0F);         
												Source_Buffer[10] = PARKING_ENTER_MODE ; 
												Source_Buffer[11]= 0;//(SecondNumber[0]-'0')<<4 | ((SecondNumber[1]-'0')&0x0F);  
												Source_Buffer[12]= 0;//SecondNumber[2];
												Source_Buffer[13]= 0;//(ThirdNumber_Region[0]-'0')<<4 | ((ThirdNumber_Region[1]-'0')&0x0F);          
												Source_Buffer[14]= gp;   
												
												Readed_BCC = 0;																			
																				for (index=0; index<15; index++)  {Readed_BCC^=Source_Buffer[index];}
											                 	Source_Buffer[15]=Readed_BCC;

												
												#if defined UHF
												if (!WriteBufferUHf(Source_Buffer,1,1))
													return 0;
												#endif
												
												if (ISO14443_SingleTagSelect(snr))
												{
																		if (ISO14443_SingleTagSelect(snr))
																			continue;
												}
												
												if (ISO14443_LoginE2(Login_First_Arg,Login_Second_Arg))
												{
																	if (ISO14443_SingleTagSelect(snr))
																	{
																		if (ISO14443_SingleTagSelect(snr))
																			continue;
																	} 
																	if (ISO14443_LoginE2(Login_First_Arg,Login_Second_Arg)) continue;
												}
												
												if (!ISO14443_LoginE2(Login_First_Arg,Login_Second_Arg))
												{
													

//													sprintf(buff,"{\"ProcessCardOff\":{\"ID\":%d,\"UC\":%d,\"Gp\":%d,\"Y\":%d,\"M\":%d,\"D\":%d,\"H\":%d,\"m\":%d,\"S\":%d,\"EEM\":%d}}\n",cardId,uc,Source_Buffer[14],Source_Buffer[4],Source_Buffer[5],Source_Buffer[6],Source_Buffer[7],Source_Buffer[8],Source_Buffer[9],Source_Buffer[10]); // ID card , ersal UC card
//																send(buff);  //ersal ID card
//																		if (Source_Buffer[10] == PARKING_ENTER_MODE)//*************************if
//																		{

																												if (!ISO14443_WriteBlock(Read_Write_First_Arg, Source_Buffer))//*****************Read_Write_First_Arg
																												{
																																if (!ISO14443_WriteBlock(Read_Write_First_Arg_Restore, Source_Buffer))
																																{
																																							send("{\"C\":{\"Status\":0,\"Type\":\"ChangeGroup\"}}\n");
																																							return 0;
																																}
																																else
																																continue;
																												}
																									      else {continue;}		
//																		}	
//														    	else {send("{\"C\":{\"Status\":101}}\n");return 0;}			
												}
												else {continue;}//send("{\"C\":{\"Status\":3}}\n");}	
						}//if****
		}
	}
}
//****************************************	
char infoCardForParking(void)
{
	unsigned char snr[5]={0};
	volatile unsigned char Source_Buffer[16];
	unsigned int tick = HAL_GetTick();
	unsigned char buff[200]={0};
	unsigned char Readed_BCC=0,index=0;
	while (1)
	{
								if (HAL_GetTick() - tick > 5000 ) {send("{\"ProcessCardOff\":{\"Status\":1}}\n");  return 255;}
								WDTR
								if (ISO14443_SingleTagSelect(snr))
								{
									if (ISO14443_SingleTagSelect(snr))
										continue;
								}
												if (ISO14443_LoginE2(5,0x37))
												{
															if (ISO14443_SingleTagSelect(snr))
															{
																if (ISO14443_SingleTagSelect(snr))
																	continue;
															} 
													if (ISO14443_LoginE2(5,0x37)) continue;
												}
												
								if (!ISO14443_ReadBlock(22, Source_Buffer))
								{
									tick = HAL_GetTick();
									while (2)
									{
										if (HAL_GetTick() - tick > 5000 ) {send("{\"ProcessCardOff\":{\"Status\":1}}\n");  return 255;}				
										WDTR;
										Readed_BCC = 0;
										for (index=0; index<15; index++){Readed_BCC^=Source_Buffer[index];}
										if (Readed_BCC == Source_Buffer[15])
										{
																if (Source_Buffer[14]>10)
																{
																						if (restoreRGroupParking(snr,Source_Buffer))
																						{
																							send("{\"ProcessCardOff\":{\"Status\":8}}\n");
																							return 255; 
																						}
																						else 
																							continue;								
																}
																if (Source_Buffer[7]>24 || Source_Buffer[8]>59 || Source_Buffer[9]>59 ||  Source_Buffer[6] >31 ||   Source_Buffer[5] > 12 || Source_Buffer[4]> 32)
																{
																						if (restoreRGroupParking(snr,Source_Buffer))
																						{
																							send("{\"ProcessCardOff\":{\"Status\":6}}\n");
																							return 255; 
																						}
																						else
																							continue;
																}
																if (!((Source_Buffer[10] == PARKING_EXIT_MODE) ||  (Source_Buffer[10] == PARKING_ENTER_MODE)))
																{ 	
																						if (restoreRGroupParking(snr,Source_Buffer))
																						{
																							send("{\"ProcessCardOff\":{\"Status\":7}}\n");
																							return 255; 
																						}
																						else
																							continue;
																}
																
sprintf(buff,"{\"ProcessCardOff\":{\"ID\":%d,\"UC\":%d,\"Gp\":%d,\"Y\":%d,\"M\":%d,\"D\":%d,\"H\":%d,\"m\":%d,\"S\":%d,\"EEM\":%d}}\n",cardId,uc,Source_Buffer[14],Source_Buffer[4],Source_Buffer[5],Source_Buffer[6],Source_Buffer[7],Source_Buffer[8],Source_Buffer[9],Source_Buffer[10]); // ID card , ersal UC card
																send(buff);  //ersal ID card
																return 0;											
										}
										else
										{
																if (restoreRGroupParking(snr,Source_Buffer))
																{
																	send("{\"ProcessCardOff\":{\"Status\":4}}\n"); 
																	return 255;
																}
																else
																	continue;
										}
									}//while (2)				
								}
								else
									continue;
	}//while (1)
}

//==============================================================
#endif

//**************************************************************************************
void Sendpacket(unsigned char PacketCode, unsigned int PacketNo)
{ // 30 ,0
  unsigned char pack[7];
	unsigned char packToUsb[50];
	pack[0] = 0x02;
	pack[1] = PacketCode;
	pack[2] = 0x05;
	pack[3] = PacketNo%256;
	pack[4] = PacketNo/256;
	pack[5] = PacketCode^5^(PacketNo%256);
	pack[6] = 0x03;
	//HAL_Delay(1);
	sprintf(packToUsb,"{\"PackNo\":{\"Idx\":%u}}\n",PacketNo);
	//CDC_Transmit_FS((unsigned char*)pack,7);
	//HAL_Delay(1);
	send(packToUsb);
	HAL_Delay(10);	
}
unsigned char GetPage(unsigned int Packetno) {
	unsigned long int i;
	unsigned char BCC=0;
	unsigned int tickReadWrite = HAL_GetTick();
/*	while (rxBufferHeadPos<=0)
	{
		WDTR
		
		if ((HAL_GetTick() - tickReadWrite) > 5000) //5s
			return 7;
		HAL_Delay(1);
	}
*/	
	HAL_Delay(5);		
	if ((fromAndroid[0] == 2) && (fromAndroid[1] == 0x54) && (fromAndroid[2] == 0x48) && (fromAndroid[3] == 0x45))
	{
		//cnt=1;
		Sendpacket(30, 0);
		return(100);
	}
	
	if (rxBufferHeadPos!=8) 
		if ((fromAndroid[0]!=2) || (fromAndroid[rxBufferHeadPos-1]!=3))
			return(3);
  BCC=0;
  for (i=1; i<rxBufferHeadPos-2; i++)
    BCC^=fromAndroid[i];
  if (BCC!=fromAndroid[rxBufferHeadPos-2])
    return(4);

  if (Packetno!=(fromAndroid[3]+(fromAndroid[4]*256)))
    return(6);
  
  if (rxBufferHeadPos==7)
  {
    Sendpacket(30, 0);
    return(100);
  }
  return(0);
}
void mainUpdateProccess(void) {
	/*
	(status) {
		0 	 -> HasMemory
		1 	 -> CrcError
		2 	 -> WriteError
		26 	 -> Successful
		230  -> WaitForUpdate
		90   -> NoMemory
		85   -> EndOfUpdate
		else -> null
	}
	*/
	#define ERRORWRITEMEM send("{\"UpdateFirmware\":{\"Status\":2}}\n");
	#define ERRORCRC      send("{\"UpdateFirmware\":{\"Status\":1}}\n");
	#define WRITEMEMOK    send("{\"UpdateFirmware\":{\"Status\":85}\n");
	unsigned short 
		Res = 1, 
		idx = 1;

	unsigned int 
		tickUp = 0,
		page;
	unsigned char 
		readMem[264],
		readMemm[264],
		tryUpdate = 0;
	
	rxBufferHeadPos = 0;	
	HAL_Delay(1);		
	page = 0;
  while (Res != 100) {
		WDTR
		rxBufferHeadPos = 0;
		rxBufferTailPos = 0;
		CDCReceiveLen   = 0;
    Sendpacket(30, idx);
		tickUp = HAL_GetTick();
		while (!rxBufferHeadPos) {//while ((idx == 1) || (idx == 2))
			WDTR
//			if (rxBufferHeadPos != 0)
//				break;
			if ((HAL_GetTick() - tickUp) > 12000)
				break;			
		}
		HAL_Delay(10);	
    if (!(Res = GetPage(idx)))     {
			if (SPI_Flash_Write(fromAndroid,page, 263))
				if (SPI_Flash_Write(fromAndroid,page, 263)) {
					ERRORWRITEMEM
					return;
				}
			//SPI_Flash_Read(readMem,page,263);
			idx++;
			page 		 += 263;	
			tryUpdate = 0;				
    }
		else {
			if (tryUpdate > 5) {
				ERRORCRC
				return;
			}
			rxBufferHeadPos = 0;
			memset(fromAndroid, 0, 263);
			tryUpdate++;			
			continue;
    }
  }
  if (Res == 100) {
		SPI_Flash_Read(readMem,  page -(263 * 2), 263);
		SPI_Flash_Read(readMem,  page -263, 263);
		SPI_Flash_Read(readMemm, page, 263);
		SPI_Flash_Write("ASDFGHJKL", page, 16);
		HAL_Delay(10);		
		SPI_Flash_Write("ASDFGHJKL",0x7fff0,16);
		//SPI_Flash_Read(readMem,page,263);
		//Sendpacket(30,100);
		HAL_Delay(10);
		WRITEMEMOK
  }		
}

void updateFirmware(void) { 
	#define UNAVAILABLEMEM send("{\"UpdateFirmware\":{\"Status\":90}}\n");
	#define AVAILABLEMEM   send("{\"UpdateFirmware\":{\"Status\":0}}\n");

	if ((fromAndroid[0] == 0x02) && 
			(fromAndroid[1] == 0xc7) && 
			(fromAndroid[2] == 0xff)) {
		if (SPI_Flash_ReadID() == 0xEF16) {
			AVAILABLEMEM
			mainUpdateProccess();
			return;
		}
		else
			UNAVAILABLEMEM
	}
	else if (fromAndroid[0] == 'u') {
		#if (deviceType==BD) || (deviceType==BCU)	|| (deviceType==AutoPark)
			_EE_SIZE = 64;
		 _EE_ADDR_INUSE   = 0x0801fc00;
		 _EE_FLASH_BANK   = FLASH_BANK_1;
		 _EE_PAGE_OR_SECTOR = 0;
		#endif
		
		//if (SPI_Flash_ReadID() ==  0xEF16)
		//{
		if (SPI_Flash_Write("zxcvbnm", 0x7fff0, 10)) {
			if (!(SPI_Flash_Write("zxcvbnm", 0x7fff0, 10)))
		//	send("{\"UpdateFirmware\":{\"Status\":19}}\n");
		//else
			send("{\"UpdateFirmware\":{\"Status\":26}}\n");
		//return;
		}
		else
			send("{\"UpdateFirmware\":{\"Status\":26}}\n");
		//}
		
		if (ee_init()) {
			HAL_Delay(100); 
			ee_format(0); 
			if (ee_write(0, 8, "BooT1234")) {
				HAL_Delay(5);
				send("{\"UpdateFirmware\":{\"Status\":26}\n");
				HAL_Delay(5); 			
			}
			else 
				send("{\"UpdateFirmware\":{\"Status\":19}\n");
		}	
		else 
			send("{\"UpdateFirmware\":{\"Status\":19}\n");
	}
}

void configBaudRate(void) {//0x42
	unsigned char buf[50]={0};
	unsigned char ins[] = "USART0";
	unsigned int baudRate = ( fromAndroid[2] << 16) + ( fromAndroid[3] << 8) + ( fromAndroid[4] & 0xff) ;
	uart_no = fromAndroid[1];
	
	if (baudRate == 0) {
		INVALIDJson
		return;
	}
	if (fromAndroid[1] == 1) {
		#ifndef atlas
		ins[5] = '1'; 
		huart1.Instance = USART1;	
		huart1.Init.BaudRate = 5;
		huart1.Init.WordLength = UART_WORDLENGTH_8B;
		huart1.Init.StopBits = UART_STOPBITS_1;
		huart1.Init.Parity = UART_PARITY_NONE;
		huart1.Init.Mode = UART_MODE_TX_RX;
		huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
		huart1.Init.OverSampling = UART_OVERSAMPLING_16;
		if (HAL_UART_Init(&huart1) != HAL_OK)
			Error_Handler();
		__HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
		#else
		send("{\"CBR\":{\"Status\":1}}\n");
		return;
		#endif
	}
	else if (fromAndroid[1] == 2) {
		ins[5] = '2';
		huart2.Instance = USART2;	
		huart2.Init.BaudRate = baudRate;
		huart2.Init.WordLength = UART_WORDLENGTH_8B;
		huart2.Init.StopBits = UART_STOPBITS_1;
		huart2.Init.Parity = UART_PARITY_NONE;
		huart2.Init.Mode = UART_MODE_TX_RX;
		huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
		huart2.Init.OverSampling = UART_OVERSAMPLING_16;
		if (HAL_UART_Init(&huart2) != HAL_OK)
			Error_Handler();
		__HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);
	}
	else if (fromAndroid[1] == 3) {
		ins[5] = '3';
		huart3.Instance 				 = USART3;	
		huart3.Init.BaudRate 		 = baudRate; 
		huart3.Init.WordLength 	 = UART_WORDLENGTH_8B;
		huart3.Init.StopBits 		 = UART_STOPBITS_1;
		huart3.Init.Parity 			 = UART_PARITY_NONE;
		huart3.Init.Mode 				 = UART_MODE_TX_RX;
		huart3.Init.HwFlowCtl 	 = UART_HWCONTROL_NONE;
		huart3.Init.OverSampling = UART_OVERSAMPLING_16;
		if (HAL_UART_Init(&huart3) != HAL_OK)
			Error_Handler();
		__HAL_UART_ENABLE_IT(&huart3, UART_IT_RXNE);
	}
	else {
		INVALIDJson
		return;
	}		
	sprintf(buf,"{\"CBR\":{\"Status\":0,\"Ins\":%s,\"BR\":%u}}\n",ins,baudRate);
	send(buf);
	memset(fromAndroid,0,10);
}
//***********************************************************************************
#ifdef Printer
void Printer_SendSerialData(unsigned char *buff) 
{
	 HAL_UART_Transmit(&huart3,(unsigned char*)buff ,1,1);
	return;
}
char PrinterProcess(void)
{
	unsigned int cnt=1,tick=HAL_GetTick();
	unsigned char pk[30];
	
	HAL_Delay(1);
	CDC_FlushRxBuffer_FS();
	send("{\"P\":{\"SendPacket\":0}}\n");
	HAL_Delay(1);
	
	while (1)
	{
		WDTR
		if ((HAL_GetTick() - tick) > 4000)  //4s 
		{
			send("{\"P\":{\"Status\":1}}\n");
			return 1;
		}
		if (rxBufferHeadPos)
		{
			tick=HAL_GetTick();
			HAL_Delay(5);
			if ((fromAndroid[0] == 't') && (fromAndroid[1] == 'a') && (fromAndroid[2] == 'm') && (fromAndroid[3] == 'a') && (fromAndroid[4] == 'm'))
			{
				send("{\"Receive\":{\"Status\":26}\n");
				CDC_FlushRxBuffer_FS();
				return 26;
			}
			for (unsigned short cnt=0; cnt< rxBufferHeadPos;cnt++)
				Printer_SendSerialData(&fromAndroid[cnt]);
			WDTR
			HAL_Delay(5);
			sprintf(pk,"{\"P\":{\"SendPacket\":%d}}\n",cnt++);
			send(pk);
			CDC_FlushRxBuffer_FS();
		}				
	}	
}
#endif

//********************************************************************************
#ifdef Max30102
void max30102Init(void) {
  max30102_init(&max30102, &hi2c1);
  max30102_reset(&max30102);
  max30102_clear_fifo(&max30102);
  max30102_set_fifo_config(&max30102, max30102_smp_ave_8, 1, 7);
  
  // Sensor settings
  max30102_set_led_pulse_width(&max30102, max30102_pw_16_bit);
  max30102_set_adc_resolution(&max30102, max30102_adc_2048);
  max30102_set_sampling_rate(&max30102, max30102_sr_3200);
  max30102_set_led_current_1(&max30102, 6.2);
  max30102_set_led_current_2(&max30102, 6.2);

  // Enter SpO2 mode
  max30102_set_mode(&max30102, max30102_spo2);
  max30102_set_a_full(&max30102, 1);
}

void testConnectionMax30102(void) {
	HAL_Delay(1);
	HAL_I2C_Mem_Read(&hi2c1, 0xae, 0xff, 1,
                       &ID, 1, 1);
	max30102Init();
	if (0x15 == ID) {
		//Max30102_Init(&hi2c1);
		
		send("{\"M\":{\"Status\":0}}\n");
		max30102Flag = 1;
	}
	else {
		send("{\"M\":{\"Status\":130}}\n");
		max30102Flag = 0;
	}
	HAL_Delay(1);
	ID = 0;
	memset(fromAndroid,'\0',sizeof(fromAndroid));
}
#endif

//*********************************************************************************
#ifdef heightHum
void heighHumProccess(void)
{
	/*
	if ((uart3buf[0] == 'h') && (uart3buf[2] == 't'))
	{
		sprintf(globalBuff,"{\"H\":{\"HeightHuman\":%d}}\n",(uart3buf[1]&0xff));
		send(globalBuff);
	}
	HAL_Delay(3);
	uart3Len = 0;	
	memset(globalBuff,'\0',sizeof(globalBuff));
	*/
	unsigned char heightBuf[10];
	SoftUartReadRxBuffer(0,(unsigned char *)heightBuf,(unsigned int)sizeof(heightBuf));
	if ((heightBuf[0] == 'h') && (heightBuf[2] == 't'))
	{
		sprintf(globalBuff,"{\"H\":{\"HeightHuman\":%d}}\n",(heightBuf[1]&0xff));
		send(globalBuff);
	}
	memset(heightBuf,0,sizeof(heightBuf));
	SoftUartInit(0,GPIOC,GPIO_PIN_13,GPIOB,GPIO_PIN_3);
  SoftUartEnableRx(0);
}
#endif

//*********************************************************************************
#ifdef CoordinatedOldBCU
void SendConfig(void)
{
unsigned int Idx, crc=0, i; 
unsigned char GlobalBuffer[100];
	
//unsigned char GrouhPrice[10][1]={{40},{200},{300},{400},{500},{70},{60},{50},{80},{90}};
	//pk :      4 	DeviceIDDeviceID 	 UC		OperatorID	  Y		M		D		H		m   s  price
	//example : 04       0079         18c9   12131415     01  03  0a  0f  0e 00   0514  Hex 
 //			    	04  		  121					6345   303240213		1    3  10  15  14  0   1300  Dec
	
	GlobalBuffer[0]=STX;
	GlobalBuffer[1]=4; //Connection Established
	GlobalBuffer[2]=fromAndroid[2];//456%256; //DeviceID
	GlobalBuffer[3]=fromAndroid[1];//456/256; //DeviceID
	GlobalBuffer[4]=0; //Len;
	GlobalBuffer[5]=0; //Len;
	Idx=6;
	GlobalBuffer[Idx++]=fromAndroid[4];//%256;   //UC
	GlobalBuffer[Idx++]=fromAndroid[3];//20108/256;   //UC
	
	GlobalBuffer[Idx++]=fromAndroid[8]; //OperatorID
	GlobalBuffer[Idx++]=fromAndroid[7]; //OperatorID 
	GlobalBuffer[Idx++]=fromAndroid[6]; //OperatorID
	GlobalBuffer[Idx++]=fromAndroid[5]; //OperatorID
	//IntToByte(1234,GlobalBuffer+Idx); Idx+=4; //OperatorID

	GlobalBuffer[Idx++]=fromAndroid[9]+100;   //Y
	GlobalBuffer[Idx++]=fromAndroid[10];       //M  
	GlobalBuffer[Idx++]=fromAndroid[11];                   //D
	GlobalBuffer[Idx++]=fromAndroid[12];                   //H
	GlobalBuffer[Idx++]=fromAndroid[13];                    //m
	GlobalBuffer[Idx++]=fromAndroid[14];                    //s
															
	GlobalBuffer[Idx++]=fromAndroid[16];//%256; //price 1200 ---> 04b0
	GlobalBuffer[Idx++]=fromAndroid[15];//256;  //price 
	DoorPrice = fromAndroid[16] + (fromAndroid[15] * 256);														
//      for (i=0; i<9; i++)
//      {        
//          GlobalBuffer[Idx++]=0;
//          GlobalBuffer[Idx++]=0;
////				  GlobalBuffer[Idx++]=GrouhPrice[i][0]%256;
////          GlobalBuffer[Idx++]=GrouhPrice[i][0]/256;
//      } 
	
	for (i=0; i<19; i++)
	{        
	 GlobalBuffer[Idx++]=0;
	 GlobalBuffer[Idx++]=0;
	}       
	
	GlobalBuffer[4]=(Idx-6)%256; //Len;
	GlobalBuffer[5]=(Idx-6)/256; //Len;
	crc=crc16(0, GlobalBuffer+1, Idx-1);
	GlobalBuffer[Idx++]=crc&0xFF;
	GlobalBuffer[Idx++]=crc>>8;
	GlobalBuffer[Idx++]=ETX;         			
	SetTX485();
	HAL_UART_Transmit(&huart1,(unsigned char*)GlobalBuffer ,Idx,100);
	SetRX485();
	HAL_Delay(1);
	send("{\"BD\":{\"Status\":26}}\n");
	HAL_Delay(1);
	memset(fromAndroid,0,20);
}
void test(void)
{
unsigned char SendBuf[15];
	unsigned int Index = 123456;
unsigned char i, Byte; 
unsigned short crc=0;
unsigned char GlobalBuffer[15];
	unsigned char Type=10;
	unsigned int Status=1;
	
//   GlobalBuffer[0]=STX;
//   GlobalBuffer[1]=11; //Result
//   GlobalBuffer[2]=121%256;
//   GlobalBuffer[3]=121/256;
//   GlobalBuffer[4]=2;
//   GlobalBuffer[5]=0;
//   GlobalBuffer[6]=Type;
//   GlobalBuffer[7]=Status;
//   crc=crc16(0, GlobalBuffer+1, 7);
//   GlobalBuffer[8]=crc&0xFF;
//   GlobalBuffer[9]=crc>>8;
//   GlobalBuffer[10]=ETX;
//	SetTX485();
//	HAL_UART_Transmit(&huart1,(unsigned char*)GlobalBuffer ,11,100);
//	HAL_Delay(5);		 
//	SetRX485();	
//	for (unsigned char i = 0;i<123*3;i++)
//	{
  GlobalBuffer[0]=STX;
  GlobalBuffer[1]=13; //Request
  GlobalBuffer[4]=5;
  GlobalBuffer[5]=0;
	GlobalBuffer[6] = 6;//;uart1Buff[6]; 12131415
	GlobalBuffer[7] = 0x7B;//;uart1Buff[7];
	GlobalBuffer[8] = 0;//;uart1Buff[8];
	GlobalBuffer[9] = 0;//;uart1Buff[9];
	GlobalBuffer[10] = 0;//;uart1Buff[10];
  crc=crc16(0, GlobalBuffer+1, 10);
  GlobalBuffer[11]=crc&0xFF;
  GlobalBuffer[12]=crc>>8;
  GlobalBuffer[13]=ETX;
	SetTX485();
	HAL_UART_Transmit(&huart1,(unsigned char*)GlobalBuffer ,14,100);
  HAL_Delay(100);	
	WDTR
//	}
//	unsigned char GlobalBuffer[15];
//	GlobalBuffer[0]=2;
//  GlobalBuffer[1]=13; //Request
//  GlobalBuffer[4]=5;
//  GlobalBuffer[5]=0;
//  crc=crc16(0, GlobalBuffer+1, 10);
//  GlobalBuffer[11]=crc&0xFF;
//  GlobalBuffer[12]=crc>>8;
//  GlobalBuffer[13]=3;
//	HAL_UART_Transmit(&huart1,(unsigned char*)SendBuf ,11,100);
}
//---------------------------------------------------------------------
void SendAliveAck(void)
{
unsigned int Idx, crc=0, i; 
unsigned char GlobalBuffer[100];
	GlobalBuffer[0]=2;
	GlobalBuffer[1]=3; //Connection Established
	GlobalBuffer[2]=121%256;
	GlobalBuffer[3]=121/256;
	GlobalBuffer[4]=0; //Len;
	GlobalBuffer[5]=0; //Len;
	Idx=6;

	IntToBytes(303240213,GlobalBuffer+Idx); Idx+=4;

	GlobalBuffer[Idx++]=1400-1300;  
	GlobalBuffer[Idx++]=3;  
	GlobalBuffer[Idx++]=10;  
	GlobalBuffer[Idx++]=18;  
	GlobalBuffer[Idx++]=02;  
	GlobalBuffer[Idx++]=1;  
	

	GlobalBuffer[4]=(Idx-6)%256; //Len;
	GlobalBuffer[5]=(Idx-6)/256; //Len;
	crc=crc16(0, GlobalBuffer+1, Idx-1);
	GlobalBuffer[Idx++]=crc&0xFF;
	GlobalBuffer[Idx++]=crc>>8;
	GlobalBuffer[Idx++]=3;

	SetTX485();
	HAL_UART_Transmit(&huart1,(unsigned char*)GlobalBuffer ,Idx,100);
	HAL_Delay(5);		 
	SetRX485();
      
}
//-------------------------------------------------------------------------
unsigned char ProcessBD90Data(char DoorID)
{
unsigned int crc, LastDeviceRegistered, Idx, i, j, DataLen,MNB=0;
unsigned char FirstTicket=1,IsRepeated,K,DefLen=29;
unsigned char TicketBuffer[64],Do_NotIncrement=0,Byte=0,Counter,GlobalBuffer[20];
char Str[40],str2[50];
unsigned long int ID; 	
  
  if (uart1Buff[6]==5)      DefLen=29;
  else if (uart1Buff[6]==6) DefLen=31; 
  else if (uart1Buff[6]==7) DefLen=31+31; 

  DataLen=uart1Buff[4]+(uart1Buff[5]*256);
  if (DataLen==0)    
		return(0);

  DataLen-=5;
  DataLen/=DefLen;

  for (Idx=0; Idx<DataLen; Idx++)
  {
    Do_NotIncrement=0;    
      
    IsRepeated=0;
    for (i=0; i<LastPacketsNum; i++)
    {
      for (j=0; j<20; j++)
      {
        if (uart1Buff[(Idx*DefLen)+j+11]!=LastPackets1[i][j])
          break;
      }
      if (j>=20)  //means repeated record
      {
			  IsRepeated=1;
			  break;
			}  
    }		
		
    for (i=0; i<LastPacketsNum; i++)
    {
      for (j=0; j<20; j++)
      {
        if (uart1Buff[(Idx*DefLen)+j+11]!=LastPackets2[i][j])
          break;
      }
      if (j>=20)  //means repeated record
      {
			  IsRepeated=1;
			  break;
			}  
    }
		
    if (IsRepeated) 
	    continue;  
    
    LastDeviceRegistered=uart1Buff[2]+(uart1Buff[3]*256);

    TicketBuffer[0]=2;  
    for (Counter=0;Counter<22;Counter++)   TicketBuffer[Counter+1]=uart1Buff[(Idx*DefLen)+11+Counter];
		
	  ID=TicketBuffer[3]; ID<<=8;
    ID+=TicketBuffer[2]; ID<<=8;
    ID+=TicketBuffer[1];
		
    if (uart1Buff[(Idx*DefLen)+33]==0xE9)
    {
      TicketBuffer[23]=uart1Buff[(Idx*DefLen)+36];//Operator
      TicketBuffer[24]=uart1Buff[(Idx*DefLen)+37];
      TicketBuffer[25]=uart1Buff[(Idx*DefLen)+38];
      TicketBuffer[26]=uart1Buff[(Idx*DefLen)+39]; //LastOP   32
    }else{    
      TicketBuffer[23]=OperatorID&0xFF;
      TicketBuffer[24]=(OperatorID>>8)&0xFF;
      TicketBuffer[25]=(OperatorID>>16)&0xFF;
      TicketBuffer[26]=0;
    }		
		
    if (uart1Buff[(Idx*DefLen)+33]==0xE9) 
    {
      i=uart1Buff[(Idx*DefLen)+34]+(uart1Buff[(Idx*DefLen)+35]*256);
//      if (DeviceID!=i)
//        TicketBuffer[0]=22; //Unmatched Device ID  
    }       
    
    if (TicketBuffer[11]==0xAD)//ExtraStatus
    {
      TicketBuffer[0]=30+TicketBuffer[12];  
    }

    TicketBuffer[27]=0;
    TicketBuffer[28]=0; 
    TicketBuffer[29]=0;  
    
    if ((DefLen==31) || (DefLen==62))
      {
       TicketBuffer[27]=uart1Buff[(Idx*DefLen)+40];//last Dev
       TicketBuffer[28]=uart1Buff[(Idx*DefLen)+41];
       TicketBuffer[29]=0x44;
      }

    TicketBuffer[30]=0;
    TicketBuffer[31]=0; 
		
		if (DefLen==62)
		{
		 RecoredsLen=64;   
		 for (Counter=0;Counter<31;Counter++)   TicketBuffer[Counter+30]=uart1Buff[(Idx*DefLen)+42+Counter];    
		}
	else
		{
			RecoredsLen=32;  
		 for (Counter=0;Counter<31;Counter++)   TicketBuffer[Counter+30]=0;    
		}
    
    i=uart1Buff[(Idx*DefLen)+23]+(uart1Buff[(Idx*DefLen)+24]*256);
                     
    if (TicketBuffer[0]!=2)
    {
      LastDeviceRegistered=0;
    }
    if (RecoredsLen==64)
      {
       Transactions2++;   
       OffTransactions2++; 
      }
    else
      {
       Transactions++;   
       OffTransactions++;  
      }
			
		if (LastDeviceRegistered==1)
    {  
      if (!Do_NotIncrement)      
         Door1Num++;
      Door1CurNum++;
      DoorPrice=i;
    }
    else if (LastDeviceRegistered==2)
    {
      Door2CurNum++;
      DoorPrice=i;
      if (!Do_NotIncrement)  
         Door2Num++;
    }
      
		if (LastDeviceRegistered!=0)  //?????????
			ExtraStatus=1;

  for (Idx=0; Idx<DataLen; Idx++)
  {
    for (j=0; j<20; j++)
      LastPackets[LastPacketsPos][j]=uart1Buff[(Idx*DefLen)+j+11];
    if (++LastPacketsPos>=LastPacketsNum)
      LastPacketsPos=0;    
  }
  GlobalBuffer[0]=STX;
  GlobalBuffer[1]=13; //Request
	GlobalBuffer[2]= uart1Buff[2];
	GlobalBuffer[3]= uart1Buff[3];
  GlobalBuffer[4]= 5;
  GlobalBuffer[5]= 0;
	GlobalBuffer[6] = uart1Buff[6];
	GlobalBuffer[7] = uart1Buff[7];
	GlobalBuffer[8] = uart1Buff[8];
	GlobalBuffer[9] = uart1Buff[9];
	GlobalBuffer[10] = uart1Buff[10];
  crc=crc16(0, GlobalBuffer+1, 10);
  GlobalBuffer[11]= crc&0xFF;
  GlobalBuffer[12]= crc>>8;
  GlobalBuffer[13]= ETX;
	SetTX485();
	HAL_UART_Transmit(&huart1,(unsigned char*)GlobalBuffer ,14,100);
	HAL_Delay(5);		 
	SetRX485();	
	}
}
//-----------------------------------------------------
unsigned char ProcessPacket(unsigned char Port) 
{
unsigned char str[150],BCC, Byte, Col, Row=0, Idx;
unsigned int  CardID, Counter,Len, i, j,k,  SerialChip, DID,OfflineCounter,Data_Length; 
unsigned short crc=0;
unsigned long int Limit, Address,Sector;   
unsigned long int Temp,time;    
unsigned char Error=0;
unsigned char Refresh=0;
unsigned short UC;

	
  if (uart1Len < 5)
    return(1);
	

//  if (uart1Len  < (uart1Buff[4]+(uart1Buff[5]*256)))
//    return(2);   
//  
//  if ((uart1Buff[0]!=2) || (uart1Buff[uart1Len-1]!=3))
//    return(3);  


//  crc=crc16(0, uart1Buff+1, uart1Len-4);
//  if (crc!=(uart1Buff[uart1Len-3]+(uart1Buff[uart1Len-2]*256)))
//      return(4);       

  DID=uart1Buff[3]; DID<<=8;
  DID=DID+uart1Buff[2];  
	
  if (uart1Buff[1]!=14)
    {
     if ((DID!=0xFFFF) && ((uart1Buff[1]>=10) || (uart1Buff[1]==1)) && (uart1Buff[1]!=12) && (uart1Buff[1]!=91))
        return(5);
    }

//  ConnectionError=0;
//  ActivePort=Port; 
	
	//if (uart1Buff[1]!=12) ConnectedToServer=1;
	
	WDTR;
		
  switch (uart1Buff[1])
  {
    case 3:  //Alive
      switch (DID)
      {
        case 1:
					BD1ReadyTimer=30;
					BD1Ready=1;
					Refresh=0;
          Door1Statuses=uart1Buff[7];
					Door1Ver=uart1Buff[9]+(uart1Buff[10]*256);
					Door1Release=uart1Buff[11]+(uart1Buff[12]*256);
					Door1Model=uart1Buff[13];
//  				i=uart1Buff[9]+(uart1Buff[10]*256);
//				  if (Door1Ver!=i) Refresh=1;
//          Door1Ver=i;
//				
//					i=uart1Buff[11]+(uart1Buff[12]*256);
//				  if (Door1Release!=i) Refresh=1;
//          Door1Release=i;				
//				  if (Door1Model!=uart1Buff[13]) Refresh=1;
//          Door1Model=uart1Buff[13];
//				
//				  if (uart1Buff[4]==10) //Len 10
//				  {
//  				  i=uart1Buff[14]+(uart1Buff[15]*256);
//            if (DoorPrice!=i)
//							BD1Ready=0;
//            //DoorPrice=i;
//				  }

//  				if (BD1Ready==0)
//				  {
//						 BD1Ready=1;				
//             //ShowDoorsNumber(1); must edit							
//			    }
          break;
						
        case 2:
					Refresh=0;
				  BD2ReadyTimer=30;
          Door2Statuses=uart1Buff[7];
					BD2Ready=1;
          Door2Statuses=uart1Buff[7];
					Door2Ver=uart1Buff[9]+(uart1Buff[10]*256);
					Door2Release=uart1Buff[11]+(uart1Buff[12]*256);
					Door2Model=uart1Buff[13];				  
//  				i=uart1Buff[9]+(uart1Buff[10]*256);
//				  if (Door2Ver!=i) Refresh=1;
//          Door2Ver=i;
//				
//					i=uart1Buff[11]+(uart1Buff[12]*256);
//				  if (Door2Release!=i) Refresh=1;
//          Door2Release=i;
//				
//				  if (Door2Model!=uart1Buff[13]) Refresh=1;
//          Door2Model=uart1Buff[13];
//				
//				  if (uart1Buff[4]==10) //Len 10
//				  {
//  				  i=uart1Buff[14]+(uart1Buff[15]*256);
//            if (DoorPrice!=i)
//							BD2Ready=0;
//            //Indicators.DoorsData.Door2Price=i;
//				  }

//				  if (BD2Ready==0)
//					{
//						 BD2Ready=1;	 		
//             //ShowDoorsNumber(2);	must edited 						
//				  }

//          BD2ReadyTimer=17;  //12Sec
          BD2Ready=1;
          break;                                                                                                                                                                                                                                                                                                                                               
      }  
			HAL_Delay(50);
			SendConfig();
      //SendAliveAck(); //must edit
      break;

    case 12: //Data  		
       if ((uart1Buff[6]==5) || (uart1Buff[6]==6) || (uart1Buff[6]==7)) //BD90 Data
        {   
         if (DID==1)       {BD1Ready=1;BD1ReadyTimer=12; }
				 else if (DID==2)  {BD2Ready=1;BD2ReadyTimer=12; }
         ProcessBD90Data(DID);
        }
       SetRX485(); 
       break;						 
      
    case 10:  //Command 
      //ProcessCommands();  
      break;
      
    
//    case 13: //Data Accepted
//           switch (uart1Buff[6]) 
//                 {
//  	              case 2:  //Online Data ACK
//										
//          Address=uart1Buff[10]; Address<<=8;
//          Address=Address+uart1Buff[9]; Address<<=8;
//          Address=Address+uart1Buff[8]; Address<<=8;
//          Address=Address+uart1Buff[7];
//									
//				  if (Address==WaitForAddress)
//				  {
//            Sector=addTransactions+((Address==0?(MAX_TRANSACTIONS-1):Address-1)*LEN_TRANSACTIONS);
//            if (Address==trHead)
//            {
//              if (trHead==0)
//                Sector=addTransactions+((MAX_TRANSACTIONS-1)*LEN_TRANSACTIONS);
//              else
//                Sector=addTransactions+((trHead-1)*LEN_TRANSACTIONS);
//            }
//            str[0]=0x90;
//            SaveFromRamToDF(Sector+LEN_TRANSACTIONS-1, 1, str);
//            Indicators.OffTransactions=(Address>trHead)?(MAX_TRANSACTIONS-Address)+trHead:trHead-Address;

//            LoadFont("Koodak40.sif");
//            GUI_DrawGradientV(330, 5, 400, 28,0x000F0000,0x00070000);							

//		        GUI_SetColor(GUI_WHITE); 
//            sprintf(str,"?: %d",Indicators.OffTransactions);
//            PutText(330, 0, 400, 25, str, GUI_TA_RIGHT);
//	 
//            //sprintf(str,"?: %d",Indicators.OffLocations);
//            //PutText(250, 5, 330, 30, str, GUI_TA_RIGHT);
//						
//				  }
//				  WaitForAddress=0;
//          if (ConnectedToServer) SendOfflines();
//			  }
//		
//       break;     

		case 14:
      if ((uart1Buff[2]+uart1Buff[3]*256==1))
				{
				 Door1Model=uart1Buff[6];
 			   if (BD1Ready==0)
					  {
						 BD1Ready=1;		 		
             //ShowDoorsNumber(1);							
				    }

          BD1ReadyTimer=30;  //12Sec
          BD1Ready=1;
				 }
      if ((uart1Buff[2]+uart1Buff[3]*256==2)) 
			{
				 Door2Model=uart1Buff[6];
 			   if (BD2Ready==0)
					  {
						 BD2Ready=1;		 		
             //ShowDoorsNumber(2);							
				    }

          BD2ReadyTimer=30;  //12Sec
          BD2Ready=1;				
			}
			
//			if (uart1Buff[6]==165) //BlackList
//			{
//				k=uart1Buff[7]+(uart1Buff[8]*256);
//				
//				j=(k-1)*64;
//				if (j+64<=BlackList_Count)
//					Len=64;
//				else
//				{
//					if (j>BlackList_Count)
//						Len=0;
//					else
//					  Len=BlackList_Count-j;
//				}

//				Len*=8;
//				LoadFromDFToRam(addBlackList+(j*8), Len, &uart1Buff[9]);

//        uart1Buff[0]=STX;
//        uart1Buff[1]=15; 
//        //uart1Buff[2]=Config.DeviceID%256;
//        //uart1Buff[3]=Config.DeviceID/256;
//        uart1Buff[4]=Len+3&0xFF;
//        uart1Buff[5]=(Len+3>>8)&0xFF;
//        uart1Buff[6]=165;
//        uart1Buff[7]=k&0xFF;
//        uart1Buff[8]=(k>>8)&0xFF;
//				
//        crc=crc16(0x33, uart1Buff+1, Len+8);
//        uart1Buff[Len+9]=crc&0xFF;
//        uart1Buff[Len+10]=crc>>8;
//        uart1Buff[Len+11]=ETX;
//				
//        SetTX485();
//				for (i=0; i<Len+12; i++)
//				  SendByte(uart1Buff[i]);
//      	GUI_Delay(5);
//        SetRX485();
//				break;
//			}
      
//      if (uart1Buff[6]==LABKHANDDOOR)      
//				Send_FirmwarePacketToDoors(uart1Buff[6],uart1Buff[2]+uart1Buff[3]*256,uart1Buff[7]+uart1Buff[8]*256);
      
      break;
						
						
    case 15:  //Data
      //ProcessData(); 
      break;
   }//switch
  return(0);
}

#endif

//*********************************************************************************
#if (deviceType == BCU) || (deviceType == BD) || (deviceType == BusEtebar)
unsigned char rwGroupPrice(unsigned char rw,unsigned gp,unsigned char *snr)
{
   unsigned short Login_First_Arg = 5,Login_Second_Arg = 0x37,Read_Write_First_Arg=22,Login_Successfully=1;
   unsigned int tick;
   unsigned char Source_Buffer[32],Readed_BCC,BCC=0;
   if (rw==0) //read
   {
		 HAL_RTC_GetTime(&hrtc,&time,RTC_FORMAT_BIN);
		 HAL_RTC_GetDate(&hrtc,&date,RTC_FORMAT_BIN);
	   tick = HAL_GetTick();
	   while (Login_Successfully)
	   {
		   if (HAL_GetTick() - tick > 2000 ) {send("{\"C\":{\"Status\":1}}\n");  return 255;}
			 WDTR
			 if (ISO14443_SingleTagSelect(snr))
			 {
				 if (ISO14443_SingleTagSelect(snr))
					 continue;
			 } 
			 if (!ISO14443_LoginE2(Login_First_Arg,Login_Second_Arg))
			 {
				 if (!ISO14443_ReadBlock(Read_Write_First_Arg, Source_Buffer))
				 {
					 Readed_BCC = 0;
					 for (unsigned char index=0; index<15; index++){Readed_BCC^=Source_Buffer[index];}
					 if (Source_Buffer[15]==Readed_BCC)
					 {
						 if (Source_Buffer[5] > Source_Buffer[7])
							 return 255;	
						 
						 if ((Source_Buffer[5] > 23) || (Source_Buffer[7]>23) || (Source_Buffer[1] > 12) || (Source_Buffer[4] > 31) || (Source_Buffer[6] > 59) || (Source_Buffer[8] > 59))
							 return 255;	
						 
						 if ((Source_Buffer[14]<0) || (Source_Buffer[14]>9))
							 return 255;	
						 
							if (Source_Buffer[14] == 0)
								return Source_Buffer[14];
							
						 if (Source_Buffer[0] < date.Year)
							 return 254; // expire 
						 else if (Source_Buffer[0] == date.Year)
						 {
							 if (Source_Buffer[1] < date.Month)
								 return 254; // expire 
							 else if (date.Month == Source_Buffer[1])
							 {
								 if (Source_Buffer[4] < date.Date)
									 return 254; // expire 
							 }
						 }						 
						 if (time.Hours < Source_Buffer[5])
							 return 254;
						 else if (time.Hours == Source_Buffer[5])
						 {
							  if (time.Minutes < Source_Buffer[6])
									return 254;
						 }						 
						 if (time.Hours > Source_Buffer[7])
							 return 254; // expire 
						 else if (time.Hours == Source_Buffer[7])
						 {
							 if (time.Minutes > Source_Buffer[8])
								 return 254; //expire 
						 }
						 return Source_Buffer[14];
					 }
					 else return 255;
				 }	
				 else continue;				
			 }
			 else continue;
	    }				   		   	   
    }
   else if (rw==1) //write 
   {
		 //07 gp Ye Me De Hs ms He me codeMeli0 codeMeli1 codeMeli2 codeMeli3 codeMeli4 crc  , miladi 		 
		 for (unsigned char index=0; index<14; index++){BCC^=fromAndroid[index];}
		 if (BCC != fromAndroid[14]) 
		 {
			 INVALIDJson
			 return 1;
		 }		 
		 if (fromAndroid[5] > fromAndroid[7])
		 {
			 send("{\"C\":{\"Status\":2}}\n");
			 return 1;
		 }		 
		 if ((fromAndroid[5] > 23) || (fromAndroid[7]>23) || (fromAndroid[3] > 12) || (fromAndroid[4] > 31) || (fromAndroid[6] > 59) || (fromAndroid[8] > 59))
		 {
			 send("{\"C\":{\"Status\":3}}\n");
			 return 1;
		 }
		 if ( (fromAndroid[9] >= 100) || (fromAndroid[10] >= 100) || (fromAndroid[11] >= 100) || (fromAndroid[12] >= 100) || (fromAndroid[13] >= 100))
		 {
			 send("{\"C\":{\"Status\":4}}\n");
			 return 1;
		 }
		 
		 send("{\"Receive\":{\"Status\":26},\"C\":{\"Type\":\"ChangeGroup\"}}\n");
	   tick = HAL_GetTick();
	   while (Login_Successfully)
	   {
		   WDTR;
		   if (HAL_GetTick() - tick > 5000 ) {send("{\"C\":{\"Status\":1}}\n");  return 255;}
		   if (ISO14443_SingleTagSelect(snr))
				{
					if (ISO14443_SingleTagSelect(snr))
						continue;
				} 
				Readed_BCC=0;
				Source_Buffer[0] = fromAndroid[2]; // year miladi, payan takhfif Ye
				Source_Buffer[1] = fromAndroid[3]; // month, payan takhfi  Me
				
				#if (deviceType == BCU) || (deviceType == BD)
				Source_Buffer[2]=Config.DeviceID%256; 
				Source_Buffer[3]=Config.DeviceID/256;
				GpFlag=0;
				#else
				Source_Buffer[2] = 0; 
				Source_Buffer[3] = 0;				
				#endif
				
//				HAL_RTC_GetTime(&hrtc,&time,RTC_FORMAT_BIN);
//				HAL_RTC_GetDate(&hrtc,&date,RTC_FORMAT_BIN);	
				Source_Buffer[4] = fromAndroid[4]; // day, payan takhfif De 
				Source_Buffer[5] = fromAndroid[5]; // saat shoro takhfif dar roz ,Hs
				Source_Buffer[6] = fromAndroid[6]; // daghighe shoro takhfif dar roz  ms
				Source_Buffer[7] = fromAndroid[7]; // saat payan takhfif dar roz, He
				Source_Buffer[8] = fromAndroid[8]; // daghighe payan takhfif dar roz, me
				//0021302286
				Source_Buffer[9] = fromAndroid[9];  // code meli  00 
				Source_Buffer[9] = fromAndroid[10]; // code meli  15
				Source_Buffer[10]= fromAndroid[11]; // code meli  1e
				Source_Buffer[11]= fromAndroid[12]; // code meli  16
				Source_Buffer[12]= fromAndroid[13]; // code meli  56
				Source_Buffer[13]= 0 ; //  resve 
				Source_Buffer[14]= gp;   
				for (unsigned char index=0; index<15; index++){Readed_BCC^=Source_Buffer[index];}
				Source_Buffer[15]=Readed_BCC;
				if (!ISO14443_LoginE2(Login_First_Arg,Login_Second_Arg))
				{
					if (!ISO14443_WriteBlock(Read_Write_First_Arg, Source_Buffer))
					{
						send("{\"C\":{\"Type\":\"ChangeGroup\",\"Status\":0}}\n");
						return 0;
					}
					else 
					{
						send("{\"C\":{\"Type\":\"ChangeGroup\",\"Status\":19}}\n");
						return 255;
					}
				}
				else 
				{
					send("{\"C\":{\"Type\":\"ChangeGroup\",\"Status\":19}}\n");
					return 255;
				}
			}
		}
}
//**************************************************************************************
#ifdef ExecTransactionBD
unsigned char ProcessCard(unsigned char MifareType,unsigned char *snr)
{
unsigned short Status;
unsigned int LastDevice;
unsigned int UC, Price,CardTimeout2,_ID;
unsigned long int ID=0;
unsigned long int MaxCharge;
unsigned char  Byte, i, crc,LastOP,OPIndex,buf[32]; 
signed long int Etebar, PreEtebar,RemEtebar,MinCharge,TempInt;
unsigned int Grouh;
char str[100],Try=0;
static unsigned char RepeatedCard_Counter=0;
unsigned char TicketBuffer[100],LastCardingDateTime[10];

 MinCharge=0;
 Grouh=1;
 
//   if (MifareType==2) 
//   {
//     Status=0;
//     if (ISO14443_LoginE2(0,0x10))
//     {
//       if (ISO14443_SingleTagSelect(snr))
//         ISO14443_SingleTagSelect(snr);
//       if (!ISO14443_LoginE2(0,0x10))
//         Status=1;
//     }else 
//       Status=1;

////     if (Status==1)
////     {
////       if (!ISO14443_ReadBlock(1, buf)) 
////       {
////         GenerateCardIdUC(buf,snr,&UC,&_ID);
////         ID=_ID;
////         #ifdef WithSAMCARD
////         if (CheckCard(UC, ID)==0)
////           ShowMessage("???? ????? ??? ??? .",R2L,60,10,RGB(200,0,0),RGB(255,0,0),7);
////         #else
////         if (CheckCard(UC, ID)!=0)
////           return(1);
////         #endif
////       }
////       #ifdef WithSAMCARD
////       return(1);
////       #endif
////     }
//      
//  
//      Try=0;
//      switch (Config.UC) //EDITED 
//      {
//        case 6355:
//          if (ID>1050000)
//            Try=1;
//          break;
//        case 6345://rafsanjan
//          if (ID>650000)
//            Try=1;
//          break;
//        case 6378://torbat 
//          if (ID>650000)
//            Try=1;
//          break;
//        case 6347:  //Shahreza   
//          if (ID>870000)
//            Try=1;
//          break;
//        case 6105: //Sabzevar
//          if (ID>1700000)
//            Try=1;
//          break;
//        case 6352: //maraghe
//          if (ID>1400000)
//            Try=1;
//          break;
//        case 6364: //gorgan
//          if (ID>3300000)
//            Try=1;
//          break;
//        case 6374: //khoy
//          if (ID>600000)
//            Try=1;
//          break;
//        case 6375: //malayer
//          if (ID>10000)
//            Try=1;
//          break;
//        case 6350: //mobarake
//          if (ID>1050000)
//            Try=1;
//          break;
//        case 6348: //shahrood
//          if (ID>1200000)
//            Try=1;
//          break;
//        case 6115: //mahshahr
//          if (ID>520000)
//            Try=1;
//          break;
//        case 6398: //saghez
//          if (ID>280000)
//            Try=1;
//          break;
//        case 6393:  //falavarjan  
//          if (ID>630000)
//            Try=1;
//          break;
//        case 6358: //Ghochan
//          if (ID>920000)
//            Try=1;
//          break;
//        case 1010: //Khormabad
//          if (ID>1100000)
//            Try=1;
//          break;
//        case 6380: //Ardebil
//          if (ID>2600000)
//            Try=1;
//          break;
//        case 6330: //Neyshabour
//          if (ID>4250000)
//            Try=1;
//          break;
//        case 6325: //Bojnoord
//          if (ID>355000000)  //edited //
//            Try=1;
//          break;
//        case 5780: //ShahinShahr
//          if (ID>7400000)
//            Try=1;
//          break;
//        //case 6350: //Zibashahr
//        //  if (ID>45000)
//        //    Try=1;
//        //  break;
//        case 6351: //Bushehr edited 
//          if (ID>7400000)
//            Try=1;
//          break;
//          
//        default:
//          Try=1;
//          break;
//      }
//      #ifdef WithSAMCARD
//      Try=0;
//      #endif
//      
//      if (Try)
//      {
//        #if (DisplayType==TFT22) 
//        SetFgColor(RED);
//        LcdFont((unsigned char *)nazanin19);
//        LCD_Clear(BLACK);            
//        Show_TextInTFT(220,70,0,150,"??? ???? ???? ??? .",R2L,1); 
//        SetColor(WHITE);
//        RectangleFill(0, 0, 220, 20, 0);
//        SetFgColor(RED);
//        LcdFont((unsigned char *)tahoma16);
//        sprintf(str, "Card [%d - %d]", UC, ID);
//        PutText(0,2,0,0, str, ALINE_LEFT);
//        #elif (DisplayType==TFT24) 
//        ShowMessage("??? ???? ???? ??? .",R2L,60,10,RGB(200,0,0),RGB(255,0,0),7); 
//        SetFgColor(RED); 
//        LcdFont((unsigned char *)tahoma16);
//        sprintf(str, "Card [ %d - %d ]", UC,ID);
//        PutText(20,30,0,0, str, ALINE_LEFT); 
//        #elif (DisplayType==TFT35) 
//        SetColor(RGB(200,20,20));
//        RectangleFill(16,80, 304, 160,8); 
//        SetColor(RGB(0,0,0));    
//        Rectangle(16,80, 304, 160,8); 
//        LcdFont((unsigned char *)nazanin19);
//        SetFgColor(WHITE);    
//        Show_TextInTFT(320,115,0,115,(unsigned char *)"??? ???? ???? ??? .",R2L,1);
//        LcdFont((unsigned char *)tahoma16);
//        sprintf(str, "Card [ %d - %d ]", UC,ID);
//        PutText(100,85,0,0, str, ALINE_LEFT);  
//        #endif
//        InfoCounter=2;
//        return(1);
//      }
//   }
 
   Price = GrouhPrice[rwGroupPrice(0,0,snr)]; 
//     switch (Config.UC)
//      {
//        case 6364:   //Gorgan
//        case 6348://Shahrood 
//          MaxCharge=50000;
//          break;  
//        case 6374:   //Khoy
//        case 5750:
//        case 6025:  //save
//     
//        case 6352:  //maraghe   
//        case 6373:  //aran
//        case 6356://Zanjan      
//        case 6110://Khorramshahr 
//          MaxCharge=10000;
//          break;
//        case 6351:      
//        case 7712:  //Eshragh  
//        case 6347:  //Shahreza   
//        case 6393:  //falavarjan   edited 20,000 ---> 200,000
//        case 2937: 
//        case 6346:  //Ghom  
//        case 6398:   //saghez          
//        case 5780:  //shahinshahr     
//        case 6310://Broojerd             
//          MaxCharge=65000;
//          break; 
//        case 6395:  //Fasa   
//        case 6330:  //neyshaboor          
//          MaxCharge=40000;
//          break;           
//        case 6345://rafsanjan
//        case 6358://Ghochan  
//        case 1010://Lorestan UNI
//        case 6391://Mahabad   
//        case 5855://kerman uni    
//        case 6404://Shiraz  uni            
//          MaxCharge=50000;
//          break;               
//        case 6350: 
//        case 6355:
//          MaxCharge=15000;
//          break;     
//        case 6378://torbat 
//          MaxCharge=25000;
//          break;            
//        case 7090://Moozeha
//        case 6380://Ardebil    
//        case 6135://Marvdasht          
//          MaxCharge=100000;
//          break;   
//        case 6105://Sabzevar 
//          MaxCharge=11000;
//          break;     
//        case 6115://Mahshahr
//          MaxCharge=100000;
//          break;
//        case 6325:  //bojnoord       
//          MaxCharge=30000;
//          break;          
//       default:
//          MaxCharge=5000;  
//      }
      
//      //-------------------------------------------
//      if (Config.UC==6356)
//        {
//         if ((ID>=224076) && (ID<=224250)) 
//           {
//            MaxCharge=40000;
//           }
//        }

      //-------------------------------------------
//      if (Config.UC==6325)  //Bojnoord
//         MinCharge=-2400; //-1200;

//      if (Config.UC==6364)  //Gorgan
//         MinCharge=-(Price*5);
      
//      ID=0;                             
//      UC=Config.UC; 
  if (MifareType==2)
    {
//      #ifdef WithSAMCARD
//		
//      Price=Config.GrouhPrice[0][0]; 

//      //SetTX485();
//      Status=ReadCardIDKashanMifare(&ID, Price*10,  &Etebar, &PreEtebar);
//      if ((Status>=19) && (Status<=29)) Status=ReadCardIDKashanMifare(&ID, Price*10,  &Etebar, &PreEtebar);
//      //printf("\n\rStatus %d",Status);
//      //SetRX485();

//      Status=ShowErrorKashan(MifareType, Status);  
//      
//      #else
//      #if (CardType==ctUniversityBus)
//      CreditLogin=0x35;
//      if (ISO14443_LoginE2(0x0E,0x35))
//      {
//     
//        if (ISO14443_SingleTagSelect(snr))
//          if (ISO14443_SingleTagSelect(snr))
//            return(0);        
//        if (ISO14443_LoginE2(0x0E,0x13)) return(0);
//        CreditLogin= 0x13;
//      }  
//      Status=ServiceExecTransaction(2, &ID, &UC, Price, MinCharge, MaxCharge, Config.BusID, &Etebar, &PreEtebar,&LastDevice,&LastOP,&OPIndex);
//      #else
//      Status=ExecTransaction(2, &ID, &UC, Price, MinCharge, MaxCharge, Config.BusID, &Etebar, &PreEtebar,&LastDevice,&LastOP,&OPIndex);
  
      if (Status==6) //Is Old Format  
      {
        ID=0;                             
        UC=Config.UC;
        Status=Old_ExecTransaction(2, &ID, &UC, Price, MinCharge, MaxCharge, Config.BusID, &Etebar, &PreEtebar,&LastDevice,&LastOP,&OPIndex);
        if (Status==1) //Is Old Format  
          {
           ID=0;                             
           UC=Config.UC;
           Status=Old_ExecTransaction(2, &ID, &UC, Price, MinCharge, MaxCharge, Config.BusID, &Etebar, &PreEtebar,&LastDevice,&LastOP,&OPIndex);
          } 
      }
     }
//   else if (MifareType==3)
//     {
////      #ifdef WithSAMCARD
////      GetTimeStamp(); 
////      XLi=timestamp;
////        
////      Price=Config.GrouhPrice[0][0]; 
////   

////     SetTX485();
////      Status=ReadCardIDKashan(&ID, Price*10,  &Etebar, &PreEtebar);
////      if ((Status>=19) && (Status<=29)) Status=ReadCardIDKashan(&ID, Price*10,  &Etebar, &PreEtebar);
////     // SetTX485();
////     // printf("\n\rID=%x ",ID);
////      SetRX485();

////      #endif

////      Status=ShowErrorKashan(MifareType, Status);  
//     
//   }
     //............................................................................... 
     if (Status==S_Success)
       if ((PreEtebar-Etebar)!=Price)
       {   
         Price=PreEtebar-Etebar;
       }

    //...............................................................................      
//      if (Status==11)
//        {
//         if (CheckCard(UC, ID))
//           return;
//        }
 
      TicketBuffer[0]=ID&0xFF;
      TicketBuffer[1]=(ID>>8)&0xFF;
      TicketBuffer[2]=(ID>>16)&0xFF;
      TicketBuffer[3]=UC;
      TicketBuffer[4]=Year-1300;
      TicketBuffer[5]=Month;
      TicketBuffer[6]=Day;
      TicketBuffer[7]=Hour;
      TicketBuffer[8]=Min;
      TicketBuffer[9]=Sec;

      TicketBuffer[10]=Grouh&0xFF;
      TicketBuffer[11]=(Grouh>>8)&0xFF;
      
      TicketBuffer[12]=Price&0xFF;
      TicketBuffer[13]=(Price>>8)&0xFF;
      
      if (PreEtebar<0)
        {
         TempInt=0x100000000-PreEtebar;
         TempInt|=(0x800000);
        }
      else
         TempInt=PreEtebar;
      
      TicketBuffer[14]=TempInt&0xFF;
      TicketBuffer[15]=(TempInt>>8)&0xFF;
      TicketBuffer[16]=(TempInt>>16)&0xFF;
      TicketBuffer[17]=(TempInt>>24)&0xFF;
      
      if (Etebar<0)
        {
         TempInt=0x100000000-Etebar;
         TempInt|=(0x800000);
        }
      else
         TempInt=Etebar;     
      
      TicketBuffer[18]=TempInt&0xFF;
      TicketBuffer[19]=(TempInt>>8)&0xFF;
      TicketBuffer[20]=(TempInt>>16)&0xFF;
      
      TicketBuffer[22]=0xE9;
      TicketBuffer[23]=Config.BusID%256;
      TicketBuffer[24]=Config.BusID/256; 
      TicketBuffer[25]=Config.OperatorID&0xFF;
      TicketBuffer[26]=(Config.OperatorID>>8)&0xFF;
      TicketBuffer[27]=(Config.OperatorID>>16)&0xFF;
      TicketBuffer[21]=LastOP;
      TicketBuffer[28]=LastDevice%256;
      TicketBuffer[29]=LastDevice/256;      
      switch (Status)
      {
        case S_Success:
          LastCardingDateTime[0]=Year-1300;
          LastCardingDateTime[1]=Month;
          LastCardingDateTime[2]=Day;
          LastCardingDateTime[3]=Hour;
          LastCardingDateTime[4]=Min;
          LastCardingDateTime[5]=Sec;
          
//          if (LastCardSec>Sec)  CardTimeout2=(60-LastCardSec)+Sec;
//          else                 CardTimeout2=Sec-LastCardSec;
        

          if (Etebar<0)
            {             
             if ((Config.UC==6325) || (Config.UC==6364))
             {
               // Show_TextInTFT(GetMaxY,100,0,100,"???? ?????? ????? - ?????",R2L,1);  // عتبار ندارد 
             }
             else
               {
               // Show_TextInTFT(220,60,0,150,"???? ?????? ????? - ?????",R2L,1); //اعتبار ندارد 
               }
            }
          else     
//            Show_TextInTFT(220,120,0,120,"????? ?????? - ?????",R2L,1); 
          
//          if ((CardTimeout2<10) &&((snr[0]==LastCardSnr[0]) && (snr[1]==LastCardSnr[1]) && (snr[2]==LastCardSnr[2]) && (snr[3]==LastCardSnr[3])))
//            {
//              RepeatedCard_Counter++;                            
//            }
//          else
//            {
//              RepeatedCard_Counter=0;
//              if (Etebar>=0)
//                {
//                 Show_TextInTFT(220,7,0,7,"??? ?????",R2L,1);                
//                }
//            }                   
//          sprintf(str,"%d" , Etebar);         
//          if (Etebar>=0) //Show_TextInTFT(220,40,0,40,(unsigned char *)str,R2L,1);

//          else
//            {
//              RepeatedCard_Counter=0;
////              Show_TextInTFT(130,85,0,40,"??? ?????",R2L,0);    // خوش آمديد             
//            } 
//          LcdFont((unsigned char *)Btitr51);//(Trafic64);
//          sprintf(str,"%d" , Etebar);
//          SetFgColor(BLACK);          
//          Show_TextInTFT(310,0,0,0,(unsigned char *)str,R2L,1); 
//          LcdFont((unsigned char *)nazanin19);  
//          SetFgColor(RGB(0,100,0)); 
//          Show_TextInTFT(325,85,0,40,"?????? :",R2L,0);         // اعتبار       
//          Show_TextInTFT(60,32,0,85,"?????",R2L,0);  // تومان 

          
//          if (WriteData2Flash(Indicators.Transactions, TicketBuffer, RecordLen))
//             WriteData2Flash(Indicators.Transactions, TicketBuffer, RecordLen);

          

//          Transactions++;   
          OffTransactions++; 

//          Indicators.TransactionUID++;
//        
//         if ((Indicators.TransactionUID&0x000000FFFF)>=65536) 
//          {
//           Indicators.TransactionUID=0;     
//          }
//          SaveIndicators();
         
          

          
//					LastCardSnr[0]=snr[0];
//					LastCardSnr[1]=snr[1];
//					LastCardSnr[2]=snr[2];
//					LastCardSnr[3]=snr[3];
//					LastCardSec=Sec;
//          Grouh=0; //Default

//          LastOffTrans=Indicators.OffTransactions;
//          if (/OffTransactions)
//            {
//             sprintf(str,"%d",Indicators.OffTransactions);
//            }   
          
//          InfoCounter=2;
          break;  
    
        case S_OutOfRange:      
//					Show_TextInTFT(220,60,0,150,"???? ??? ???? ?????.",R2L,1);  // کارت غير فعال است 
//             sprintf(str, "%d",Etebar);
          break;  
				
        case S_InvalidCardUC:
					
          break;
        default:
 //          InfoCounter=2;
          break;
      }
  return(Status);
} 
#endif
//***********************************************************************
#if (deviceType == BCU)
#if defined LCD
#define GetMaxX GetMaxX2()
#define GetMaxY GetMaxY2() 
void fistPage(void)
{
	unsigned char str[50];
	unsigned int Li = 0;
	LCDClear(YELLOW);
	SetFgColor(WHITE);
	SetColor(BLACK);  
  RectangleFill(0,30, GetMaxX+3, GetMaxY-3,8); 
  LcdFont((unsigned char *)tahoma16);
  sprintf(str, "DOOR BUS SUN Ver %d",Release);
  PutText(5,150+70,0,0, str, ALINE_LEFT);
	
	if (PN532_GetFirmwareVersion(&pn532, versionPn532,0) == 0)
			PutText(5,150+40,0,0,"READER Ok", ALINE_LEFT);
	else
			PutText(5,150+40,0,0,"READER Error", ALINE_LEFT);
	
  sprintf(str, "UC %d",Config.UC);
  PutText(5,150+10,0,0, str, ALINE_LEFT);
	
  sprintf(str, "DEVICE ID %d",Config.DeviceID);
  PutText(5,150-20,0,0, str, ALINE_LEFT);
	Li = 1;
	while (1)
	{
		ProgressBar2(40,50,430,75, Li, 6000,WHITE);
		WDTR
		Li +=300;
		if (Li>6000) break;
	}
	SetColor(BLACK);  
  RectangleFill(0,30, GetMaxX+3, GetMaxY-3,8);	
	Show_Image((unsigned char *)Jahan_logo,312,234,90,240,WHITE,0);	
  HAL_Delay(1000);	
	SetColor(RGB(128,64,00));  
  RectangleFill(0,30, GetMaxX+3, GetMaxY-3,8);	
  SetColor(RGB(0,120,250));  
  RectangleFill2(25,  GetMaxY-50, 75,  GetMaxY,0);
  SetColor(RGB(0,70,200));  
  RectangleFill2(25+25,  GetMaxY-50, 75+25,  GetMaxY,0);
  SetColor(RGB(0,20,150));  
  RectangleFill2(25+50,  GetMaxY-50, 75+50,  GetMaxY,0);   
  RectangleFill(100,  GetMaxY-50, GetMaxX-30,  GetMaxY,0);
  LcdFont((unsigned char *)nazanin19);
  SetFgColor(RGB(255,255,128));
  Show_TextInTFT(GetMaxX-60,GetMaxY-30,20,60,"Ǌ戦Ӡч䭠妔㤏 ",R2L,0); // otobos ranie hoshmand 
	//-------------------------------------------------------------------------------------------------
}
void showMainPrice(unsigned int pricee,unsigned char pricePublic)
{	
	uint8_t str[10];
  SetColor(RGB(255,255,255));    
  RectangleFill(19,160, GetMaxX-15, GetMaxY-47,0);
	SetColor(BLACK);  
	Line2(19,162,GetMaxX-15,162);
	SetColor(RGB(255,255,255)); 
	SetFgColor(0);
	LcdFont((unsigned char *)nazanin19);
	if (!pricePublic)
		Show_TextInTFT(GetMaxX-60,GetMaxY-100,20,60,"ȥǭ ȡ혢,R2L,0); // bahaye bilit 
	else
		Show_TextInTFT(GetMaxX-60,GetMaxY-100,20,60,"ȥǭ ȡ혠Ǐ",R2L,0); // bahaye bilit azad 
	if (flagRc522 == 0)
	{
		refreshPage = 1;
		LcdFont((unsigned char *)nazanin19);
		Show_TextInTFT(320,GetMaxY-100,0,25,"䭇Ҡȥ ǑӇ᠊䙭㇊",R2L,1);		
		return;
	}
	if (pricee==0)
	{
		 Show_Image((unsigned char *)RayganBmp,144,63,100,230,BLACK,0);
	}
	else
	{
		LcdFont((unsigned char *)Btitr51);
		sprintf(str,"%d",pricee);
		Show_TextInTFT(320,GetMaxY-150,0,25,(unsigned char *)str,R2L,1);	
		LcdFont((unsigned char *)nazanin19);
		Show_TextInTFT(GetMaxX-390,GetMaxY-110,20,60,"ʦ㇤",R2L,0); // toman
 	}
}

void showWellCome(void)
{
	SetColor(BLACK);  
	Line2(19,162,GetMaxX-15,162);
	SetColor(RGB(200,205,255));
  RectangleFill(19,40,GetMaxX-15,160, 0);
	SetFgColor(BLACK);
	LcdFont((unsigned char *)nazanin19);
	Show_TextInTFT(GetMaxX-180,GetMaxY-200,20,60,"ΦԠ£ϭϢ,R2L,0);	//khosh amadid 	
}
void showTransactionResult(unsigned char result,unsigned int pricee,unsigned int mandeEtebar,unsigned int ucc,unsigned char id,unsigned char gp)
{
	unsigned char str[100];
	refreshPage = 1;
	toggleLed = 0;
	switch (result)
	{
		case 0: 			  
			  SetFgColor(BLACK);
			  SetColor(RGB(200,205,255));
        RectangleFill(19,40,GetMaxX-15,160, 0);
			  LcdFont((unsigned char *)nazanin19);
			  Show_TextInTFT(GetMaxX-60,GetMaxY-180,20,60,"㦌揭 Ǒʢ,R2L,0);	//mojodi kart 
        Show_TextInTFT(GetMaxX-380,GetMaxY-230,20,60,"ʦ㇤",R2L,0); // toman		  
			  LcdFont((unsigned char *)Bnazanin48);
		    //LcdFont((unsigned char *)Btitr51);
	      sprintf(str,"%d",mandeEtebar);
			  //Show_TextInTFT(320,GetMaxY-240,0,25,(unsigned char *)str,R2L,1);  // mablagh
	    	Show_TextInTFT(320,GetMaxY-190,0,25,(unsigned char *)str,R2L,1);  // mablagh
		    if (gp == 254)
				{
					LcdFont((unsigned char *)nazanin19);	
					Show_TextInTFT(GetMaxX-60,GetMaxY-230,20,60,"·ь ǒ ȇҥ ңǤ𛐠咭䥠Ǐ ",R2L,0); // kharej az baze zamani  					
					return ;
				}
				else
				{
					LcdFont((unsigned char *)nazanin19);
					sprintf(str,"Ѧ堘Ǒʠ%d",gp); 	
					Show_TextInTFT(GetMaxX-60,GetMaxY-230,20,60,str,R2L,0); // goroh 
				}
				sprintf(str,"UC %d - ID %d",ucc,cardId);
		    LcdFont((unsigned char *)tahoma16);
		    Show_TextInTFT(GetMaxX-200,GetMaxY-230,20,60,str,R2L,0);	 
			break; //succeful
		
		case 3:
				SetColor(BLACK);
				SetColor(RGB(200,205,255));		
				RectangleFill(19,40,GetMaxX-15,160, 0);
				SetFgColor(BLACK);
				LcdFont((unsigned char *)nazanin19);
			  Show_TextInTFT(GetMaxX-60,GetMaxY-210,20,60,"㦌揭 Ǒʢ,R2L,0);	//mojodi kart 
				Show_TextInTFT(GetMaxX-130,GetMaxY-170,20,60,"㦌揭 ·ь ǒ ͏ ㌇Ң,R2L,0);	//mojodi kharej az had mojaz
		    Show_TextInTFT(GetMaxX-380,GetMaxY-230,20,60,"ʦ㇤",R2L,0); // toman
        LcdFont((unsigned char *)Bnazanin48);		
 	      sprintf(str,"%d",mandeEtebar);
			  Show_TextInTFT(320,GetMaxY-240,0,25,(unsigned char *)str,R2L,1);  // mablagh 
			  sprintf(str,"UC %d - ID %d",ucc,cardId);
		    LcdFont((unsigned char *)tahoma16);
		    Show_TextInTFT(GetMaxX-50,GetMaxY-240,20,60,str,R2L,0);	
			break; // OutOfRange
		
		case 10:
		case 9:		
				SetColor(BLACK);
				SetColor(RGB(200,205,255));		
				RectangleFill(19,40,GetMaxX-15,160, 0);
				SetFgColor(BLACK);
				LcdFont((unsigned char *)nazanin19);
 				Show_TextInTFT(GetMaxX-130,GetMaxY-170,20,60,"Ǒʠ㊚ហȥ 㑘Ҡ䭓ʢ,R2L,0);	//kart motaAlegh b in markaz nis  
		    sprintf(str,"Invalid UC [%d - %d]",ucc,cardId);
		    LcdFont((unsigned char *)tahoma16);
		    Show_TextInTFT(GetMaxX-160,GetMaxY-210,20,60,str,R2L,0);	//kart motaAlegh b in markaz nis  
			break; //invalid uc , id
		
		default:
				SetColor(BLACK);
				SetColor(RGB(200,205,255));		
				RectangleFill(19,40,GetMaxX-15,160, 0);
				SetFgColor(BLACK);
				LcdFont((unsigned char *)nazanin19);
		    sprintf(str,"ΘǠ- %d",result); // khata []
        Show_TextInTFT(GetMaxX-130,GetMaxY-170,20,60,str,R2L,0);
				Show_TextInTFT(GetMaxX-130,GetMaxY-210,20,60,"㌏χ ʡǔ 䭏",R2L,0);	//mojadadan talash konid    
				sprintf(str,"UC %d - ID %d",ucc,cardId);
		    LcdFont((unsigned char *)tahoma16);
		    Show_TextInTFT(GetMaxX-200,GetMaxY-230,20,60,str,R2L,0);	 		
			break;
	}
	
}
#endif
#ifdef ExecTransactionBCU
void IntToByte(unsigned int INT,unsigned char *BYTE)
{
  BYTE[0]=INT&0xFF;
  BYTE[1]=(INT>>8)&0xFF;
  BYTE[2]=(INT>>16)&0xFF;
  BYTE[3]=(INT>>24)&0xFF; 
}
#endif
unsigned char grgIsLeap(int year)
{
  return ((year%4) == 0 && ((year%100) != 0 || (year%400) == 0));
}
unsigned char hshIsLeap(int year)
{
  year = (year - 474) % 128;
  year = ((year >= 30) ? 0 : 29) + year;
  year = year - floor((double)year/33) - 1;
  return ((year % 4) == 0);
}
void gregorian_to_jalali(int grgYear, char grgMonth, char grgDay)
{
unsigned char grgLeap=0;
unsigned char hshLeap=0; 
int hshYear;
int hshMonth, hshDay, i;
long int hshElapsed;
long int grgElapsed;
int XmasToNorooz;
        
        hshYear = grgYear-621;
        grgLeap=grgIsLeap(grgYear);
        hshLeap=hshIsLeap(hshYear-1);                            
        grgElapsed = grgSumOfDays[(grgLeap ? 1:0)][grgMonth-1]+grgDay;
        XmasToNorooz = (hshLeap && grgLeap) ? 80 : 79;
	

	
	
	if (grgElapsed <= XmasToNorooz)
	{

		
		hshElapsed = grgElapsed+286;
		hshYear--;
		if (hshLeap && !grgLeap)
			hshElapsed++;
	}
	else
	{
		hshElapsed = grgElapsed - XmasToNorooz;
		hshLeap = hshIsLeap (hshYear);
	}
 
	for (i=1; i <= 12 ; i++)
	{
		if (hshSumOfDays [(hshLeap ? 1:0)][i] >= hshElapsed)
		{
			hshMonth = i;
			hshDay = hshElapsed - hshSumOfDays [(hshLeap ? 1:0)][i-1];
			break;
		}
	}
 
	Day = hshDay;
	Month = hshMonth;
	Year= hshYear;   
}
#if defined LCD
void showDateTime(void)
{
	uint8_t str[70];
	HAL_RTC_GetTime(&hrtc,&time,RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc,&date,RTC_FORMAT_BIN);
	gregorian_to_jalali((date.Year+2000),date.Month,date.Date);
	Year = Year - 1400;
	Hour = time.Hours;
	Min = time.Minutes;
	Sec = time.Seconds;
	oldMin = Min;
	SetColor(YELLOW);
  SetFgColor(BLACK);
	RectangleFill(0,300-10, GetMaxX, GetMaxY+100,0);	
	if ( (time.Hours>24) || (time.Minutes > 59) || (time.Seconds>59) || (date.Year > 40 ) || (date.Year < 21 ) || (date.Month > 12) || (date.Date > 32))
	{
		LcdFont((unsigned char *)nazanin19);
		Show_TextInTFT(GetMaxX-60,GetMaxY,30,25,"Θǭ Ӈڊ 栊Ǒ펢,R2L,1); // khataye saato tarikh
		TIMEDATEERRORJSON
		return;
	}
	LcdFont((unsigned char *)Yekan16);
	sprintf(str,"                 %2.2d : %2.2d                  %d/%2.2d/%2.2d                 ",Hour,Min,Year+1400,Month,Day);			
	Show_TextInTFT(GetMaxX-60,GetMaxY,30,25,(unsigned char *)str,R2L,1); // behtarin ja baraye saat 			
}

#endif
//=======================================================================
#if defined(ExecTransactionBCU)
void saveTransactionInMem(void)
{
	saveTransactionInMemFlag = !saveTransactionInMemFlag;
	if (!saveTransactionInMemFlag) send("{\"Receive\":{\"Status\":26},\"M\":{\"Type\":\"Not Save Transation In Internal Mem\"}}\n");
	if (saveTransactionInMemFlag)  send("{\"Receive\":{\"Status\":26},\"M\":{\"Type\":\"Save Transation In Internal Mem\"}}\n");	
	
}
//=======================================================================
void readVirtualAddress(void)
{
	unsigned char readMem[985];
	virtualAddress = 0;	
	_EE_SIZE = 1023;
	_EE_ADDR_INUSE   =  0x0801fc00;
	_EE_FLASH_BANK   = FLASH_BANK_1;
	_EE_PAGE_OR_SECTOR = 0;
	if (ee_read(38,985,readMem))
	{
		for (unsigned short idx = 30;idx<985;idx+=32)
			if ((readMem[idx] == 'J') && (readMem[idx+1] == 'G'))
			{
				if (virtualAddress < 38)
			  	virtualAddress = 38;
				else if (virtualAddress >= 38)
					virtualAddress += 32;
			}	
		TranactionWithOutSerial = virtualAddress/32;
	}	
}
//=====================================================================
unsigned char restoreConfigAndFormat(void)
{
	unsigned char restoreConfig[40];
	virtualAddress = 38;
	unsigned char indx = 0;
	TranactionWithOutSerial=0;
	restoreConfig[indx++] = 5;
	restoreConfig[indx++] = Config.UC>>24;
	restoreConfig[indx++] = Config.UC>>16;
	restoreConfig[indx++] = Config.UC>>8;
	restoreConfig[indx++] = Config.UC & 0xff;
	
	for (unsigned char i=0; i<10 ; i++)
	{
		restoreConfig[indx++] = GrouhPrice[i]>>8;
		restoreConfig[indx++] = GrouhPrice[i] & 0xff;
	}
	
	restoreConfig[indx++] = 0x64;
	
	restoreConfig[indx++] = Config.MinEtebar & 0xff;
	
	restoreConfig[indx++] = Config.MaxEtebar >> 24;
	restoreConfig[indx++] = Config.MaxEtebar >> 16;
	restoreConfig[indx++] = Config.MaxEtebar >> 8;
	restoreConfig[indx++] = Config.MaxEtebar  & 0xff;	
	
	restoreConfig[indx++] = Config.DeviceID >> 16;
	restoreConfig[indx++] = Config.DeviceID >> 8;
	restoreConfig[indx++] = Config.DeviceID  & 0xff;
	
	for (char cnt=0;cnt<34;cnt++)
		restoreConfig[indx] ^= restoreConfig[cnt];
	_EE_SIZE = 1023;
	_EE_ADDR_INUSE   =  0x0801fc00;
	_EE_FLASH_BANK   = FLASH_BANK_1;
	_EE_PAGE_OR_SECTOR = 0;
	ee_format(0);
	HAL_Delay(5);
	if (ee_write(0,indx+1,restoreConfig))
		return 1;
	else	
		return 0;
}
//==============================================================
void saveTransaction (unsigned char *Buf,unsigned char len)
{
	  if (virtualAddress<38)
			virtualAddress = 38;
		else if (virtualAddress >= 38)
			virtualAddress = virtualAddress + len;
		if (virtualAddress >= 970)
		{
			restoreConfigAndFormat();	
		}		
	 _EE_SIZE = 1023;
	 _EE_ADDR_INUSE   =  0x0801fc00;
	 _EE_FLASH_BANK   = FLASH_BANK_1;
	 _EE_PAGE_OR_SECTOR = 0;
	 ee_write(virtualAddress,len,Buf);
}
//==========================================================
char ProccesBusTicket(char Source)
{
unsigned char snr[5],TicketBuffer[64];
signed long int RemEtebar,PreEtebar;
signed int Etebar,Hedie;
unsigned int  DID,Grouh,UC;
unsigned char LastOP,OPIndex; 
unsigned long int ID=0,price;
unsigned char Status,hedieWriteFlag=0; 
unsigned char buf[100] , idx = 0;
int LastDevice; 
unsigned int tickk=0;
	
	HAL_RTC_GetTime(&hrtc,&time,RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc,&date,RTC_FORMAT_BIN);
	gregorian_to_jalali((date.Year+2000),date.Month,date.Date);
	Year = Year - 1400;
	Hour = time.Hours;
	Min = time.Minutes;
	Sec = time.Seconds;
	ID=0; 			
	UC = Config.UC;
	Grouh = rwGroupPrice(0,0,snr);	
	if (Grouh == 255)
	{
		send("{\"C\":{\"Status\":12}}\n");
		return 0;		
	}	
	if ((Grouh>9) && (Grouh !=254))
	{
		send("{\"C\":{\"Status\":10}}\n");
		return 0;
	}	
	if (sendTransactionFlag==0)
	{
		send("{\"C\":{\"Status\":11}}\n");
		HAL_Delay(1);
		send(buffTransaction);
		return 0;
	}
	if (Grouh == 254)
		price = GrouhPrice[0];
	else 
		price = (GrouhPrice[Grouh]);
//  price /= 100;
//  price = GrouhPrice[0] - price;
	//======================================	
//	if (GetCardEtebar(&Etebar)) { send("{\"C\":{\"Status\":13}}\n"); return 0; }
//	if (GetCardHedie(&Hedie))   { send("{\"C\":{\"Status\":14}}\n"); return 0; }
//	price = GrouhPrice[0];
//	if (Etebar < GrouhPrice[0])
//	{
//		if (Hedie+Etebar>=GrouhPrice[0])
//		{
//			Hedie = Hedie-(GrouhPrice[0]-Etebar);
//			price = Etebar;
//		}
//	}
	//=======================================
	Status=Old_ExecTransaction(2, &ID, &UC, price , Config.MinEtebar, Config.MaxEtebar, Config.DeviceID, &Etebar, &PreEtebar,&LastDevice,&LastOP,&OPIndex);
	if ((Status==1) || (Status==9))   
	{
	 ID=0;                             
	 Status=Old_ExecTransaction(2, &ID, &UC, price , Config.MinEtebar, Config.MaxEtebar, Config.DeviceID, &Etebar, &PreEtebar,&LastDevice,&LastOP,&OPIndex);
	 if ((Status==1) || (Status==9)) 
	 {
		ID=0;                             
		Status=Old_ExecTransaction(2, &ID, &UC, price , Config.MinEtebar, Config.MaxEtebar, Config.DeviceID, &Etebar, &PreEtebar,&LastDevice,&LastOP,&OPIndex);
	 }
	}			
	WDTR
//	if (Status == S_Success)
//	{
//		HediyeBuf[0]= Hedie&0xFF;
//		HediyeBuf[1]=(Hedie>>8)&0xFF;
//		HediyeBuf[2]=(Hedie>>16)&0xFF;	
//		HediyeBuf[3]=(Hedie>>24)&0xFF;	
//		HediyeBuf[4]=0xAB;	
//		HediyeBuf[5]=0xCD;
//		tickk = HAL_GetTick();
//		while (1)
//		{
//			if (HAL_GetTick()-tickk>3000) 
//			{
//				hedieWriteFlag =1;
//				break;
//			}
//			if (ISO14443_LoginE2(4,0x39))
//			{
//				if (ISO14443_SingleTagSelect(snr))
//				{
//					if (ISO14443_SingleTagSelect(snr))
//						continue;
//				}
//				if (ISO14443_LoginE2(4,0x39)) continue;
//			}                 
//			if (ISO14443_WriteBlock(18,HediyeBuf))  continue;	 	
//			break;
//		}
//	}
//	if (hedieWriteFlag == 1) 
//		sprintf(globalBuff,"{\"C\":{\"Status\":%d,\"UC\":%u,\"ID\":%u,\"Etebar\":%u,\"PreEtebar\":%u,\"Gp\":%u,\"Price\":%u,\"Hedie\":-1}}\n",Status,uc,cardId,Etebar,PreEtebar,Grouh,price);
//	else if (hedieWriteFlag == 0)
//	sprintf(globalBuff,"{\"C\":{\"Status\":%d,\"UC\":%u,\"ID\":%u,\"Etebar\":%u,\"PreEtebar\":%u,\"Gp\":%u,\"Price\":%u,\"Hedie\":%u}}\n",Status,uc,cardId,Etebar,PreEtebar,Grouh,price,Hedie);
	#if defined LCD
	showMainPrice(price,0);
	showTransactionResult(Status,price,Etebar,uc,cardId,Grouh);
	#endif
	
	sprintf(buffTransaction,"{\"C\":{\"Status\":%d,\"UC\":%u,\"ID\":%u,\"Etebar\":%u,\"PreEtebar\":%u,\"Gp\":%u,\"Price\":%u}}\n",Status,uc,cardId,Etebar,PreEtebar,Grouh,price);
	send(buffTransaction);
	
	buf[idx++] = deviceType;	
	buf[idx++] = cardId & 0xFF;
	buf[idx++] = ( cardId >> 8) & 0xFF;
	buf[idx++] = ( cardId >> 16) & 0xFF;
	buf[idx++] = ( cardId >> 24) & 0xFF;
	
	buf[idx++] =  uc & 0xFF;
	buf[idx++] = ( uc >> 8) & 0xFF;
	buf[idx++] = ( uc >> 16) & 0xFF;
	buf[idx++] = ( uc >> 24) & 0xFF;
	buf[idx++] = Status;
	
	buf[idx++] = 2; // type
	
	buf[idx++] =  Etebar & 0xFF;
	buf[idx++] = ( Etebar >> 8) & 0xFF;
	buf[idx++] = ( Etebar >> 16) & 0xFF;
	buf[idx++] = ( Etebar >> 24) & 0xFF;

	buf[idx++] =  PreEtebar & 0xFF;
	buf[idx++] = ( PreEtebar >> 8) & 0xFF;
	buf[idx++] = ( PreEtebar >> 16) & 0xFF;
	buf[idx++] = ( PreEtebar >> 24) & 0xFF;

	buf[idx++] =  price & 0xFF;
	buf[idx++] = ( price >> 8) & 0xFF;
	buf[idx++] = ( price >> 16) & 0xFF;
	buf[idx++] = ( price >> 24) & 0xFF;
	buf[idx++] = Year;
	buf[idx++] = Month;
	buf[idx++] = Day;
	buf[idx++] = Hour;
	buf[idx++] = Min;	
	buf[idx++] =  Grouh;
	if (	SPI_Flash_ReadID() == 0XEF16)
	{
		SaveTransactionRecord(buf,idx);
		SaveRingDetail();
	}		
	

	/*
	//memset(globalBuff,'\0',strlen(globalBuff));
  sendTransactionFlag	= 0;
	TicketBuffer[0]=2;      
	TicketBuffer[1]=ID&0xFF;
	TicketBuffer[2]=(ID>>8)&0xFF;
	TicketBuffer[3]=(ID>>16)&0xFF;
	TicketBuffer[4]=Status;//Config.UC;
	TicketBuffer[5]=Year-1400;
	TicketBuffer[6]=Month;
	TicketBuffer[7]=Day;
	TicketBuffer[8]=Hour;
	TicketBuffer[9]= Min;
	TicketBuffer[10]= Sec;
	TicketBuffer[11]= Grouh&0xFF;
	TicketBuffer[12]= (Grouh>>8)&0xFF;
	TicketBuffer[13]= price&0xFF;
	TicketBuffer[14]= (price>>8)&0xFF;
	IntToByte(PreEtebar,TicketBuffer+15);
	IntToByte(Etebar,TicketBuffer+19);
	TicketBuffer[23]=Config.DeviceID&0xFF;
	TicketBuffer[24]=(Config.DeviceID>>8)&0xFF;
	TicketBuffer[25]=(Config.DeviceID>>16)&0xFF;
	TicketBuffer[26]= uc&0xFF;
	TicketBuffer[27]=(uc>>8)&0xFF;
	TicketBuffer[28]=(uc>>16)&0xFF;
	TicketBuffer[29] = 0;
	for (unsigned char cnt=0;cnt<29;cnt++)
		TicketBuffer[29] ^= TicketBuffer[cnt]; 
	TicketBuffer[30] = 'J';
	TicketBuffer[31] = 'G';
	*/
//	if (saveTransactionInMemFlag)
//	if ((serialConnect == 2) || (serialConnect == 0))
//	{
//		TranactionWithOutSerial++;
//		saveTransaction(TicketBuffer,32);
//		if (TranactionWithOutSerial>=30) TranactionWithOutSerial = 1;
//	}		
}
#endif
//==================================================================
#if defined(ExecTransactionAPARK)
unsigned char ProccesAmusementParkTicket(void)
{
	#define EX_Takhfif        22
	#define EX_Tedadi         23
	#define EX_TakhfifMehman  24
	#define EX_TedadiMehman   25
	#define LSHIFT8(x) x<<=8;
	#define STATUS3  send("{\"C\":{\"Status\":3}}\n");
	#define STATUS11 send("{\"C\":{\"Status\":11}}\n");
	#define STATUS2  send("{\"C\":{\"Status\":2}}\n");
	unsigned int tick = 0;
	unsigned char snr[5],buf[16],BCC=0,ExOP=0,SubGroup=0,BreakFlag=0,IsCorrect=0,Status=0,year=0;
	unsigned char DisCount=0,SDateTime[5],EDateTime[10],HTransaction=0,LastOP,OPIndex,validHedieBuf=0;
	signed long int HEtebar=0,RemEtebar=0,PreEtebar=0,Etebar=0;
	unsigned long int ID=0;
	
	unsigned int CardEtebar=0,Price=0,Price2=0,HNPayment=0,UC = Config.UC;
	int LastDevice; 
	if (sendTransactionFlag==0)
	{
		STATUS11
		HAL_Delay(1);
		send(globalBuff);
		return 1;
	}	
	if ((Config.hTransaction > 3) || (Config.hTransaction <= 0))
	{
		flagRc522 = 0;
		STATUS3
		return 1;
	}
	tick = HAL_GetTick();
	while (1)
	{
		if (HAL_GetTick() - tick > 5000) {TIMEOUTJSON; return 1;} 
		WDTR
		if (ISO14443_SingleTagSelect(snr))
		{
			if (ISO14443_SingleTagSelect(snr))
				continue;
		} 
		if (ISO14443_LoginE2(4,0x39)) 
		{
			if (ISO14443_SingleTagSelect(snr))
			{
				if (ISO14443_SingleTagSelect(snr))
					continue; 
			}
			if (ISO14443_LoginE2(4,0x39))  
				continue; 
		}
		HEtebar=0;
		if (ISO14443_ReadValueBlock(16, &CardEtebar)) continue; 			
    if (ISO14443_ReadBlock(18, buf)) continue; //Hediye	
		if ((buf[4]==0xAB) && (buf[5]==0xCD))
		{
			HEtebar=buf[3];  LSHIFT8(HEtebar) 
			HEtebar+=buf[2]; LSHIFT8(HEtebar)
			HEtebar+=buf[1]; LSHIFT8(HEtebar)
			HEtebar+=buf[0];
			validHedieBuf = 1;
		}
		if ((buf[0]==0xFF) && (buf[1]==0xFF) && (buf[2]==0xFF) && (buf[3]==0xFF))
		{
			HEtebar=0;
			validHedieBuf = 0;
		}
		if (HEtebar > 1000000)
		{
			HEtebar=0;
			validHedieBuf = 0;
		}
		if (validHedieBuf==0)
			HEtebar=0;
		if (ISO14443_ReadBlock(17, buf)) 
    {
			if (ISO14443_LoginE2(4,0x39)) 
      {
				if (ISO14443_SingleTagSelect(snr))
				{
					if (ISO14443_SingleTagSelect(snr))
					  continue;
				}
			  if (ISO14443_LoginE2(4,0x39))  {  continue; }
		  }
	    if (ISO14443_ReadBlock(17,buf)) {  continue; }
    }
		BCC = 0;
		for (unsigned char i=0; i<15; i++)              
			BCC^=buf[i];	
		HAL_RTC_GetTime(&hrtc,&time,RTC_FORMAT_BIN);
		HAL_RTC_GetDate(&hrtc,&date,RTC_FORMAT_BIN);
		gregorian_to_jalali((date.Year+2000),date.Month,date.Date);
    year = Year;		
		Year = Year - 1400;
		Hour = time.Hours;
		Min = time.Minutes;
		Sec = time.Seconds;
    Price=GrouhPrice[0];		
    if (BCC==buf[15])
    {
			SubGroup=buf[0]; 
			SubGroup+=(buf[1]<<8);
			DisCount=buf[2];
			EDateTime[0]=buf[3];//Year%1300
			EDateTime[1]=buf[4];//Month
			EDateTime[2]=buf[5];//Day			
			SDateTime[3]=buf[6];// Start Hour
			SDateTime[4]=buf[7];// Start Min			
			EDateTime[3]=buf[8];// End Hour
			EDateTime[4]=buf[9];// End Min				
			SDateTime[0]=year - 1300 ;//Year%1300
			SDateTime[1]=Month;//Month
			SDateTime[2]=Day;//Day
			if (SubGroup==22)//tkhfifi
			{					
			  HEtebar = 0;
				if ((DisCount) && (DisCount<=100) && (SDateTime[0]<=EDateTime[0]) && (SDateTime[0]<200))
				{
					if (((year-1300)>=SDateTime[0]) && (((year-1300)<=EDateTime[0])))	
					{						
						if ((year-1300)>=SDateTime[0])
						{
							if ((year-1300)<=EDateTime[0])
							{
								if (SDateTime[0]==EDateTime[0])
								{
									if (Month>=SDateTime[1])
									{
										if (Month>EDateTime[1])  BreakFlag=1;
									}
									else BreakFlag=1;
								}
								else
								{
									if (((year-1300)== EDateTime[0]) && (Month>EDateTime[1])) BreakFlag=1;
								}
								if (!BreakFlag)
								{
									if ((Month==EDateTime[1]) && (Day>EDateTime[2])) BreakFlag=1;
								}
								if (!BreakFlag )
								{
									if (time.Hours<SDateTime[3]) BreakFlag=1;
									else if (time.Hours>EDateTime[3]) BreakFlag=1;
								}
								if (!BreakFlag )
								{
									if ((time.Hours==SDateTime[3]) && (time.Minutes<SDateTime[4])) BreakFlag=1;
									else if ((time.Hours==EDateTime[3]) && (time.Minutes>EDateTime[4])) BreakFlag=1;
								}
									
							}//if (Year<=EDateTime[0])
							else BreakFlag=1;
						}//if (Year>=SDateTime[0])
						else  {BreakFlag=1;}
					}
				}
				else BreakFlag=1;
				WDTR
				if (!BreakFlag) IsCorrect=123;						 
				if (IsCorrect==123) 
				{
					if (DisCount<=100)
					{
						Price=Price-((DisCount*Price)/100);
						Price2 = Price;
					}
				}
				else 
				{
					sprintf(globalBuff,"{\"C\":{\"Status\":123,\"ID\":%u,\"EY\":%d,\"EM\":%d,\"ED\":%d,\"SH\":%d,\"Sm\":%d,\"EH\":%d,\"Em\":%d,\"Em\":%d}}\n",cardId,EDateTime[0],EDateTime[1],EDateTime[2],SDateTime[3],SDateTime[4],EDateTime[3],EDateTime[4]);
					send(globalBuff);
					return(1);
				}
			} //if (SubGroup==22)
			if ((SubGroup==23) || (SubGroup==25))
				Price2=1;		
			switch (SubGroup)
			{
				case EX_Takhfif: ExOP=0x10; break;
				case EX_Tedadi: ExOP=0x20; break;
				case EX_TakhfifMehman: ExOP=0x30; break;
				case EX_TedadiMehman: ExOP=0x40; break;
				default: ExOP=0x00;
			}
      WDTR

			if (Config.hTransaction == 3) //  price + gift 
			{
				if (SubGroup==0)
				{
					Price2=Price;
					if (CardEtebar<Price)
					{						
						if (HEtebar+CardEtebar>=Price)
						{
							HEtebar=HEtebar-(Price-CardEtebar);
							Price2=CardEtebar;
						}
					}
				}
				if (SubGroup!=0)
				{
					if (CardEtebar<Price2)
					{						
						if (HEtebar+CardEtebar>=Price2)
						{
							HEtebar=HEtebar-(Price2-CardEtebar);
							Price2=CardEtebar;
						}
					}
				}
			}
			if (Config.hTransaction == 2) // only gift
			{	
				if (SubGroup==0)
				{
					if (HEtebar >= Price)
					{
						HEtebar = HEtebar - Price;
					}
					else
					{
						sprintf(globalBuff,"{\"C\":{\"Status\":21,\"UC\":%u,\"ID\":%u,\"gt\":%u,\"model\":%d,\"Price\":%u,\"HTran\":%d,\"validH\":%d}}\n",uc,cardId,HEtebar,SubGroup,Price,HTransaction,validHedieBuf);
						send(globalBuff);
						return 1;
					}
				}
				if (SubGroup!=0)
				{
					if (HEtebar >= Price2)
					{
						HEtebar = HEtebar - Price2;
					}
					else
					{
						sprintf(globalBuff,"{\"C\":{\"Status\":21,\"UC\":%u,\"ID\":%u,\"gt\":%u,\"model\":%d,\"Price\":%u,\"HTran\":%d,\"validH\":%d}}\n",uc,cardId,HEtebar,SubGroup,Price2,HTransaction,validHedieBuf);
						send(globalBuff);
						return 1;
					}
				}				
			}
			if (Config.hTransaction == 1)
			{
				if (SubGroup==0)
					Price2 = GrouhPrice[0];
			}
			HediyeBuf[0]=HEtebar&0xFF;
			HediyeBuf[1]=(HEtebar>>8)&0xFF;
			HediyeBuf[2]=(HEtebar>>16)&0xFF;	
			HediyeBuf[3]=(HEtebar>>24)&0xFF;	
			HediyeBuf[4]=0xAB;	
			HediyeBuf[5]=0xCD;
			if (Config.hTransaction != 2)
			{	
				ID=0;                             
				UC=Config.UC;
				Status=Old_ExecTransaction(ExOP|2, &ID, &UC, Price2, Config.MinEtebar, Config.MaxEtebar, Config.DeviceID, &Etebar, &PreEtebar,&LastDevice,&LastOP,&OPIndex);
				if ((Status==1) || (Status==13) || (Status==9)) 
				{
					Status=Old_ExecTransaction(ExOP|2, &ID, &UC, Price2, Config.MinEtebar, Config.MaxEtebar, Config.DeviceID, &Etebar, &PreEtebar,&LastDevice,&LastOP,&OPIndex);
					if ((Status==1) || (Status==13) || (Status==9)) 
					{
						Status=Old_ExecTransaction(ExOP|2, &ID, &UC, Price2, Config.MinEtebar, Config.MaxEtebar, Config.DeviceID, &Etebar, &PreEtebar,&LastDevice,&LastOP,&OPIndex);
					}
				}				
			}
			if (((Config.hTransaction == 3) && (Status == 0)) || (Config.hTransaction == 2))
			{
				if (SubGroup != 22)
				{
					HediyeBuf[0]=HEtebar&0xFF;
					HediyeBuf[1]=(HEtebar>>8)&0xFF;
					HediyeBuf[2]=(HEtebar>>16)&0xFF;	
					HediyeBuf[3]=(HEtebar>>24)&0xFF;	
					HediyeBuf[4]=0xAB;	
					HediyeBuf[5]=0xCD;
					tick = HAL_GetTick(); 
					while (1)
					{
						WDTR
						if (HAL_GetTick() - tick > 2000) 
						{
							break;
						}						
						if (ISO14443_SingleTagSelect(snr))
						{
							if (ISO14443_SingleTagSelect(snr))
								continue;
						} 
						if (ISO14443_LoginE2(4,0x39)) 
						{
							if (ISO14443_SingleTagSelect(snr))
							{
								if (ISO14443_SingleTagSelect(snr))
									continue; 
							}
							if (ISO14443_LoginE2(4,0x39))  
								continue; 
						}
						if (ISO14443_WriteBlock(18,HediyeBuf))  continue;	
						HTransaction = 1;
						break;					
					}//while (1)
				}
			}				
			if (Config.hTransaction !=2)
			{
				sprintf(globalBuff,"{\"C\":{\"Status\":%d,\"UC\":%u,\"ID\":%u,\"Etebar\":%u,\"gt\":%u,\"model\":%d,\"Price\":%u,\"HTran\":%d,\"validH\":%d}}\n",Status,uc,cardId,Etebar,HEtebar,SubGroup,Price2,HTransaction,validHedieBuf);
				send(globalBuff);
				return 0;
			}
			if (Config.hTransaction == 2)
			{
				sprintf(globalBuff,"{\"C\":{\"Status\":20,\"UC\":%u,\"ID\":%u,\"gt\":%u,\"model\":%d,\"Price\":%u,\"HTran\":%d,\"validH\":%d}}\n",uc,cardId,HEtebar,SubGroup,Price2,HTransaction,validHedieBuf);
				send(globalBuff);
				return 0;
			}
    }// if (BCC==buf[15])
		else 
		{
			STATUS2
			return 1;
		}				
	}//while (1)
	return 1;
}
#endif
//==================================================================
unsigned char ProcessCard(unsigned char snr[10])
{
/*
 unsigned char TicketBuffer[16],buf[32],RetValue=0;
 unsigned char Status, ShowMsg, BCC,m;
 unsigned int  i, Grouh;
 unsigned int UC;
 unsigned char str[20];
 signed long int Credit;
 
 if (ISO14443_LoginE2(0,0x10)) return(0);
 if (ISO14443_ReadBlock(1,buf)) return(0); 
 if (ISO14443_LoginE2(5,0x37))
   {
    if (ISO14443_SingleTagSelect(snr))
      {
       if (ISO14443_SingleTagSelect(snr))
          return(0);
      }
    if (ISO14443_LoginE2(5,0x37))  return(0);
   }

 if (ISO14443_ReadBlock(22, buf)) 
   {
    if (ISO14443_LoginE2(5,0x37)) 
      {
       if (ISO14443_SingleTagSelect(snr))
         {
          if (ISO14443_SingleTagSelect(snr))
             return(0);
         }
       if (ISO14443_LoginE2(5,0x37))  return(0);
      }
   if (ISO14443_ReadBlock(22,buf)) return(0);
  }

 
 Grouh=buf[1];  Grouh<<=8;
 Grouh+=buf[0];
  

   
 if (Grouh>20) Grouh=0; 
 if (Grouh==2)
	{
	//     if (DriverCardReadTimeout) return(1);
	//    RegisterDriverCard();
		// baraye vorodo khoroj ranande, byd ezafe shavad
	 return(1);
	} 
*/
 #if defined(ExecTransactionBCU)
 ProccesBusTicket(1);
 #endif
 #if defined(ExecTransactionAPARK)
 ProccesAmusementParkTicket();
 #endif

 
 return(0);
}
//--------------------------------------------------
unsigned char saveConfig(unsigned int len)
{
	 _EE_SIZE = 64;
	 _EE_ADDR_INUSE   =  0x0801fc00;
	 _EE_FLASH_BANK   = FLASH_BANK_1;
	 _EE_PAGE_OR_SECTOR = 0;
	if (ee_init())
	{
		HAL_Delay(10); 
		ee_format(0); 
		if (ee_write(0,len,fromAndroid))
		{
			HAL_Delay(5);
			send("{\"SaveConfig\":{\"Status\":26}}\n");
			HAL_Delay(5); 	
			virtualAddress = 0;
		}
		else send("{\"SaveConfig\":{\"Status\":19}}\n");
	}	
	else send("{\"SaveConfig\":{\"Status\":19}}\n");	
}
//----------------------------------------------------------
unsigned char loadConfig(unsigned int len)
{
	 _EE_SIZE = 64;
	 _EE_ADDR_INUSE   =  0x0801fc00;
	 _EE_FLASH_BANK   = FLASH_BANK_1;
	 _EE_PAGE_OR_SECTOR = 0;	
	unsigned char readConfig[len+2];
	flagRc522 = 0;  
	if (SPI_Flash_ReadID() == W25Q64)
	{
		SPI_Flash_Read(readConfig,addConfig,len);	
	}
	else
	{
		if (ee_init())
		{
			HAL_Delay(10);  
			if (ee_read(0,len,readConfig))
			{
				HAL_Delay(5);
				//send("{\"ReadConfig \":{\"Status\":26}\n");
				HAL_Delay(5); 			
			}
			else {send("{\"ReadConfig\":{\"Status\":19}\n"); flagRc522 = 0;  return 1;}
		}	
		else {send("{\"ReadConfig\":{\"Status\":19}\n");	flagRc522 = 0;  return 1;}
	}
	crc=0;
	
	#if defined(ExecTransactionBCU)
	for (char cnt=0;cnt<34;cnt++)
		crc ^= readConfig[cnt];
	if ((crc != readConfig[34]) ||  (readConfig[25] != 0x64))
	{ 
		send("{\"ReadConfig\":{\"Status\":1}}\n");   
		flagRc522 = 0;  
		crc=0; 
	}
	else if ((crc == readConfig[34]) ||  (readConfig[25] == 0x64) )
	{
		Config.UC =  (readConfig[1]<<24) + (readConfig[2]<<16) + (readConfig[3]<<8) + readConfig[4]; // 000018c9  6345 
		GrouhPrice[0] = (readConfig[5]<<8) + readConfig[6];  //0000 0       
		GrouhPrice[1] = (readConfig[7]<<8) + readConfig[8];  //03E8 1000
		GrouhPrice[2] = (readConfig[9]<<8) + readConfig[10]; //07d0 2000 
		GrouhPrice[3] = (readConfig[11]<<8) + readConfig[12];//0bb8 3000      
		GrouhPrice[4] = (readConfig[13]<<8) + readConfig[14];//1234 4660  
		GrouhPrice[5] = (readConfig[15]<<8) + readConfig[16];//5678 22136
		GrouhPrice[6] = (readConfig[17]<<8) + readConfig[18];//aaaa 43690
		GrouhPrice[7] = (readConfig[19]<<8) + readConfig[20];//abcd 43981
		GrouhPrice[8] = (readConfig[21]<<8) + readConfig[22];//1596 5526
		GrouhPrice[9] = (readConfig[23]<<8) + readConfig[24];//1020 4128 		
		Config.MinEtebar =  readConfig[26];
		Config.MaxEtebar = (readConfig[27]<<24) + (readConfig[28]<<16) + (readConfig[29]<<8) + readConfig[30];
		Config.DeviceID  = (readConfig[31]<<16) + (readConfig[32]<<8) + readConfig[33];
		send("{\"ReadConfig\":{\"Status\":0}}\n");
		flagRc522 = 1;
		crc=0;
	}	
	#endif
	
	#if defined(ExecTransactionAPARK)
	for (char cnt=0;cnt<35;cnt++)
		crc ^= readConfig[cnt];
	if ((crc != readConfig[35]) ||  (readConfig[25] != 0x64) || (readConfig[34] > 3) || (readConfig[34] <= 0) )
	{ 
		send("{\"ReadConfig\":{\"Status\":1}}\n");   
		flagRc522 = 0;  
		crc=0; 
	}
	else if ((crc == readConfig[35]) ||  (readConfig[25] == 0x64))
	{
		Config.UC =  (readConfig[1]<<24) + (readConfig[2]<<16) + (readConfig[3]<<8) + readConfig[4]; // 000018c9  6345 
		GrouhPrice[0] = (readConfig[5]<<8) + readConfig[6];  //0000 0       
		GrouhPrice[1] = (readConfig[7]<<8) + readConfig[8];  //03E8 1000
		GrouhPrice[2] = (readConfig[9]<<8) + readConfig[10]; //07d0 2000 
		GrouhPrice[3] = (readConfig[11]<<8) + readConfig[12];//0bb8 3000      
		GrouhPrice[4] = (readConfig[13]<<8) + readConfig[14];//1234 4660  
		GrouhPrice[5] = (readConfig[15]<<8) + readConfig[16];//5678 22136
		GrouhPrice[6] = (readConfig[17]<<8) + readConfig[18];//aaaa 43690
		GrouhPrice[7] = (readConfig[19]<<8) + readConfig[20];//abcd 43981
		GrouhPrice[8] = (readConfig[21]<<8) + readConfig[22];//1596 5526
		GrouhPrice[9] = (readConfig[23]<<8) + readConfig[24];//1020 4128 		
		Config.MinEtebar =  readConfig[26];
		Config.MaxEtebar = (readConfig[27]<<24) + (readConfig[28]<<16) + (readConfig[29]<<8) + readConfig[30];
		Config.DeviceID  = (readConfig[31]<<16) + (readConfig[32]<<8) + readConfig[33];
		Config.hTransaction = readConfig[34];
		send("{\"ReadConfig\":{\"Status\":0}}\n");
		flagRc522 = 1;
		crc=0;
	}	
	#endif
}
//=========================================================================
void receiveConfig(void)
{
	//05 000018c9 0000 03E8 07d0 0bb8 1234 5678 aaaa abcd 1596 1020 64  00 ffffffff 000010 f2
	crc=0;
	#if defined(ExecTransactionBCU)
	for (char cnt=0;cnt<34;cnt++)
		crc ^= fromAndroid[cnt];
	
	if ((crc != fromAndroid[34]) ||  (fromAndroid[25] != 0x64))
	{ 
		send("{\"Receive\":{\"Status\":19}}\n");  
		flagRc522 = 0;  
		crc=0; 
		memset(fromAndroid,'\0',sizeof(fromAndroid));
		return;
	}
	else if ((crc == fromAndroid[34]) ||  (fromAndroid[25] == 0x64))
	{
		Config.UC =  (fromAndroid[1]<<24) + (fromAndroid[2]<<16) + (fromAndroid[3]<<8) + fromAndroid[4]; // 000018c9  6345 
		GrouhPrice[0] = (fromAndroid[5]<<8) + fromAndroid[6];  //0000 0       
		GrouhPrice[1] = (fromAndroid[7]<<8) + fromAndroid[8];  //03E8 1000
		GrouhPrice[2] = (fromAndroid[9]<<8) + fromAndroid[10]; //07d0 2000 
		GrouhPrice[3] = (fromAndroid[11]<<8) + fromAndroid[12];//0bb8 3000      
		GrouhPrice[4] = (fromAndroid[13]<<8) + fromAndroid[14];//1234 4660  
		GrouhPrice[5] = (fromAndroid[15]<<8) + fromAndroid[16];//5678 22136
		GrouhPrice[6] = (fromAndroid[17]<<8) + fromAndroid[18];//aaaa 43690
		GrouhPrice[7] = (fromAndroid[19]<<8) + fromAndroid[20];//abcd 43981
		GrouhPrice[8] = (fromAndroid[21]<<8) + fromAndroid[22];//1596 5526
		GrouhPrice[9] = (fromAndroid[23]<<8) + fromAndroid[24];//1020 4128 		
		Config.MinEtebar = (fromAndroid[26]);
		Config.MaxEtebar = (fromAndroid[27]<<24) + (fromAndroid[28]<<16) + (fromAndroid[29]<<8) + fromAndroid[30];
		Config.DeviceID  = (fromAndroid[31]<<16) + (fromAndroid[32]<<8) +  fromAndroid[33];
		if (SPI_Flash_ReadID() == W25Q64)
		{
			if (!(SPI_Flash_Write(fromAndroid,addConfig,35)))
				send("{\"SaveConfig\":{\"Status\":26}}\n");
			else 
				send("{\"SaveConfig\":{\"Status\":19}}\n");
		}
		else
			saveConfig(35);
		#if defined LCD
		showMainPrice(GrouhPrice[0],1);
		#endif
		memset(fromAndroid,'\0',sizeof(fromAndroid));
		flagRc522 = 1;
		crc=0;
	}	
	#endif
	
	#if defined(ExecTransactionAPARK)
	for (char cnt=0;cnt<35;cnt++)
		crc ^= fromAndroid[cnt];
	
	if ((crc != fromAndroid[35]) ||  (fromAndroid[25] != 0x64) || (fromAndroid[34] > 3) || (fromAndroid[34] <= 0))
	{ 
		send("{\"Receive\":{\"Status\":19}}\n");  
		flagRc522 = 0;  
		crc=0; 
		memset(fromAndroid,'\0',sizeof(fromAndroid));
		return;
	}
	else if ((crc == fromAndroid[35]) ||  (fromAndroid[25] == 0x64))
	{
		Config.UC =  (fromAndroid[1]<<24) + (fromAndroid[2]<<16) + (fromAndroid[3]<<8) + fromAndroid[4]; // 000018c9  6345 
		GrouhPrice[0] = (fromAndroid[5]<<8) + fromAndroid[6];  //0000 0       
		GrouhPrice[1] = (fromAndroid[7]<<8) + fromAndroid[8];  //03E8 1000
		GrouhPrice[2] = (fromAndroid[9]<<8) + fromAndroid[10]; //07d0 2000 
		GrouhPrice[3] = (fromAndroid[11]<<8) + fromAndroid[12];//0bb8 3000      
		GrouhPrice[4] = (fromAndroid[13]<<8) + fromAndroid[14];//1234 4660  
		GrouhPrice[5] = (fromAndroid[15]<<8) + fromAndroid[16];//5678 22136
		GrouhPrice[6] = (fromAndroid[17]<<8) + fromAndroid[18];//aaaa 43690
		GrouhPrice[7] = (fromAndroid[19]<<8) + fromAndroid[20];//abcd 43981
		GrouhPrice[8] = (fromAndroid[21]<<8) + fromAndroid[22];//1596 5526
		GrouhPrice[9] = (fromAndroid[23]<<8) + fromAndroid[24];//1020 4128 		
		Config.MinEtebar = (fromAndroid[26]);
		Config.MaxEtebar = (fromAndroid[27]<<24) + (fromAndroid[28]<<16) + (fromAndroid[29]<<8) + fromAndroid[30];
		Config.DeviceID  = (fromAndroid[31]<<16) + (fromAndroid[32]<<8) +  fromAndroid[33];
		Config.hTransaction = fromAndroid[34];
		saveConfig(36);
		memset(fromAndroid,'\0',sizeof(fromAndroid));
		flagRc522 = 1;
		crc=0;
	}	
	#endif	
}	

//=========================================================================

void sendConfig(void)
{
	unsigned char buf[900];
//	sprintf(buf,"{\"SC\":{\"UC\":%u,\"GrouhPrice0\":%u,\"GrouhPrice1\":%u,\"GrouhPrice2\":%u,\"GrouhPrice3\":%u,\"GrouhPrice4\":%u,\"GrouhPrice5\":%u,\"GrouhPrice6\":%u,\"GrouhPrice7\":%u,\"GrouhPrice8\":%u,\"GrouhPrice9\":%u}}\n",Config.UC,GrouhPrice[0],GrouhPrice[1],GrouhPrice[2],GrouhPrice[3],GrouhPrice[4],GrouhPrice[5],GrouhPrice[6],GrouhPrice[7],GrouhPrice[8],GrouhPrice[9]);
	if (flagRc522)
	{
		#if defined(ExecTransactionBCU)
		sprintf(buf,"{\"SC\":{\"UC\":%u,\"P\":%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,\"Max\":%u,\"Min\":%d,\"DID\":%u}}\n",Config.UC,GrouhPrice[0],GrouhPrice[1],GrouhPrice[2],GrouhPrice[3],GrouhPrice[4],GrouhPrice[5],GrouhPrice[6],GrouhPrice[7],GrouhPrice[8],GrouhPrice[9],Config.MaxEtebar,Config.MinEtebar,Config.DeviceID);
		//sprintf(buf,"{\"SC\":{\"UC\":%u,\"Price\":%u,\"Max\":%u,\"Min\":%u,\"DID\":%u}}\n",Config.UC,GrouhPrice[0],Config.MaxEtebar,Config.MinEtebar,Config.DeviceID);
	  #endif
		#if defined(ExecTransactionAPARK)
		sprintf(buf,"{\"SC\":{\"UC\":%u,\"P\":%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,\"Max\":%u,\"Min\":%u,\"DID\":%u,\"Htra\":%d}}\n",Config.UC,GrouhPrice[0],GrouhPrice[1],GrouhPrice[2],GrouhPrice[3],GrouhPrice[4],GrouhPrice[5],GrouhPrice[6],GrouhPrice[7],GrouhPrice[8],GrouhPrice[9],Config.MaxEtebar,Config.MinEtebar,Config.DeviceID,Config.hTransaction);
	  #endif		
		send(buf);
	}
	else if (flagRc522==0)
		send("{\"Config\":{\"Status\":30}}\n");
	memset(fromAndroid,'\0',sizeof(fromAndroid));
}

//=========================================================================

void processCardFlag(void)
{
	send("{\"Receive\":{\"Status\":26},\"C\":{\"Type\":\"ProcessCard\"}}\n");
	memset(fromAndroid,'\0',10);
	GpFlag=1; 
}

void processCardFlagOff(void)
{
	send("{\"Receive\":{\"Status\":26},\"C\":{\"Type\":\"ProcessCardOff\"}}\n");
	memset(fromAndroid,'\0',10);
	GpFlag=0; 
}

void processReciveTrans(void)
{
	send("{\"Receive\":{\"Status\":26},\"C\":{\"Type\":\"receiveAckTran\"}}\n");
	memset(fromAndroid,'\0',10);
	sendTransactionFlag = 1; 
}
//=========================================================================
#if defined(ExecTransactionBCU)
void deleteTransactionInMem(void)
{
	if (restoreConfigAndFormat())
		send("{\"Receive\":{\"Status\":26},\"Trasaction\":{\"Type\":\"MemDelete\",\"Status\":0}}\n");
	else
		send("{\"Receive\":{\"Status\":26},\"Trasaction\":{\"Type\":\"MemDelete\",\"Status\":1}}\n");	
}
//==================================================================================
void sendTransactionFromMemory(void)
{
//	struct{		
//		unsigned int  ID;
//		unsigned int  UC;
//		unsigned char status;		
//		unsigned long int etebar;
//		unsigned long int preEtebar;
//		unsigned char gp;
//		unsigned short price;
//		unsigned char y,m,d,h,mm,row;
//	}Transaction;
//	if (!saveTransactionInMemFlag)
//	{
//		send("{\"Receive\":{\"Status\":26},\"M\":{\"Type\":\"Not Save Transation In Internal Mem\"}}\n");
//		return;
//	}
	
	unsigned char row = 0;	
	unsigned char readMem[985];
	unsigned char bufTrasaction[50];
	_EE_SIZE = 1023;
	_EE_ADDR_INUSE   =  0x0801fc00;
	_EE_FLASH_BANK   = FLASH_BANK_1;
	_EE_PAGE_OR_SECTOR = 0;
	if (ee_read(38,985,readMem))
	{
		for (unsigned short idx = 30;idx<985;idx+=32)
			if ((readMem[idx] == 'J') && (readMem[idx+1] == 'G'))
			{
				row ++;
				sprintf(bufTrasaction,"Row=%d*",row);
			  send(bufTrasaction);
				sprintf(bufTrasaction,"Status=%d*",readMem[idx-26]);
			  send(bufTrasaction);
				sprintf(bufTrasaction,"UC=%u*",(readMem[idx-2] << 16 ) + (readMem[idx-3] << 8 ) +  readMem[idx-4]);
			  send(bufTrasaction);	
				sprintf(bufTrasaction,"ID=%u*",(readMem[idx-27] << 16 ) + (readMem[idx-28] << 8 ) +  readMem[idx-29]);
			  send(bufTrasaction);	
				sprintf(bufTrasaction,"Etebar=%u*",(readMem[idx-8] << 24 ) + (readMem[idx-9] << 16 ) + (readMem[idx-10] << 8 ) +  readMem[idx-11]);
			  send(bufTrasaction);
				sprintf(bufTrasaction,"PreEtebar=%u*",(readMem[idx-12] << 24 ) + (readMem[idx-13] << 16 ) + (readMem[idx-14] << 8 ) +  readMem[idx-15]);
			  send(bufTrasaction);				
				sprintf(bufTrasaction,"Gp=%d*",readMem[idx-19]);
			  send(bufTrasaction);
				sprintf(bufTrasaction,"Price=%d*",(readMem[idx-16] << 8 ) +  readMem[idx-17]);
			  send(bufTrasaction);	
				sprintf(bufTrasaction,"Y=%d*",readMem[idx-25]);
			  send(bufTrasaction);	
				sprintf(bufTrasaction,"m=%d*",readMem[idx-24]);
			  send(bufTrasaction);
				sprintf(bufTrasaction,"D=%d*",readMem[idx-23]);
			  send(bufTrasaction);	
				sprintf(bufTrasaction,"H=%d*",readMem[idx-22]);
			  send(bufTrasaction);	
				sprintf(bufTrasaction,"M=%d\n",readMem[idx-21]);
			  send(bufTrasaction);
				HAL_Delay(200);
//				Transaction.ID = (readMem[idx-27] << 16 ) + (readMem[idx-28] << 8 ) +  readMem[idx-29];
//				Transaction.status = readMem[idx-26];
//				Transaction.UC = (readMem[idx-2] << 16 ) + (readMem[idx-3] << 8 ) +  readMem[idx-4];
//				Transaction.etebar = (readMem[idx-8] << 24 ) + (readMem[idx-9] << 16 ) + (readMem[idx-10] << 8 ) +  readMem[idx-11];
//				Transaction.preEtebar = (readMem[idx-12] << 24 ) + (readMem[idx-13] << 16 ) + (readMem[idx-14] << 8 ) +  readMem[idx-15];
//				Transaction.price = (readMem[idx-16] << 8 ) +  readMem[idx-17];
//				Transaction.gp =  readMem[idx-19];
//				Transaction.y = readMem[idx-25];
//				Transaction.m = readMem[idx-24];
//				Transaction.d = readMem[idx-23];				
//				Transaction.h = readMem[idx-22];
//				Transaction.mm = readMem[idx-21];				
				WDTR
			}
		deleteTransactionInMem();
	}
	else send("{\"Receive\":{\"Status\":26},\"Trasaction\":{\"Type\":\"Mem\",\"Status\":19}}\n");
	
//	send("{\"Receive\":{\"Status\":26},\"Trasaction\":{\"Type\":\"Mem\",\"Status\":%d}}\n",value);
}
/*
void ExecTransactionWithNet(void)
{
	//4d 00001388 000003e8 64  0000000000000000 010611 102920 56
	
	unsigned long int id = 0,Id=0,Uc=0;
	unsigned char ser[10],bufRead[16];
	unsigned int Li=0;		
	if (flagRc522 == 0)
	{
		send("{\"Config\":{\"Status\":30}}\n"); // no config 
		return ;
	}
	crc=0;
	GpFlag = 0;
	for (char cnt=0;cnt<24;cnt++)
		crc ^= fromAndroid[cnt];	
	if ((crc != fromAndroid[24])  ||  (fromAndroid[9] != 0x64))
	{ 
		send("{\"Receive\":{\"Status\":19}}\n");  
		crc=0; 
		execTransactionFlag = 0;
		memset(fromAndroid,'\0',sizeof(fromAndroid));
		return;
	}
	else if ((crc == fromAndroid[24]) &&  (fromAndroid[9] == 0x64) )
	{
		send("{\"Receive\":{\"Status\":26},\"C\":{\"Type\":\"ExecTransactionWithNet\"}}\n");		
		id = (fromAndroid[1]<<24) + (fromAndroid[2]<<16) + (fromAndroid[3]<<8) + fromAndroid[4];        // 00 00 13 88 --> 5000
		payMent = (fromAndroid[5]<<24) + (fromAndroid[6]<<16) + (fromAndroid[7]<<8) + fromAndroid[8];   // 00 00 03 e8 --> 1000
    //10 -- 17 reseve 		
		Year = fromAndroid[18]; //01
		Month = fromAndroid[19]; // 06
		Day = fromAndroid[20]; // 11 
		Hour = fromAndroid[21]; // 10
		Min = fromAndroid[22]; //29
		Sec = fromAndroid[23]; // 20 
		MINEtebar = Config.MinEtebar;
		MAXEtebar = Config.MaxEtebar;
		deviceID = Config.DeviceID;		
		memset(fromAndroid,'\0',sizeof(fromAndroid));
		crc=0;		
		#ifdef finger 
	  rxBufferHeadPos = 0;
		#endif
		CDCReceiveLen = 0;
		Li = HAL_GetTick(); 
		while (1)
		{
			if (HAL_GetTick() - Li > 10000) { TIMEOUTJSON return;}
			#if  defined finger 
			if (rxBufferHeadPos)
			#else
			if (CDCReceiveLen)
			#endif
				if ((fromAndroid[0] == 'C') || (fromAndroid[0]=='c'))
				{
					send("{\"Receive\":{\"Status\":26},\"C\":{\"Type\":\"Cansel\"}}\n"); 
					rxBufferHeadPos = 0;
					return;
				}
			WDTR
			if (ISO14443_SingleTagSelect(ser))
			{
				if (ISO14443_SingleTagSelect(ser))
					continue;
			} 
			if (ISO14443_LoginE2(0,0x10))
			{
			if (ISO14443_SingleTagSelect(ser))
			{
				if (ISO14443_SingleTagSelect(ser))
					continue;
			}
			if (ISO14443_LoginE2(0,0x10)) {continue;}
			}
			if (!ISO14443_LoginE2(0,0x10))
			{
				if (!ISO14443_ReadBlock(1,bufRead))
				{
					if ((bufRead[6] == 0xAB) && (bufRead[7] == 0) && (bufRead[15] == 0))//edited 
					{
						Uc =  bufRead[0];
						Uc += (bufRead[1]<<8);
						Id = bufRead[2];
						Id +=(bufRead[3]<<8);
						Id +=(bufRead[4]<<16);
						Id +=(bufRead[5]<<24);
						if ((Uc & 0x8000)==0x8000)
						{
							Uc = Uc & 0x7FFF;
							Uc = Uc ^ 0x6A29;
							Id = Id ^ 0xB2A6;
						} 
						if (id != Id)
						{
							send("{\"C\":{\"Status\":2}}\n");	
							return;
						}
						IncrementCreditToCard(payMent,0);
						return ;					
					} else continue;
				} else continue;
			} else continue;
		}
	}		
}
*/
#endif
#endif
#endif

#if (deviceType==AutoPark)
unsigned char saveConfig(unsigned int len)
{
	 _EE_SIZE = 64;
	 _EE_ADDR_INUSE   =  0x0801fc00;
	 _EE_FLASH_BANK   = FLASH_BANK_1;
	 _EE_PAGE_OR_SECTOR = 0;
	
	if (SPI_Flash_ReadID() == W25Q64)
	{
		if (SPI_Flash_Write(config,addConfig,len))
			send("{\"SaveConfig\":{\"Status\":19}}\n");
		else
			send("{\"SaveConfig\":{\"Status\":26}}\n");
	}
	else
	{
		if (ee_init())
		{
			HAL_Delay(10); 
			ee_format(0); 
			if (ee_write(0,len,config))
			{
				HAL_Delay(5);
				send("{\"SaveConfig\":{\"Status\":26}}\n");
				HAL_Delay(5); 			
			}
			else send("{\"SaveConfig\":{\"Status\":19}}\n");
		}	
		else send("{\"SaveConfig\":{\"Status\":19}}\n");	
	}
	configFlag = 1;
}

unsigned char readConfig(unsigned int len)
{
	 _EE_SIZE = 64;
	 _EE_ADDR_INUSE   =  0x0801fc00;
	 _EE_FLASH_BANK   = FLASH_BANK_1;
	 _EE_PAGE_OR_SECTOR = 0;	

	unsigned char readConfig[5];
	
	if (SPI_Flash_ReadID() == W25Q64)
	{
		SPI_Flash_Read(readConfig,addConfig,len);
	}
	else
	{
		if (ee_init())
		{
			HAL_Delay(10);  
			if (ee_read(0,len,readConfig))
			{
				HAL_Delay(5);
				//send("{\"ReadConfig \":{\"Status\":26}\n");			
			}
			else {send("{\"ReadConfig\":{\"Status\":19}}\n"); configFlag = 0;  return 1;}
		}	
		else {send("{\"ReadConfig\":{\"Status\":19}}\n");	configFlag = 0;  return 1;}
	}
	
	if ((readConfig[0] == 0x10) && (readConfig[2] == (0x10^readConfig[1])))
	{
		configFlag = 1;
		EnteringExitingMode = readConfig[1] & 0x0f ; 
		EnteringExitingModeMember = readConfig[1] & 0xf0; 
	}	
}
void sendDeviceMode(void)
{
	// 0 --> vorod , 1 --> khoroj , 2 --> vorod khoroj 
	if (configFlag)
	{
		switch (EnteringExitingModeMember | EnteringExitingMode)
		{
			case 0x00: send("{\"Receive\":{\"Status\":26},\"DeviceMode\":0x00}\n"); break;
			case 0x11: send("{\"Receive\":{\"Status\":26},\"DeviceMode\":0x11}\n"); break;
			case 0x21: send("{\"Receive\":{\"Status\":26},\"DeviceMode\":0x21}\n"); break;
			case 0x20: send("{\"Receive\":{\"Status\":26},\"DeviceMode\":0x20}\n"); break;
			default: send("{\"Receive\":{\"Status\":19},\"DeviceMode\":255}\n"); break;		
		}
	}
	else if (configFlag==0)
		send("{\"Config\":{\"Status\":30}}\n");
	memset(fromAndroid,'\0',10);
}
void DeviceEnteringExitingMode(unsigned char mode)
{
	/*
	0x00 --> kolan vorod 
	0x11 --> kolan khoroj
	0x12 --> aza vorod khoroj , vorod
	0x02 --> aza vorod khoroj , khoroj 
	*/
	if ( (mode == 0x00)  || (mode == 0x11) || (mode == 0x21) || (mode ==0x20))		
	{
		EnteringExitingMode = (mode & 0x0f); // 0 --> vorod , 1 --> khoroj 
		EnteringExitingModeMember = ( mode & 0xf0) ; //0--> vorod , 0x10 --> khoroj , 0x20 --> vorod khoroj 
		config[0] = 0x10;
		config[1] = mode;
		config[2] = (0x10) ^ (mode);
		saveConfig(3);
		configFlag=1;
//		send("{\"Receive\":{\"Status\":26},\"C\":{\"Type\":\"DeviceMode\"}}\n");
	}	 
	else
		send("{\"Receive\":{\"Status\":19},\"C\":{\"Type\":\"DeviceMode\"}}\n");
	
	memset(fromAndroid,'\0',sizeof(fromAndroid));
}

void openGate(void)
{
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_10,1);
	HAL_Delay(1000);
	WDTR
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_10,0);
}

void Invalid_ValidUser(void)
{
	send("{\"Receive\":{\"Status\":26}}\n");
	reciveAck = 0; 
}
//*********************************************************************************
unsigned char WriteExitMode(void)
{
	send("{\"Receive\":{\"Status\":26},\"C\":{\"Type\":\"WriteExitMode\"}}\n");
	unsigned short Login_First_Arg = 5,Login_Second_Arg = 0x37,Read_Write_First_Arg=22,Login_Successfully=1;
  unsigned int tick;
  unsigned char Source_Buffer[32];
	unsigned char snr[5];	
	tick = HAL_GetTick();
	while (Login_Successfully)
	{
		if (HAL_GetTick() - tick > 5000 ) {send("{\"C\":{\"Status\":1}}\n");  return 255;}
		WDTR
		if (ISO14443_SingleTagSelect(snr))
		{
			if (ISO14443_SingleTagSelect(snr))
				continue;
		} 
		if (ISO14443_LoginE2(Login_First_Arg,Login_Second_Arg))
		{
			if (ISO14443_SingleTagSelect(snr))
			{
				if (ISO14443_SingleTagSelect(snr))
					continue;
			} 
			if (ISO14443_LoginE2(Login_First_Arg,Login_Second_Arg)) continue;
		}
		if (!ISO14443_LoginE2(Login_First_Arg,Login_Second_Arg))
		{
			WDTR
			Source_Buffer[0] = 0;
			Source_Buffer[1] = 0;
			Source_Buffer[2] = 0;
			Source_Buffer[3] = 0;								
			Source_Buffer[4] = 0;
			Source_Buffer[5] = 0;
			Source_Buffer[6] = 0;
			Source_Buffer[7] = 0;
			Source_Buffer[8] = 0;
			Source_Buffer[9] = 0;
			Source_Buffer[10] = 0;
			Source_Buffer[11] = 0;
			Source_Buffer[12] = 0;
			Source_Buffer[13] = 0;
			Source_Buffer[14] = 0;
			Source_Buffer[15] = 0;
			if (!ISO14443_WriteBlock(Read_Write_First_Arg, Source_Buffer))
			{
				send("{\"C\":{\"Status\":0,\"Type\":\"WriteExitMode\"}}\n");
				return 0;
			}
			else { send("{\"C\":{\"Status\":2}}\n"); return 255;} 
		}
		else { send("{\"C\":{\"Status\":3}}\n"); return 255;}
	}
	
}
//***********************************************************
void GetCardDispenserStatusProccess(void);


//=========================================================
unsigned char ProcessCard(unsigned char *snr)
{
	unsigned short Login_First_Arg = 5,Login_Second_Arg = 0x37,Read_Write_First_Arg=22,Login_Successfully=1;
  unsigned int tick;
  unsigned char Source_Buffer[32],Readed_BCC=0;
	unsigned char gp=0;
	unsigned char buff[20],idx = 0;
	switch (EnteringExitingMode)
	{
		case 0:		 // biron az parking va dar hale vorod b parking 	
			HAL_RTC_GetTime(&hrtc,&time,RTC_FORMAT_BIN);
			HAL_RTC_GetDate(&hrtc,&date,RTC_FORMAT_BIN);
			Hour = time.Hours;
			Min = time.Minutes;
			Sec = time.Seconds;
			tick = HAL_GetTick();
			while (Login_Successfully)
			{
				Readed_BCC = 0;
				if (enableReader2==0)
					gpOfCard = 200; // goroh kasai k ozv hastan va bedone dokme card migiran  
				if (HAL_GetTick() - tick > 4000) 
				{ 
					send("{\"C\":{\"Status\":1}}\n"); 
					if (enableReader2==1)
						Dispenser_To_Recyclebin(); 
					return 255;
				}
				WDTR
				if (ISO14443_SingleTagSelect(snr))
				{
					if (ISO14443_SingleTagSelect(snr))
						continue;
				} 
				
				if (ISO14443_LoginE2(Login_First_Arg,Login_Second_Arg))
				{
					if (ISO14443_SingleTagSelect(snr))
					{
						if (ISO14443_SingleTagSelect(snr))
							continue;
					} 
					if (ISO14443_LoginE2(Login_First_Arg,Login_Second_Arg)) continue;
				}
				
				if (!ISO14443_LoginE2(Login_First_Arg,Login_Second_Arg))
				{
					if (!ISO14443_ReadBlock(Read_Write_First_Arg, Source_Buffer))
					{
						WDTR
						for (unsigned char i=0;i<15;i++) Readed_BCC^=Source_Buffer[i];
						if (Readed_BCC == Source_Buffer[15])
						{
							WDTR
							if (Source_Buffer[13] == 0)
							{
								Readed_BCC = 0;
								Source_Buffer[0]= 0;
								Source_Buffer[1]= 0;
								Source_Buffer[2]= 0;
								Source_Buffer[3]= 0;								
								Source_Buffer[4] = date.Year;
								Source_Buffer[5] = date.Month;
								Source_Buffer[6] = date.Date;
								Source_Buffer[7] = time.Hours;
								Source_Buffer[8] = time.Minutes;
								Source_Buffer[9] = time.Seconds;
								Source_Buffer[10] = 1;
								Source_Buffer[11] = 0;
								Source_Buffer[12] = 0;
								Source_Buffer[13] = 1;
								Source_Buffer[14] = gpOfCard;
								for (unsigned char index=0; index<15; index++){Readed_BCC^=Source_Buffer[index];}
								Source_Buffer[15]=Readed_BCC;
								if (!ISO14443_WriteBlock(Read_Write_First_Arg, Source_Buffer))
								{
									sprintf(globalBuff,"{\"C\":{\"ID\":%d,\"UC\":%d,\"Gp\":%d,\"Y\":%d,\"M\":%d,\"D\":%d,\"H\":%d,\"m\":%d,\"S\":%d,\"EEM\":%d}}\n",cardId,uc,gpOfCard,date.Year,date.Month,date.Date,time.Hours,time.Minutes,time.Seconds,EnteringExitingMode); // ID card , ersal UC card
									//sprintf(globalBuff,"{\"C\":{\"ID\":%d,\"UC\":%d}}\n",cardId,uc); // ID card , ersal UC card
									send(globalBuff);  //ersal ID card
									if (enableReader2)
									{
										if (Dispenser_To_Customer() != 100)
										{
											HAL_Delay(1000);
											if (Dispenser_To_Customer() != 100)
											{
												HAL_Delay(1000);
												if (Dispenser_To_Customer() != 100)
												{
													HAL_Delay(100);
													GetCardDispenserStatusProccess();
												}													
											}
										}
									}
									buff[idx++] = deviceType;
									buff[idx++] = cardId & 0xFF;
									buff[idx++] = ( cardId >> 8) & 0xFF;
									buff[idx++] = ( cardId >> 16) & 0xFF;
									buff[idx++] = ( cardId >> 24) & 0xFF;
									buff[idx++] =  uc & 0xFF;
									buff[idx++] = ( uc >> 8) & 0xFF;
									buff[idx++] = ( uc >> 16) & 0xFF;
									buff[idx++] = ( uc >> 24) & 0xFF;
									buff[idx++] = gpOfCard;
									buff[idx++] = date.Year;
									buff[idx++] = date.Month;
									buff[idx++] = date.Date;
									buff[idx++] = time.Hours;
									buff[idx++] = time.Minutes;
									buff[idx++] = time.Seconds;
									buff[idx++] = EnteringExitingMode;
									if (	memID == 0XEF16)
									{
										SaveTransactionRecord(buff,idx);
										SaveRingDetail();
									} 
									WDTR
									HAL_Delay(1);									
									openGate(); 
									return 0;
								}
								else { if (enableReader2) Dispenser_To_Recyclebin(); send("{\"C\":{\"Status\":2}}\n"); return 255;} 
							}
							else 
								{ 
									if (enableReader2) 
										Dispenser_To_Recyclebin(); 								
									sprintf(globalBuff,"{\"C\":{\"ID\":%d,\"UC\":%d,\"Status\":3}}\n",cardId,uc,etebar);	
									send(globalBuff); 
									return 255;
								}
						}
						else { if (enableReader2) Dispenser_To_Recyclebin(); send("{\"C\":{\"Status\":4}}\n"); return 255;}
					}
					else { if (enableReader2) Dispenser_To_Recyclebin(); send("{\"C\":{\"Status\":5}}\n"); return 255;}				
				}
				else { if (enableReader2) Dispenser_To_Recyclebin(); send("{\"C\":{\"Status\":6}}\n"); return 255;}				
			}
			break;
				
		case 1: // dakhele parking va dar hale khoroj az parking
			tick = HAL_GetTick();
			while (Login_Successfully)
			{
				Readed_BCC = 0;
				if (HAL_GetTick() - tick > 4000 ) 
				{
					send("{\"C\":{\"Status\":1}}\n");
					if (enableReader2==1)
						CardAcceptorMoveToFront(); 
					return 255;
				}
				WDTR
				if (ISO14443_SingleTagSelect(snr))
				{
					if (ISO14443_SingleTagSelect(snr))
						continue;
				}
				if (ISO14443_LoginE2(Login_First_Arg,Login_Second_Arg))
				{
					if (ISO14443_SingleTagSelect(snr))
					{
						if (ISO14443_SingleTagSelect(snr))
							continue;
					} 
					if (ISO14443_LoginE2(Login_First_Arg,Login_Second_Arg)) continue;
				}				
				if (!ISO14443_LoginE2(Login_First_Arg,Login_Second_Arg))
				{
					if (!ISO14443_ReadBlock(Read_Write_First_Arg, Source_Buffer))
					{
						WDTR
						for (unsigned char i=0;i<15;i++) Readed_BCC^=Source_Buffer[i];
						if (Readed_BCC == Source_Buffer[15])
						{
							WDTR
							if (Source_Buffer[13] == 1)
							{
								Readed_BCC = 0;
								Source_Buffer[0]= 0;
								Source_Buffer[1]= 0;
								Source_Buffer[2]= 0;
								Source_Buffer[3]= 0;								
								date.Year  = Source_Buffer[4];
								date.Month = Source_Buffer[5];
								date.Date  = Source_Buffer[6];
								time.Hours = Source_Buffer[7];
								time.Minutes=Source_Buffer[8];
								time.Seconds=Source_Buffer[9];
								Source_Buffer[10] = 0;
								Source_Buffer[11] = 0;
								Source_Buffer[12] = 0;
								Source_Buffer[13] = 0;
								gp = Source_Buffer[14];
								for (unsigned char index=0; index<15; index++){Readed_BCC^=Source_Buffer[index];}
								Source_Buffer[15]=Readed_BCC;
								if (!ISO14443_WriteBlock(Read_Write_First_Arg, Source_Buffer))
								{
									sprintf(globalBuff,"{\"C\":{\"ID\":%d,\"UC\":%d,\"Gp\":%d,\"Y\":%d,\"M\":%d,\"D\":%d,\"H\":%d,\"m\":%d,\"S\":%d,\"EEM\":%d}}\n",cardId,uc,gp,date.Year,date.Month,date.Date,time.Hours,time.Minutes,time.Seconds,EnteringExitingMode); // ID card , ersal UC card
									send(globalBuff);  //ersal ID card
									if (enableReader2)
										reciveAck = 1;
									buff[idx++] = deviceType;
									buff[idx++] = cardId & 0xFF;
									buff[idx++] = ( cardId >> 8) & 0xFF;
									buff[idx++] = ( cardId >> 16) & 0xFF;
									buff[idx++] = ( cardId >> 24) & 0xFF;
									buff[idx++] =  uc & 0xFF;
									buff[idx++] = ( uc >> 8) & 0xFF;
									buff[idx++] = ( uc >> 16) & 0xFF;
									buff[idx++] = ( uc >> 24) & 0xFF;
									buff[idx++] = gpOfCard;
									buff[idx++] = date.Year;
									buff[idx++] = date.Month;
									buff[idx++] = date.Date;
									buff[idx++] = time.Hours;
									buff[idx++] = time.Minutes;
									buff[idx++] = time.Seconds;
									buff[idx++] = EnteringExitingMode;
									if (	memID == 0XEF16)
									{
										SaveTransactionRecord(buff,idx);
										SaveRingDetail();
									}  
									// edited 
  								//	CardAcceptorMoveToRear();
  								//	openGate();
									return 0;
								}
								else { send("{\"C\":{\"Status\":2}}\n"); return 255;} 
							}
							else 
								{ 									
									sprintf(globalBuff,"{\"C\":{\"ID\":%d,\"UC\":%d,\"Status\":3}}\n",cardId,uc,etebar);	
									send(globalBuff); 
									if (enableReader2) 
										CardAcceptorMoveToRear(); 
									return 255;
								}
						}
						else { send("{\"C\":{\"Status\":4}}\n"); return 255;}
					}
					else { send("{\"C\":{\"Status\":5}}\n"); return 255;}				
				}
				else { send("{\"C\":{\"Status\":6}}\n"); return 255;}				
			}
			break;
			
		default: send("{\"Config\":{\"Status\":30}}\n"); break;
	}		
}
//**********************************************************************************
void exitEnterWithDis(void)
{	
	enableReader2 = 1;	
	MFRC522_Init();
	unsigned char serNumReader2[5];
	unsigned char buffReader2[50];
	
//	#ifndef    NoDispenser


												if (EnteringExitingMode==0)
												{
																			if (giveCardWithKey==0)
																			{
																				gpOfCard = fromAndroid[1]; // goroh kasai k ba touch kart migiran moshakhas mishe 
																				send("{\"Receive\":{\"Status\":26}}\n");
																			}
																			else if (giveCardWithKey)
																			{
																				send("{\"Receive\":{\"Status\":85}}\n");
																				gpOfCard = 0;  // goroh kasai k ba key card migiran 
																			}
																							
																			if (Get_CardDispenserStatus() == 100)
																			{
																				HAL_Delay(50);
																				if (Dispenser_To_Reader()!=100)
																				{
																					HAL_Delay(50);
																					if (Dispenser_To_Reader()!=100)
																					{
																						GetCardDispenserStatusProccess();
																						enableReader2 = 0;
																						return;					
																					}
																				}
																			}
																			else 
																			{
																				GetCardDispenserStatusProccess();
																				enableReader2 = 0;
																				return;
																			}
													
												}
//#endif
	HAL_Delay(5);
	unsigned int ti = HAL_GetTick();
	while (1)
	{		
																													if ( HAL_GetTick() - ti > 5000 ) 
																													{
																														enableReader2 = 0; 
//																														#ifndef    NoDispenser
																														send("{\"C\":{\"Status\":7}}\n"); 
																														if (EnteringExitingMode==1)
																															CardAcceptorMoveToFront(); 
																														if (EnteringExitingMode==0)
																															Dispenser_To_Recyclebin();
//																														#endif
																														return;
																													}  
											if (ISO14443_SingleTagSelect(serNumReader2))
											{
												if (ISO14443_SingleTagSelect(serNumReader2))
													 continue;
											}         
											if (ISO14443_LoginE2(0,0x10))
											{
												if (ISO14443_SingleTagSelect(serNumReader2))
												{
													if (ISO14443_SingleTagSelect(serNumReader2))
														 continue;
												}
												if (ISO14443_LoginE2(0,0x10)) {continue;}
											}   
											if (!ISO14443_ReadBlock(1,buffReader2)) 
											{
												for (unsigned char i=0; i<16; i++)
													dataCard[i]=buffReader2[i];
												crcCard=0xC9;
												for (unsigned char i=0; i<15; i++)
													crcCard^=dataCard[i];
												 if (crcCard==dataCard[15])
												 {      
													for (unsigned char i=0; i<15; i++) 
													{
														dataCard[i]^=KeyTable[serNumReader2[i%4]/((i/4)+1)]^serNumReader2[3-(i%4)];
													}
													crcCard=0xE2;  
													for (unsigned char i=0; i<14; i++)
														crcCard^=dataCard[i];
													if (crcCard!=dataCard[14])  
													{
														for (unsigned char i=0; i<16; i++)
															dataCard[i]=buffReader2[i];
														crcCard=dataCard[15]+1;  
													}
													else
													{  
														 uc=dataCard[1]; uc<<=8;
														 uc+=dataCard[0];
														 cardId=dataCard[5];  cardId<<=8;
														 cardId+=dataCard[4]; cardId<<=8;
														 cardId+=dataCard[3]; cardId<<=8;
														 cardId+=dataCard[2];
														 crcCard=dataCard[15];  
													}
												}
												if (crcCard!=dataCard[15])		
												{	
													if ( (buffReader2[6] == 0xab) && (buffReader2[7] == 0))
													{
														uc=buffReader2[0];
														uc+=(buffReader2[1]<<8);
														cardId=buffReader2[2];
														cardId+=(buffReader2[3]<<8);
														cardId+=(buffReader2[4]<<16);
														cardId+=(buffReader2[5]<<24);
														if ((uc & 0x8000)==0x8000)
														{
															uc = uc & 0x7FFF;
															uc = uc ^ 0x6A29;
															cardId = cardId ^ 0xB2A6;
														}
													} else continue;
												}
												if (configFlag)
												{
													ProcessCard(serNumReader2);
													enableReader2 = 0;
													return;
												}
												else if (configFlag==0)
													send("{\"Config\":{\"Status\":30}}\n");
												enableReader2 = 0;
									//			}
									//			else continue;
											}
	}
	enableReader2 = 0;
}
//=================
void CardAcceptorMoveToRearProccess(void)
{
	if (EnteringExitingMode==0)
		return;	
	send("{\"Receive\":{\"Status\":26}}\n");		
	HAL_Delay(5);
	switch (CardAcceptorMoveToRear())
	{
		case 0x59: send("{\"ACC\":{\"Status\":89}}\n"); break; // success
		case 0x4E: send("{\"ACC\":{\"Status\":78}}\n"); break; // fail
		case 0x45: send("{\"ACC\":{\"Status\":69}}\n"); break; // no card in reader 
		case 0x57: send("{\"ACC\":{\"Status\":87}}\n"); break; // card is not in the right position 			
	}
	memset(fromAndroid,0,10);
	CDCReceiveLen = 0;
}
//************************************************************
void CardAcceptorMoveToFrontProccess(void)
{
	if (EnteringExitingMode==0)
		return;	
	send("{\"Receive\":{\"Status\":26}}\n");		
	HAL_Delay(5);
	switch (CardAcceptorMoveToFront())
	{
		case 0x59: send("{\"ACC\":{\"Status\":89}}\n"); break;
		case 0x4E: send("{\"ACC\":{\"Status\":78}}\n"); break;
		case 0x45: send("{\"ACC\":{\"Status\":69}}\n"); break;
		case 0x57: send("{\"ACC\":{\"Status\":87}}\n"); break;			
	}
	memset(fromAndroid,0,10);
	CDCReceiveLen = 0;
}
//********************************************************
void CardAcceptorStatusProccess(void)
{
	if (EnteringExitingMode==0)
		return;	
	send("{\"Receive\":{\"Status\":26}}\n");		
	HAL_Delay(5);
	switch (Get_CardAcceptorStatus())
	{
		case 0x46: send("{\"ACC\":{\"Status\":70}}\n"); break;
		case 0x47: send("{\"ACC\":{\"Status\":71}}\n"); break;
		case 0x48: send("{\"ACC\":{\"Status\":72}}\n"); break; // card is eject from the front side 
		case 0x49: send("{\"ACC\":{\"Status\":73}}\n"); break;
		case 0x4A: send("{\"ACC\":{\"Status\":74}}\n"); break;	
		case 0x4B: send("{\"ACC\":{\"Status\":75}}\n"); break;
		case 0x4C: send("{\"ACC\":{\"Status\":76}}\n"); break;
		case 0x4D: send("{\"ACC\":{\"Status\":77}}\n"); break; // card is ejected from the rear side 
		case 0x4E: send("{\"ACC\":{\"Status\":78}}\n"); break; // no card 	
	}	
	memset(fromAndroid,0,10);
	CDCReceiveLen = 0;
}

void DispenserToReaderProccess(void)
{
	if (EnteringExitingMode==1)
		return;	
	send("{\"Receive\":{\"Status\":26}}\n");		
	HAL_Delay(5);
	switch (Dispenser_To_Reader())
	{
		case 100: send("{\"DIS\":{\"Status\":100}}\n"); break;
		case 105: send("{\"DIS\":{\"Status\":105}}\n"); break;
	}
	memset(fromAndroid,0,10);
	CDCReceiveLen = 0;
}

void GetCardDispenserStatusProccess(void)
{
	if (EnteringExitingMode==1)
		return;	
//	send("{\"Receive\":{\"Status\":26}}\n");		
//	HAL_Delay(5);
//send("{\"DIS\":{\"Status\":100}}\n");
	
	switch (Get_CardDispenserStatus())
	{
		case 100: send("{\"DIS\":{\"Status\":100}}\n");break; // ready - ok
		case 111: send("{\"DIS\":{\"Status\":111}}\n");break; // Recycling bin is full
		case 101: send("{\"DIS\":{\"Status\":101}}\n");break; // empty
		case 102: send("{\"DIS\":{\"Status\":102}}\n");break; // card in taking away place
		case 103: send("{\"DIS\":{\"Status\":103}}\n");break; // card in reader area
		case 104: send("{\"DIS\":{\"Status\":104}}\n");break; // not ready for pre-read before issuing
		case 105: send("{\"DIS\":{\"Status\":105}}\n");break; 
		case 106: send("{\"DIS\":{\"Status\":106}}\n");break; // blocking
		case 107: send("{\"DIS\":{\"Status\":107}}\n");break; // the machine in issuing card 
		case 108: send("{\"DIS\":{\"Status\":108}}\n");break; // error in issuing
		case 109: send("{\"DIS\":{\"Status\":109}}\n");break; // the machine in returning back the card
		case 110: send("{\"DIS\":{\"Status\":110}}\n");break;	// low card
		default:  send("{\"DIS\":{\"Status\":112}}\n");break; 
	}
	memset(fromAndroid,0,10);
	CDCReceiveLen = 0;
}
void DispenserToCustomerProccess(void)
{
	if (EnteringExitingMode==1)
		return;	
	send("{\"Receive\":{\"Status\":26}}\n");		
	HAL_Delay(5);
	switch (Dispenser_To_Customer())
	{
		case 100: send("{\"DIS\":{\"Status\":100}}\n"); break;
		case 105: send("{\"DIS\":{\"Status\":105}}\n");break;
	}
	memset(fromAndroid,0,10);
	CDCReceiveLen = 0;
}

void DispenserToRecyclebinProccess(void)
{
	if (EnteringExitingMode==1)
		return;	
	send("{\"Receive\":{\"Status\":26}}\n");		
	HAL_Delay(5);
	switch (Dispenser_To_Recyclebin())
	{
		case 100: send("{\"DIS\":{\"Status\":100}}\n"); break;
		case 105: send("{\"DIS\":{\"Status\":105}}\n");break;
	}
	memset(fromAndroid,0,10);
	CDCReceiveLen = 0;
}

#endif
#if defined AutoStart
void tabletOn(void)
{
	if (fromAndroid[1] == 0xaa)
	{
		send("{\"Receive\":{\"Status\":26}}\n");		
		POWERTAB(1);		
	}
}
void tabletOff(void)
{
	if (fromAndroid[1] == 0xff)
	{
		send("{\"Receive\":{\"Status\":26}}\n");		
		POWERTAB(0);		
	}
}
#endif
#ifdef remote
unsigned char remoteCode(void)
{
	unsigned int counter=0;
	CDCReceiveLen = fromAndroid[0] = 0;
	send("{\"Receive\":{\"Status\":26}}\n");
	counter = HAL_GetTick();
	while (1)
	{
		if (HAL_GetTick() - counter > 15000) { send("{\"Remote\":{\"Status\":1}}\n"); return 0;} //timeout 15sec 
		WDTR
		if (CDCReceiveLen)
		{
			if ((fromAndroid[0] == 'C') || (fromAndroid[0] == 'c'))
			{
				CDCReceiveLen = fromAndroid[0] = 0;
				send("{\"Remote\":{\"Status\":2}}\n"); 
				return 0;
			}
			CDCReceiveLen = 0;
		}
		if (DetectTruePacket)
		{
			DetectTruePacket = 0;
			tickLearnCode = 0;
			enableIRQ = 1;
			sprintf(globalBuff,"{\"Remote\":{\"Code\":%u}}\n",learncode);
			send(globalBuff);
			HAL_Delay(1000);
			DetectTruePacket = 0;
			tickLearnCode = 0;
			enableIRQ = 1;
			return 0;
		}		
	}
	return 0;
}
#endif

#ifdef virtualRemote
void delay_us (uint16_t us)
{
	TIM1->CNT=0;  
	while (TIM1->CNT < us); 
}
void One(void)
{
	GPIOB->BSRR = GPIO_PIN_9;
	delay_us(300); // Htime
	GPIOB->BSRR = (uint32_t)GPIO_PIN_9 << 16u;
	delay_us(100); // Ltime
}
void Zero(void)
{
	GPIOB->BSRR = GPIO_PIN_9;
	delay_us(100); // Htime
	GPIOB->BSRR = (uint32_t)GPIO_PIN_9 << 16u;
	delay_us(300); // Ltime
}
void genrateVirtualRemoteCode(void)
{
	char binaryNum[20] = {0},binaryNumKey[4]={0};
	unsigned int num = 0 , key = 0;
	unsigned char i = 0;
	num = (fromAndroid[1] << 16) + (fromAndroid[2] << 8) + fromAndroid[3];
	key = fromAndroid[4];
	if ( (key > 0x0f) || (key == 0) || (num > 0x0ffff0)) 
	{
		INVALIDJson
		return;
	}
	send("{\"Receive\":{\"Status\":26}}\n");
  for (i = 0 ; num > 0 ; ){
      binaryNum[i++] = num % 2;
      num /= 2;
  }
	
	for (i = 0 ; key > 0 ; ){
      binaryNumKey[i++] = key % 2;
      key /= 2;
  }
	for (i = 0 ; i < 20 ; i++)
	{
		if (binaryNum[19-i] == 0)
			Zero();
		if (binaryNum[19-i] == 1)
			One();
	}	
	for (i = 0 ; i < 4 ; i++)
	{
		if (binaryNumKey[3-i] == 0)
			Zero();
		if (binaryNumKey[3-i] == 1)
			One();
	}	  
	GPIOB->BSRR = GPIO_PIN_9; 
	delay_us(1872); // Htime 12 * 156
	GPIOB->BSRR = (uint32_t)GPIO_PIN_9 << 16u;
	delay_us(6084); // Ltime 39 * 156 		 
	GPIOB->BSRR = GPIO_PIN_9; 
	fromAndroid[0] = fromAndroid[1] = fromAndroid[2] = fromAndroid[3] = fromAndroid[4] = 0;
}
#endif

#if (deviceType == AmusementPark) && defined (cardAccDis)
char cardDeliveryProcess(void)
{
	send("{\"Receive\":{\"Status\":26}}\n");
	switch (Get_CardDispenserStatus())
	{
		case 100:
			HAL_Delay(100);
			if (Dispenser_To_Reader() == 100)
			{
				HAL_Delay(500);
				if (Dispenser_To_Customer() == 100)
				{
					send("{\"DIS\":{\"Status\":100}}\n");
					return 0;
				}
			}
			else
			{
				HAL_Delay(500);
				if (Dispenser_To_Reader() == 100)
				{
					HAL_Delay(500);
					if (Dispenser_To_Customer() == 100)
					{
						send("{\"DIS\":{\"Status\":100}}\n");
						return 0;
					}
				}
				else
				{
					send("{\"DIS\":{\"Status\":105}}\n");
					return 1;
				}
			}
			break; // ready - ok
		case 111: send("{\"DIS\":{\"Status\":111}}\n");break; // Recycling bin is full
		case 101: send("{\"DIS\":{\"Status\":101}}\n");break; // empty
		case 102: send("{\"DIS\":{\"Status\":102}}\n");break; // card in taking away place
		case 103: send("{\"DIS\":{\"Status\":103}}\n");break; // card in reader area
		case 104: send("{\"DIS\":{\"Status\":104}}\n");break; // not ready for pre-read before issuing
		case 105: send("{\"DIS\":{\"Status\":105}}\n");break; 
		case 106: send("{\"DIS\":{\"Status\":106}}\n");break; // blocking
		case 107: send("{\"DIS\":{\"Status\":107}}\n");break; // the machine in issuing card 
		case 108: send("{\"DIS\":{\"Status\":108}}\n");break; // error in issuing
		case 109: send("{\"DIS\":{\"Status\":109}}\n");break; // the machine in returning back the card
		case 110: send("{\"DIS\":{\"Status\":110}}\n");break;	// low card
		default:  send("{\"DIS\":{\"Status\":112}}\n");break; 
	}
	fromAndroid[0] = 0;
}

#endif
#if defined(ExecTransaction)
	#define CANSELLJSON send("{\"Receive\":{\"Status\":26},\"C\":{\"Type\":\"Cansel\"}}\n");
void ziroGiftCheck(void) {	
	unsigned int ti = 0;
	unsigned char snr[6],buf[100];
	unsigned long int UCC=0,cardIDD=0;
	crc = 0;
	for (unsigned char cnt = 0;cnt<9;cnt++)
		crc ^= fromAndroid[cnt];
	if (crc != fromAndroid[9])
	{
		INVALIDJson;
		rxAndroid = 1;
	  memset(fromAndroid,'\0',sizeof(fromAndroid));
		return;
	}
	ziroEtebarID = (fromAndroid[1]<<24) + (fromAndroid[2]<<16) + (fromAndroid[3]<<8) + fromAndroid[4];
	ziroEtebarUC = (fromAndroid[5]<<24) + (fromAndroid[6]<<16) + (fromAndroid[7]<<8) + fromAndroid[8];
	HediyeBuf[0]=0;
	HediyeBuf[1]=0;
  HediyeBuf[2]=0;	
  HediyeBuf[3]=0;	
  HediyeBuf[4]=0xAB;	
  HediyeBuf[5]=0xCD;
  CDCReceiveLen = 0;	
	ti =HAL_GetTick();
	while (1)
	{
		WDTR
		if (HAL_GetTick() - ti > 16000) 
		{
			TIMEOUTJSON
			break;
		}
		if (CDCReceiveLen)
		{
			if ((fromAndroid[0] == 'c') || (fromAndroid[0] == 'C'))
			{
				CANSELLJSON;					
				break;
			}
			else
				CDCReceiveLen = 0;	
		}
    if (ISO14443_SingleTagSelect(snr))
    {
      if (ISO14443_SingleTagSelect(snr))
        continue;
    }   
    if (ISO14443_LoginE2(0,0x10))
    {
     if (ISO14443_SingleTagSelect(snr))
     {
      if (ISO14443_SingleTagSelect(snr))
				continue;
     }
     if (ISO14443_LoginE2(0,0x10)) {continue;}
    }
		if (!ISO14443_ReadBlock(1,buffRC522)) 		 
		{			
			if ((buffRC522[6] == 0xAB) && (buffRC522[7] == 0) && (buffRC522[15] == 0))
			{
				UCC=buffRC522[0];
				UCC+=(buffRC522[1]<<8);
				cardIDD=buffRC522[2];
				cardIDD+=(buffRC522[3]<<8);
				cardIDD+=(buffRC522[4]<<16);
				cardIDD+=(buffRC522[5]<<24);
				if ((UCC & 0x8000)==0x8000)
				{
					UCC = UCC & 0x7FFF;
					UCC = UCC ^ 0x6A29;
					cardIDD = cardIDD ^ 0xB2A6;
				}
				if ((cardIDD == ziroEtebarID) && (UCC == ziroEtebarUC))
				{
				 if (ISO14443_SingleTagSelect(snr))
         {
          if (ISO14443_SingleTagSelect(snr))
               continue; 
         }
				 if (ISO14443_LoginE2(4,0x39))
				 {
					if (ISO14443_SingleTagSelect(snr))
					{
						if (ISO14443_SingleTagSelect(snr))
							continue;
					}
					if (ISO14443_LoginE2(4,0x39)) {continue;}
				 }	
				 if (ISO14443_WriteBlock(18,HediyeBuf))  continue;
				 sprintf(buf,"{\"C\":{\"Status\":0,\"UC\":%u,\"ID\":%u}}\n",UCC,cardIDD);
				 send(buf);  
         break;				 
				}	
				else if (cardIDD != ziroEtebarID)
				{
				 sprintf(buf,"{\"C\":{\"Status\":9,\"UC\":%u,\"ID\":%u}}\n",UCC,cardIDD);
				 send(buf);  
         break;							
				}
				else if (UCC != ziroEtebarUC)
				{
				 sprintf(buf,"{\"C\":{\"Status\":10,\"UC\":%u,\"ID\":%u}}\n",UCC,cardIDD);
				 send(buf);  
         break;		
				}
			}
		}
		else
			continue;
	}
	CDCReceiveLen = 0;	
	return ;
}

void resetCredit(void) {
	signed int
		preCredit = 0,
		preGift 	= 0;
	
	unsigned int 
		Block, 
		ti = 0;

	unsigned char 
		snr[6], 
		buf[100], 
		block[16],
		OPBlock[16], 
		step 			= 0, 
		crc 			= 0x00,
		requestID = 0x00;

	unsigned long int 
		UCC 		= 0,
		cardIDD = 0;
	
	//RC DID(4byte) Year Month Day Hour Minute Second CardID(4Byte) UC(4Byte) RequestID(4Byte)

	if (fromAndroid[1] != 'C')
		return;
	
	crc = 0;
	for (unsigned char cnt = 0;cnt<24;cnt++)
		crc ^= fromAndroid[cnt];
	if (crc != fromAndroid[24]) {
		INVALIDJson;
		rxAndroid = 1;
	  memset(fromAndroid,'\0',sizeof(fromAndroid));
		return;
	}

	{//Initialization
	OPBlock[0]  = fromAndroid[11]; //Second	
	OPBlock[2]  = 0; 						 //pre credit
	OPBlock[3]  = 0;
	OPBlock[4]  = 0;
	OPBlock[5]  = 2; //Decrement
	OPBlock[6]  = (fromAndroid[6] << 4) + (fromAndroid[7] & 0x0F); //Year-Month
	OPBlock[7]  = fromAndroid[8]; //Day
	OPBlock[8]  = 0x00; //Credit
	OPBlock[9]  = 0x00;
	OPBlock[10] = 0x00;
	OPBlock[11] = fromAndroid[4]; //Device ID
	OPBlock[12] = fromAndroid[5];
	OPBlock[13] = fromAndroid[9]; //Hour
	OPBlock[14] = fromAndroid[10]; //Minute
	for (step=0; step<15; step++)
		crc ^= OPBlock[step];
	OPBlock[15] = crc;

	ziroEtebarID = (fromAndroid[12]<<24) + (fromAndroid[13]<<16) + (fromAndroid[14]<<8) + fromAndroid[15];
	ziroEtebarUC = (fromAndroid[16]<<24) + (fromAndroid[17]<<16) + (fromAndroid[18]<<8) + fromAndroid[19];
	requestID    = (fromAndroid[20]<<24) + (fromAndroid[21]<<16) + (fromAndroid[22]<<8) + fromAndroid[23];
	HediyeBuf[0] = 0;
	HediyeBuf[1] = 0;
  HediyeBuf[2] = 0;	
  HediyeBuf[3] = 0;	
  HediyeBuf[4] = 0xAB;	
  HediyeBuf[5] = 0xCD;
  CDCReceiveLen = 0;	
	ti = HAL_GetTick();
	step = 0;
	UCC  = 0;
	}

	while (1) {
		
		{// Calculate TimuOut 
			WDTR
			if ((HAL_GetTick() - ti) > 16000) {
				TIMEOUTJSON
				break;
			}}
		
		{// If the process is canceled?
		if (CDCReceiveLen) {
			if ((fromAndroid[0] == 'c') || (fromAndroid[0] == 'C')) {
				CANSELLJSON;					
				break;
			}
			else
				CDCReceiveLen = 0;	
		}}
		if (UCC == 0) {// Is the card placed on the reader? 
			if (ISO14443_SingleTagSelect(snr)) 		continue;
			if (ISO14443_LoginE2(0,0x10))      		continue;
			if (ISO14443_ReadBlock(1, buffRC522)) continue;
			if ((buffRC522[6] == 0xAB) && (buffRC522[7] == 0) && (buffRC522[15] == 0)) {
				UCC      = buffRC522[0];
				UCC     += (buffRC522[1] << 8);
				cardIDD  = buffRC522[2];
				cardIDD += (buffRC522[3] << 8);
				cardIDD += (buffRC522[4] << 16);
				cardIDD += (buffRC522[5] << 24);
				if ((UCC & 0x8000) == 0x8000) {
					UCC = UCC & 0x7FFF;
					UCC = UCC ^ 0x6A29;
					cardIDD = cardIDD ^ 0xB2A6;
				}
			}
			else {
				sprintf(buf, "{\"C\":{\"Status\":8,\"UC\":0,\"ID\":0}}\n"); //None-JahanGostar Card
				send(buf);
				break;
			}
		}
		
		if ((cardIDD == ziroEtebarID) && (UCC == ziroEtebarUC)) {
			//Check if there is a card placed on the reader
			//sendDebug("F1\n");
			if (ISO14443_SingleTagSelect(snr))
				continue;

			//sendDebug("F2\n");
			if (step < 1) {//Reset the credit block
				if (ISO14443_LoginE2(CreditSector, CreditLogin)) continue;
				if (ISO14443_ReadValueBlock(16, &preCredit)) 		 continue;
				if (ISO14443_WriteValue(16, 0)) 						 		 continue;
				step = 1;
			}
			
			//sendDebug("F3\n");
			if (step < 2) {//Reset the Gift block
				if (ISO14443_LoginE2(4, 0x39)) 					continue;
				if (ISO14443_ReadBlock(18, block)) 			continue;
				if ((block[4] == 0xAB) && (block[5] == 0xCD)) {
					preGift  = block[3]; preGift <<= 8;
					preGift += block[2]; preGift <<= 8; 
					preGift += block[1]; preGift <<= 8; 
					preGift += block[0];
				}
				else
					preGift = -1;
				if (ISO14443_WriteBlock(18, HediyeBuf)) continue;
				step = 2;
			}
			
			//sendDebug("F4\n");
			if (step < 3) {//Read the last OP index
				if (ISO14443_LoginE2(6, 0x14)) 					 continue;
				if (ISO14443_ReadValueBlock(24, &Block)) continue;
				step = 3;
			}

			//sendDebug("F5\n");
			if (step < 4) {//Write the OP record
				if (ISO14443_LoginE2(Old_GetBlockIndex(Block) / 4, 0x35)) 	continue;
				if (ISO14443_WriteBlock(Old_GetBlockIndex(Block), OPBlock)) continue;
				step = 4;
			}

			//sendDebug("F6\n");
			if (step < 5) {//Write the OP index
				if (++Block > 13) Block = 0;  
				if (ISO14443_LoginE2(6, 0x35)) 			continue;
				if (ISO14443_WriteValue(24, Block)) continue;
				step = 5;
				break;
			}
		}
		
		else if (cardIDD != ziroEtebarID) {
			sprintf(buf, "{\"C\":{\"Status\":9,\"UC\":%u,\"ID\":%u}}\n", UCC, cardIDD);
			send(buf);
			break;
		}
		
		else if (UCC != ziroEtebarUC) {
			sprintf(buf, "{\"C\":{\"Status\":10,\"UC\":%u,\"ID\":%u}}\n", UCC, cardIDD);
			send(buf);
			break;
		}
	}
	
	if (step == 5) { //The process is succesfully done
		sprintf(buf, "{\"C\":{\"Status\":%d,\"UC\":%u,\"ID\":%u,\"" \
				"Etebar\":%d,\"PreHedie\":%d,\"Hedie\":%d,\"PreEtebar\":%d,\"Price\"" \
				":%d,\"EEM\":%u}}\n", 0, ziroEtebarUC, ziroEtebarID, 0, 
				preGift, 0, preCredit, preCredit, requestID);
		send(buf);
	}
	else if (step >= 1) { //At least the main credit is changed!
		sprintf(buf, "{\"C\":{\"Status\":%d,\"UC\":%u,\"ID\":%u,\"" \
				"Etebar\":%d,\"PreHedie\":%d,\"Hedie\":%d,\"PreEtebar\":%d,\"Price\"" \
				":%d,\"EEM\":%u}}\n", 113, ziroEtebarUC, ziroEtebarID, 0, 
			step >= 2 ? preGift : -1, step >= 2 ? 0 : -1, preCredit, preCredit, 
			requestID);
		send(buf);
	}
	else if (step == 0)
		TIMEOUTJSON

	if (step > 0) {
		ti = HAL_GetTick();
		UCC = 3;
		while (UCC > 0) {
			if ((HAL_GetTick() - ti) >= 1000) {
				send(buf);
				ti = HAL_GetTick();
				UCC--;
			}
		}
	}
	
	CDCReceiveLen = 0;	
	return ;
}

#endif
#if defined GATEROADBLOCK
unsigned char CloseBridge(void)
{
unsigned int Li=0;
unsigned char  k=0;

//	printf("\n\rCloseBridge");
	
	if (IR2==1)
		return(23);

	RELAY_PWR(0) 
	HAL_Delay(RELAY_REST_TIME); //make sure gate is not working
	RELAY_DIR(0) 
	HAL_Delay(RELAY_REST_TIME); 
	RELAY_PWR(1)
  Li = HAL_GetTick();
  while (HAL_GetTick() - Li < 10000)	
	{
		WDTR;
		if (SNSDOWN==1)
			if (++k>10)
				break;			
		if (IR2==1)
		{
      //Res=22;
			break;			
		}
			
	}
	RELAY_PWR(0) 
	HAL_Delay(RELAY_REST_TIME); 
	RELAY_DIR(0)
	return(0);
	
}
void OpenBridge(unsigned char inout)  //1 in  //2 out
{
	if (inout==1)
	  RELAY_IN(1)
	else
    RELAY_OUT(1)
	BridgeOpenTime=10;
}
void gateRoadBlockProcess(void)
{
	// 6f Len(1byte) Command(1byte) State(1byte) Device(1byte) Reserve(4byte) crc(1byte)
  //  0     1           2              3             4           5-8            9   
  // command : 	
	// state : 1 in  , 2 out  , 3 open , 4 close 
  // Device : g = gate / r = roadBlock 	
	unsigned char checkSum = 0;
	unsigned char buf[50] = {0};
	if (fromAndroid[1] < 7) // Len 
	{
		INVALIDJson;
		return;
	}
	
	for (unsigned char idx = 2; idx < 9 ; idx++)
		checkSum ^= fromAndroid[idx];
	
	if (checkSum != fromAndroid[9])
	{
		INVALIDJson
		return;
	}
		
	if (fromAndroid[4] == 'g') //device gate 
	{
		DeviceType = 'g';
		if (fromAndroid[3] == 1) // state , in 
		{
			if (BridgeOpenTime == 0)
			{
				OpenBridge(1); // in
				send("{\"GR\":{\"Status\":1}}\n");
			}
			else
			{
				sprintf(buf,"{\"GR\":{\"OpenCloseTime\":%d}}\n",BridgeOpenTime);
				send(buf);
			}
		}
		else if (fromAndroid[3] == 2) // state , out 
		{
			if (BridgeOpenTime == 0)
			{
				OpenBridge(2); // out
				send("{\"GR\":{\"Status\":2}}\n");
			}
			else
			{
				sprintf(buf,"{\"GR\":{\"OpenCloseTime\":%d}}\n",BridgeOpenTime);
				send(buf);
			}
		}
		else
			INVALIDJson
	}
	else if (fromAndroid[4] == 'r') //device rahband 
	{
		DeviceType = 'r';
		if (fromAndroid[3] == 3)  // state , open 
		{
			if ((CurBridgeState!=BRIDGE_OPEN) && (CurBridgeState!=BRIDGE_OPENING) && (CurBridgeState!=BRIDGE_OPENED))
				CurBridgeState=BRIDGE_OPEN;
			else
			{
				sprintf(buf,"{\"GR\":{\"CurBridgeState\":%d}}\n",CurBridgeState);
				send(buf);				
			}
			BridgeOpenTime = fromAndroid[5] * 1000;
			timeDelay = fromAndroid[6] * 1000;
		}
		else if (fromAndroid[3] == 4) // state , close 
		{
  		if ((CurBridgeState!=BRIDGE_CLOSE) && (CurBridgeState!=BRIDGE_CLOSING) && (CurBridgeState!=BRIDGE_CLOSED))
    		CurBridgeState=BRIDGE_CLOSE;
			else
			{
				sprintf(buf,"{\"GR\":{\"CurBridgeState\":%d}}\n",CurBridgeState);
				send(buf);
			}				
			BridgeOpenTime = fromAndroid[5] * 1000;
			timeDelay = fromAndroid[6] * 1000;
		}
		else
			INVALIDJson
	}
	else
		INVALIDJson;
	memset(fromAndroid,0,20);
}

#endif

void GetCPUiD(void)
{
	unsigned char buf[100];
  uint32_t a = *((uint32_t*)UID_BASE);
  uint32_t b = *((uint32_t*)UID_BASE + 0x4);
  uint32_t c = *((uint32_t*)UID_BASE + 0x8);
	sprintf(buf,"{\"CPUID\":{\"X1\":%8.8X,\"X2\":%8.8X,\"X3\":%8.8X}}\n",a,b,c);
	send(buf);
	return ;
}
#ifdef NFC

void ProcessNFCCard(void)
{
	volatile unsigned int cash = 0;
	volatile unsigned int price = 0;
  uint8_t NtagCheck[4] ={0};
	if (NtagReadBlock(8,NtagCheck))
	{
		HAL_Delay(50);
		if (NtagReadBlock(8,NtagCheck))
		{
			send("eror-2\n");
			return;
		}
	}
	if (NtagCheck[3] != (NtagCheck[0] ^ NtagCheck[1] ^ NtagCheck[2] ^ 0xC9))
	{
			send("eror-1\n");
			//return;		
	}
	
	if (NtagReadBlock(6,NtagRead))
	{
		HAL_Delay(50);
		if (NtagReadBlock(6,NtagRead))
		{
			send("eror\n");
			return;
		}
	}
	NtagRead[0] = NtagRead[0] ^ NtagCheck[0] ^ 0xAA;
	NtagRead[1] = NtagRead[1] ^ NtagCheck[1] ^ 0xBB;
	NtagRead[2] = NtagRead[2] ^ NtagCheck[2] ^ 0xCC;
	NtagRead[3] = NtagRead[3] ^ NtagCheck[3] ^ 0xDD;
	cash = (NtagRead[0] << 24)  +  (NtagRead[1] << 16)  + (NtagRead[2] << 8) + NtagRead[3] ;
	if (NtagReadBlock(7,NtagRead))
	{
		HAL_Delay(50);
		if (NtagReadBlock(7,NtagRead))
		{
			send("error\n");
			return;
		}
	}
	NtagRead[0] = NtagRead[0] ^ NtagReadZiroBlock[0] ^ 0xAB;
	NtagRead[1] = NtagRead[1] ^ NtagReadZiroBlock[1] ^ 0xBA;
	NtagRead[2] = NtagRead[2] ^ NtagReadZiroBlock[2] ^ 0xCD;
	NtagRead[3] = NtagRead[3] ^ NtagReadZiroBlock[3] ^ 0xDC;
	
	if ( cash != ( (NtagRead[0] << 24) +  (NtagRead[1] << 16)  + (NtagRead[2] << 8) + NtagRead[3] )) 
	{
		send("error1\n");
		//return ;
	}
	if (cash <  Config.MinEtebar)
	{
		send("error2\n");
		//return;
	}
	if (cash <  GrouhPrice[0])
	{
		send("error3\n");
		//return;
	}
	if (cash >  Config.MaxEtebar)
	{
		send("error4\n");
		//return;
	}
	HAL_RTC_GetTime(&hrtc,&time,RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc,&date,RTC_FORMAT_BIN);
	NtagWrite[0] = time.Minutes ^ 0xB6;
	NtagWrite[1] = time.Seconds ^ 0xB7;
	NtagWrite[2] = date.Date ^ 0xB8;
	NtagWrite[3] = NtagWrite[0] ^ NtagWrite[1] ^ NtagWrite[2] ^ 0xC9;
	if (NtagWriteBlock(8,NtagWrite))
	{
		HAL_Delay(10);
		if (NtagWriteBlock(8,NtagWrite))
		{
			send("error5");
			return;
		}
	}
	price = cash - GrouhPrice[0];	
	NtagWrite[0] = ((price >> 24) & 0xff ) ^ NtagWrite[0] ^ 0xAA;
	NtagWrite[1] = ((price >> 16) & 0xff ) ^ NtagWrite[1] ^ 0xBB;
	NtagWrite[2] = ((price >> 8 ) & 0xff ) ^ NtagWrite[2] ^ 0xCC;
	NtagWrite[3] = ((price)       & 0xff ) ^ NtagWrite[3] ^ 0xDD;
	if (NtagWriteBlock(6,NtagWrite))
	{
		HAL_Delay(10);
		if (NtagWriteBlock(6,NtagWrite))
		{
			send("error6");
			return;
		}
	}
	NtagWrite[0] = ((price >> 24) & 0xff ) ^ NtagReadZiroBlock[0] ^ 0xAB;
	NtagWrite[1] = ((price >> 16) & 0xff ) ^ NtagReadZiroBlock[1] ^ 0xBA;
	NtagWrite[2] = ((price >> 8 ) & 0xff ) ^ NtagReadZiroBlock[2] ^ 0xCD;
	NtagWrite[3] = ((price)       & 0xff ) ^ NtagReadZiroBlock[3] ^ 0xDC;	
	if (NtagWriteBlock(7,NtagWrite))
	{
		HAL_Delay(10);
		if (NtagWriteBlock(7,NtagWrite))
		{
			send("error7");
			return;
		}
	}
}
#endif
//ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss
void reSendLastTransaction(void)
{	
	unsigned char readRecord[LEN_TRANSACTIONS] = {0};
	unsigned char sendToAndroid[300];
  unsigned int ucRam = 0,cardIdRam = 0;
	#if (deviceType==AutoPark)
	unsigned char gpRam,yRam,MRam,dRam,hRam,mRam,sRam,eemRam ; 
	#endif
	
	#if (deviceType==AmusementPark)
	unsigned int etebarRam,preEtebarRam,hPaymentRam,paymentRam,hNPaymentRam ;
  unsigned char statusRam;	
	#endif	
	
	#if (deviceType==BusEtebar)
	unsigned int etebarRam,preEtebarRam,paymentRam ;
  unsigned char statusRam;	
	#endif	

	#if (deviceType==BCU)
	unsigned int etebarRam,preEtebarRam,paymentRam,gpRam ;
  unsigned char statusRam;	
	#endif	
	
	if (	memID == 0XEF16)
	{
		if ((buffTransaction[0] == 0) && (buffTransaction[1] == 0))
		{			
			if (trHead)
			{
				LoadFromDFToRam(addTransactions+((trHead-1)*LEN_TRANSACTIONS),LEN_TRANSACTIONS,readRecord);				
				if (readRecord[0] == deviceType)
				{
					#if (deviceType==AutoPark)
					cardIdRam = readRecord[1] + ( readRecord[2] << 8) + ( readRecord[3] << 16) + ( readRecord[4] << 24);   
					ucRam     = readRecord[5] + ( readRecord[6] << 8) + ( readRecord[7] << 16) + ( readRecord[8] << 24);
					gpRam     = readRecord[9];
					yRam      = readRecord[10];
					MRam      = readRecord[11];
					dRam      = readRecord[12];
					hRam      = readRecord[13];
					mRam      = readRecord[14];
					sRam      = readRecord[15];
					eemRam    = readRecord[16];
					sprintf(sendToAndroid,"{\"C\":{\"ID\":%d,\"UC\":%d,\"Gp\":%d,\"Y\":%d,\"M\":%d,\"D\":%d,\"H\":%d,\"m\":%d,\"S\":%d,\"EEM\":%d}}\n",cardIdRam,ucRam,gpRam,yRam,MRam,dRam,hRam,mRam,sRam,eemRam);
					#endif
					
					#if (deviceType==AmusementPark)
					cardIdRam     = readRecord[1] + ( readRecord[2] << 8) + ( readRecord[3] << 16) + ( readRecord[4] << 24);   
					ucRam         = readRecord[5] + ( readRecord[6] << 8) + ( readRecord[7] << 16) + ( readRecord[8] << 24);
					statusRam     = readRecord[9];			
					etebarRam     = readRecord[11] + ( readRecord[12] << 8) + ( readRecord[13] << 16) + ( readRecord[14] << 24);
					preEtebarRam  = readRecord[15] + ( readRecord[16] << 8) + ( readRecord[17] << 16) + ( readRecord[18] << 24);
					hPaymentRam   = readRecord[19] + ( readRecord[20] << 8) + ( readRecord[21] << 16) + ( readRecord[22] << 24);
					paymentRam    = readRecord[23] + ( readRecord[24] << 8) + ( readRecord[25] << 16) + ( readRecord[26] << 24);
					hNPaymentRam  = readRecord[27] + ( readRecord[28] << 8) + ( readRecord[29] << 16) + ( readRecord[30] << 24);
					sprintf(sendToAndroid,"{\"C\":{\"Status\":%d,\"UC\":%u,\"ID\":%u,\"Etebar\":%u,\"PreEtebar\":%u,\"Gt\":%u,\"Price\":%u,\"Hedie\":%u}}\n",statusRam,ucRam,cardIdRam,etebarRam,preEtebarRam,hPaymentRam,paymentRam,hNPaymentRam);
					#endif
					
					#if (deviceType==BusEtebar)
					cardIdRam     = readRecord[1] + ( readRecord[2] << 8) + ( readRecord[3] << 16) + ( readRecord[4] << 24);   
					ucRam         = readRecord[5] + ( readRecord[6] << 8) + ( readRecord[7] << 16) + ( readRecord[8] << 24);
					statusRam     = readRecord[9];			
					etebarRam     = readRecord[11] + ( readRecord[12] << 8) + ( readRecord[13] << 16) + ( readRecord[14] << 24);
					preEtebarRam  = readRecord[15] + ( readRecord[16] << 8) + ( readRecord[17] << 16) + ( readRecord[18] << 24);
					paymentRam   = readRecord[19] + ( readRecord[20] << 8) + ( readRecord[21] << 16) + ( readRecord[22] << 24);
					sprintf(sendToAndroid,"{\"C\":{\"Status\":%d,\"UC\":%u,\"ID\":%u,\"Etebar\":%u,\"PreEtebar\":%u,\"Gt\":%u,\"Price\":%u,\"Hedie\":%u}}\n",statusRam,ucRam,cardIdRam,etebarRam,preEtebarRam,paymentRam);
					#endif
					#if (deviceType==BCU)
					cardIdRam     = readRecord[1] + ( readRecord[2] << 8) + ( readRecord[3] << 16) + ( readRecord[4] << 24);   
					ucRam         = readRecord[5] + ( readRecord[6] << 8) + ( readRecord[7] << 16) + ( readRecord[8] << 24);
					statusRam     = readRecord[9];			
					etebarRam     = readRecord[11] + ( readRecord[12] << 8) + ( readRecord[13] << 16) + ( readRecord[14] << 24);
					preEtebarRam  = readRecord[15] + ( readRecord[16] << 8) + ( readRecord[17] << 16) + ( readRecord[18] << 24);
					paymentRam    = readRecord[19] + ( readRecord[20] << 8) + ( readRecord[21] << 16) + ( readRecord[22] << 24);
					gpRam         = readRecord[28];
					sprintf(sendToAndroid,"{\"C\":{\"Status\":%d,\"UC\":%u,\"ID\":%u,\"Etebar\":%u,\"PreEtebar\":%u,\"Gp\":%d,\"Price\":%u}}\n",statusRam,ucRam,cardIdRam,etebarRam,preEtebarRam,gpRam,paymentRam);
					#endif
					send(sendToAndroid);  //ersal ID card
					HAL_Delay(10);
				}
				
				return;				
			}
			else
			{
				EmptyMem
				return;
			}
		}
		else
		{
			send(buffTransaction);
			return;
		}
		
	}
	else
	{
		if ((buffTransaction[0] == 0) && (buffTransaction[1] == 0))
		{
			EmptyMem
			return;
		}
		else
		{
			send(buffTransaction);
			return;			
		}
	}		
}

void trHeadSend(void) {
	unsigned char sendToAndroid[100] = {0};
	sprintf(sendToAndroid,"{\"Mem\":{\"Head\":%u,\"Tail\":%u}}\n",trHead,trTail);
	send(sendToAndroid);
	HAL_Delay(5);
}

void DeletetrHead (void) {	
	trTail = trHead = 0;
	SaveRingDetail();
	DeleteMem
}
//ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void) {
	unsigned long ti; //HNA
	unsigned char iHNA;
	
  HAL_Init();
  SystemClock_Config();

  MX_GPIO_Init();
  MX_RTC_Init();
  MX_SPI1_Init();
	MX_SPI2_Init();
  MX_TIM3_Init();
	
	#if defined (Max30102) || defined(PN532_Routine)
	MX_I2C1_Init();
	#endif
	
	#ifndef atlas
		#if (deviceType != Appointment)
		MX_USART1_UART_Init();
		#endif
	#endif
	
	#if (deviceType != Appointment)
  MX_USART2_UART_Init();
	#endif
	
	#if defined(Printer) || /*defined(heightHum) ||*/ defined(cardAccDis) || (deviceType == Sport) || defined (UHF) || defined(pos)
  MX_USART3_UART_Init();
	#endif

#ifdef serialport
	MX_USART3_UART_Init();            		         
	#endif

  MX_IWDG_Init();
  /* USER CODE BEGIN 2 */
  
	#ifdef remote 
	MX_TIM2_Init();
	HAL_TIM_Base_Start_IT(&htim2);
  HAL_TIM_Base_Start(&htim2);	
	#endif

	#ifdef virtualRemote 
	MX_TIM1_Init();
	HAL_TIM_Base_Start(&htim1);	
	#endif

	#if defined DS7 || defined heightHum 
	HAL_TIM_Base_Start_IT(&htim3);
	#endif
	
	#if (deviceType != Appointment)
	HAL_TIM_Base_Start(&htim3);
	#endif
	
	#if defined DS7

SoftUartInit(0,GPIOB,GPIO_PIN_3,GPIOC,GPIO_PIN_13);
	sendWithSoftUart("ss",2);
	#endif
	
	#if defined heightHum
	SoftUartInit(0,GPIOC,GPIO_PIN_13,GPIOB,GPIO_PIN_3);
	SoftUartEnableRx(0);
	#endif
	
  #if (deviceType != Appointment)
	__HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);
	#endif
	
	HAL_Delay(10);
	
	#if defined (Printer) || /*defined(heightHum) ||*/ defined(cardAccDis) || (deviceType == Sport) || defined (UHF) || defined(pos)
	__HAL_UART_ENABLE_IT(&huart3, UART_IT_RXNE);
	#endif
	
	#ifndef atlas
		#if (deviceType != Appointment)
		__HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
		#endif
	#endif	
	
	bufferLen=0;	
	
	#ifdef PN532_Routine
	PN532_Reset();
	#endif
	
  MFRC522_Init();
  
	ee_init();
	ledBlik(7,30);
	
	
	#if (deviceType==BD) || (deviceType==BCU)
	SetRX485()
	#if defined(ExecTransactionBCU)
	loadConfig(35);
	#endif
	#if defined(ExecTransactionAPARK)
	loadConfig(36);
	#endif
//	readVirtualAddress();
	#endif	
//	#ifdef AutoStart
//	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_10,1);
//	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,1);
//	HAL_Delay(2025);
//	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_10,0);
//	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,0);	
//	#endif	
	#if (deviceType==AutoPark)
	readConfig(3);
				GetCardDispenserStatusProccess();     

	#endif
	
	HAL_Delay(1000);
  MX_USB_DEVICE_Init();
	HAL_Delay(1000);
	
	#if defined AutoStart
	POWERTAB(0)
	HAL_Delay(2025);
	WDTR
	HAL_Delay(2025);
	POWERTAB(1)
	#endif

	#ifdef Max30102
	testConnectionMax30102();
	#endif
	
	#ifdef mediaTechTab
	for (unsigned char a=0;a<18;a++)
	{		
		HAL_Delay(1000);
		__HAL_IWDG_RELOAD_COUNTER(&hiwdg);
	}
	ledBlik(5,30);
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_10,1);
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,1);
	#endif
 
	
	resetSystem = 1;
	WDTR
	
  #if defined LCD
	serialConnectWithTab = 10;
	Init_TFT24();	
	fistPage();
	showMainPrice(GrouhPrice[0],1);
	showWellCome();
	showDateTime();
	#endif
	
	#if defined GATEROADBLOCK
	CloseBridge();
	#endif
	//ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss
  memID  = SPI_Flash_ReadID();
  if (memID == 0XEF16) {
		SPI_Flash_Write("QWERTYUIOP",0x7fff0,16);	
		LoadRecordsInfo();	
	}
  //ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss
  /* USER CODE BEGIN WHILE */
	//*****************************************************************************************
	//****************************************************************************************
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//			  HAL_Delay(1000);
//		unsigned char pasargadTest[] = {
//		0x00, 0x00 ,0x00 ,0x0E ,0x02 ,0xF9 ,0x45 ,0x70 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00, 
//		0x00 ,0x31 ,0x32 ,0x34 ,0x35 ,0x1C ,0x32 ,0x30 ,0x30 ,0x30 ,0x30 ,0x1C ,0x1C ,0x1C ,0x1C,
//		0x16 ,0x56 ,0x80 ,0x0f ,0x73 ,0x93 ,0xfc ,0xce ,0xe6 ,0xaa ,0xbf ,0x7a ,0x45 ,0x3c ,0x6a,
//		0x6e ,0xa8 ,0x31 ,0x44 ,0xfd
//	};
	#if (deviceType==AutoPark)
				GetCardDispenserStatusProccess();     
	#endif      

	MFRC522_Init();
	#ifdef Max30102
	ADC1_Init();
	#endif
	uint32_t tAlive  = HAL_GetTick(); //HNA
	uint32_t adcTick = HAL_GetTick();
  while (1) {
		if ((HAL_GetTick() - tAlive) >= 3000) {
			tAlive = HAL_GetTick();
			dontSend = 1;
			//send(ALIVE_MESSAGE);
			dontSend = 0;
		}

		#ifdef serialport
		
				 HAL_UART_Transmit(&huart2, fromAndroid, strlen(fromAndroid), 1000);	//posTest Terminal
		 
		#endif
		#if defined GATEROADBLOCK
		WDTR
		//------------------------- GATE --------------------------
		if (DeviceType == 'g')
		{
			if (SNSUP == 0)
			{
				if ((++Time>15) && (BridgeOpenTime>0))
				{
					BridgeOpenTime=0;
					RELAY_IN(0)
					RELAY_OUT(0)
					send("{\"GR\":{\"Status\":3}}\n");
				}
			}
			else 
				Time=0;	
			
			if (tickOpenClose>=1000)
			{
				tickOpenClose=0;
				if (BridgeOpenTime)
				{
					if (--BridgeOpenTime==0)
					{
						RELAY_OUT(0)
						RELAY_IN(0)
						send("{\"GR\":{\"Status\":4}}\n");
					}
				}
			} 
		}
		//------------------------- Road Block ---------------------------------------
		if (DeviceType == 'r')
		{
			switch (CurBridgeState)
			{
				case BRIDGE_OPEN:
					CurBridgeState = BRIDGE_OPENING;
					TimeKeeper = HAL_GetTick();
					RELAY_PWR(0)
					HAL_Delay(RELAY_REST_TIME); //make sure gate is not working
				  if (RELAYSTATE == 0)
						RELAY_DIR(1) 
					else
					{
						send("{\"GR\":{\"Status\":10}}\n"); // relaye power off nshode ast state open
						CurBridgeState = BRIDGE_OPEN;
						RELAY_PWR(0)
						continue;
					}
					HAL_Delay(RELAY_REST_TIME); 
					RELAY_PWR(1)
					send("{\"GR\":{\"Status\":5}}\n"); 
					break;
				case BRIDGE_OPENING:
					if ((SNSUP) || (HAL_GetTick() - TimeKeeper > timeDelay)) 
					{
						RELAY_PWR(0) 
						HAL_Delay(RELAY_REST_TIME); 
						if (RELAYSTATE == 0)
							RELAY_DIR(0)
						else
						{
							send("{\"GR\":{\"Status\":11}}\n");
							CurBridgeState = BRIDGE_OPENING;
							RELAY_PWR(0)
							continue;
						}
						sprintf(buffRC522,"{\"GR\":{\"SNSUP\":%d,\"Time\":%d}}\n",SNSUP,HAL_GetTick() - TimeKeeper);
						send(buffRC522);
						CurBridgeState=BRIDGE_OPENED;
						TimeKeeper=HAL_GetTick();
						onceSend = 1;
					}
					break;
				case BRIDGE_OPENED:					
					if (BridgeOpenTime!=0)
					{
						if (HAL_GetTick() - TimeKeeper > BridgeOpenTime)
						{
							send("{\"GR\":{\"Status\":6}}\n");
							CurBridgeState = BRIDGE_CLOSE;
						}
					}
					else
					{
						if (onceSend)
						{
							onceSend = 0;
							send("{\"GR\":{\"Status\":12}}\n");
						}
					}
					break;
				case BRIDGE_CLOSE:
					send("{\"GR\":{\"Status\":7}}\n");
					CurBridgeState=BRIDGE_CLOSING;
					TimeKeeper=HAL_GetTick();
					RELAY_PWR(0) 
					HAL_Delay(RELAY_REST_TIME); //make sure gate is not working
				  if (RELAYSTATE == 0)
						RELAY_DIR(0) 
					else
					{
						send("{\"GR\":{\"Status\":13}}\n");
						CurBridgeState = BRIDGE_CLOSE;							
					}
					HAL_Delay(RELAY_REST_TIME); 
					RELAY_PWR(1)
					break;				
				case BRIDGE_CLOSING:
					if ((SNSDOWN) || (HAL_GetTick() - TimeKeeper > timeDelay)) //Timer
					{
						RELAY_PWR(0) 
						HAL_Delay(RELAY_REST_TIME);
						if (RELAYSTATE == 0)						
							RELAY_DIR(0)
						else
						{
							send("{\"GR\":{\"Status\":14}}\n");
							CurBridgeState = BRIDGE_CLOSING;
							RELAY_PWR(0) 
						}
						CurBridgeState=BRIDGE_CLOSED;
						sprintf(buffRC522,"{\"GR\":{\"SNSDOWN\":%d,\"Time\":%d}}\n",SNSDOWN,HAL_GetTick()-TimeKeeper);
						send(buffRC522);
					}
					if (IR2==1)
					{
						RELAY_PWR(0) 
						HAL_Delay(RELAY_REST_TIME);
						if (RELAYSTATE == 0)	
							RELAY_DIR(0)
						else
						{
							RELAY_PWR(0) 
							send("{\"GR\":{\"Status\":15}}\n");
							CurBridgeState = BRIDGE_CLOSING;							
						}
						CurBridgeState = BRIDGE_OPEN;
						send("{\"GR\":{\"Status\":8}}\n");

					}
					break;				
				case BRIDGE_CLOSED:
					send("{\"GR\":{\"Status\":9}}\n");
				  CurBridgeState = Idle;
					break;
			}
		}	
		
 		if (enterExit != 10)
			if (IR1 == 0)
				enterExit = 1;	
			
		if (enterExit != 1)
			if (IR3 == 0)
				enterExit = 10;	
			
		if (enterExit == 1)
			if (IR3 == 0)
				enterExit++;		
			
		if (enterExit == 10)
			if (IR1 == 0)
				enterExit = 11;	
			
    if ((IR1 == 0) && (IR3 == 0))
			enterExit = 0;	
		
		if (enterExit == 2)
		{
			send("{\"GR\":{\"EnterExit\":1}}\n"); // vorod
			enterExit = 0;
      while ((IR1 == 0) || (IR3 == 0))
				WDTR
			 enterExit = 0;				
		}		
		if (enterExit == 11)
		{
			send("{\"GR\":{\"EnterExit\":0}}\n"); // khoroj
			enterExit = 0;
      while ((IR1 == 0) || (IR3 == 0))
				WDTR
			 enterExit = 0;		
		}
		#endif
		#if defined(PN532_Routine)//*****************************
			if (i2cStatus != HAL_OK)
			{
				PN532_Reset();
				WDTR
				MX_I2C1_Init();
				HAL_Delay(1);
			}
		#endif	
			
		WDTR
//---------------------------------------------------------MFRC522		
		#ifdef Max30102
		heartBeat();
		if (max30102Flag) {
			//max30102_read_fifo(&max30102);
			
			/*Max30102_Task();
			if ((Max30102_IsFingerOnSensor() == 1) && ((hr_old != Max30102_GetHeartRate())
					|| (spo2_old != Max30102_GetSpO2Value())))
			{
				hr_old = Max30102_GetHeartRate();
				spo2_old = Max30102_GetSpO2Value();
				if ((hr_old != 0) || (spo2_old != 0)) 
				{
					if (hr_old<=200)
					{
						memset(HrSpo2Buffer,0,sizeof(HrSpo2Buffer));
						sprintf(HrSpo2Buffer,"{\"M\":{\"HR\":%d}}\n", hr_old);
						send(HrSpo2Buffer);
					}
				}
				HAL_Delay(10);
			}*/
		}			
		#endif

		#ifdef MaxReciveBuf
		if (rxBufferHeadPos)
		#endif
		
		#ifndef MaxReciveBuf
		if (CDCReceiveLen)
		#endif
		{ 
  		HAL_Delay(1);
			
			#ifdef finger
			/* Command Write template by ID */	
			
				if ((fromAndroid[0] == 0x57) && (fromAndroid[5] == 0x65)) {
					send("{\"Receive\":{\"Status\":26}\n");
					WDTR					
					FPRN_WriteTemplateByID(100, 570, fromAndroid); 
					WDTR
					rxAndroid = 1;
					CDC_FlushRxBuffer_FS();
					WDTR
					continue;
				}
			#endif
							
			#ifdef DEBUG_HNA
			if (fromAndroid[0]) {
				sprintf(globalBuff, "");				
				
				for (iHNA = 0; iHNA < 50; iHNA++)
					sprintf(globalBuff, "%s%02X ", globalBuff, fromAndroid[iHNA]);
				
				sprintf(globalBuff, "%s\n", globalBuff);
				
				for (iHNA=0; iHNA<strlen(globalBuff); iHNA++)
					pos_SendSerialData(&globalBuff[iHNA]);
			}
			#endif
				
			switch (fromAndroid[0]) {
			/* coomand ersale version */
				case 'V':  sendVersion();      	         rxAndroid = 1; break; //0x56 
				/* coomand mode update frimware */
			  case 'u':  updateFirmware();       	     rxAndroid = 1; break;
				/* coomand send CPU ID */
				case 'P':  GetCPUiD();       	           rxAndroid = 1; break;
				/* coomand mode update Baudrate */
			  case 'B':  configBaudRate();             rxAndroid = 1; break;
				/* coomand software reset */
				case 'r':  send("{\"Recive\":{\"Status\":70}}\n"); HAL_Delay(10); NVIC_SystemReset(); break; //0x72
				/* coomand set time */
				case 0x80: setTimeDate();                rxAndroid = 1; break; 
				/* coomand send time to tablet */
				case 0x81: getTimeDate();                rxAndroid = 1; break; 
				#if defined AutoStart
				case 0x01: tabletOn();									 rxAndroid = 1; break; 
				case 0x02: updateFirmware();             rxAndroid = 1; break; 
				#endif
				
				case 0x03: 
					#ifdef PN532_Routine 
						PN532_GetFirmwareVersion(&pn532, versionPn532,1); 
					#endif
				  #ifdef RC522_Routine
						printf("{\"C\":{\"ReaderType\":RC522,\"Version\":%d}}\n",Read_MFRC522(VersionReg));				  
				  #endif
					rxAndroid = 1; 
					break;
				
				case 0x09: 
					printf("{\"Mem\":{\"Version\":0x%X}}\n",SPI_Flash_ReadID()); rxAndroid = 1; break;
				case 0x08: tabletOff();                  rxAndroid = 1; break; 
			#ifdef pos
				/* coomand ersal mablagh roye pos */
				case 0x70:  Pos();                     rxAndroid = 1;   break;
			#endif
				
			#ifdef Printer
				case 'P':  PrinterProcess();           rxAndroid = 1; break;
			#endif
				
			#ifdef ExecTransaction
				/* coomand goroh card */
				case 'h':  RC522WriteGpCardCheck();      rxAndroid = 1; break; //0x68
				/* Command charge  */
				case '*':  flagRc522 = 1;                rxAndroid = 1; break; //0x2A
				/* Command  discharge */
				case '#':  decreaseFlag = 1;             rxAndroid = 1; break; //0x2A
				/* Command daryafte etebar card */
				case 'E':  etebarCheck();                rxAndroid = 1; break; //0x45 
				/* Command gozaresh Card */
				case 'g':  ReadCardTable(fromAndroid[1]);rxAndroid = 1; break; //0x67
				/* Command sefr krdn etebar card */
				case 'Z':  ziroEtebarCheck();            rxAndroid = 1; break; //0x5A
				/* Command sefr krdn Gift card */
				case 'G':  ziroGiftCheck();              rxAndroid = 1; break; //0x5A
				//Reset the main credit and gift
				case 'R':  resetCredit(); 							 rxAndroid = 1; break; //0x52
				/* Command cansel kardan charg/discharge */
				case 'c':  canselCheck();                rxAndroid = 1; break; //0x63
				//Is it possible to use Gift to decrease?
				case 0x11: useGift = fromAndroid[1] == 0x01; rxAndroid = 1; break; 
				#ifdef cardAccDis	
				case 0x12: cardDeliveryProcess();        rxAndroid = 1; break;
				#endif
			#endif

			#if defined (ExecTransactionBus) || defined (ExecTransactionBCU) || defined(ExecTransactionBD) || defined(ExecTransactionAPARK)
				/* Command daryafte etebar card */
				case 'E':  etebarCheck();                rxAndroid = 1; break; //0x45								
				/* Command gozaresh Card */
				case 'g':  ReadCardTable(fromAndroid[1]);rxAndroid = 1; break; //0x67
				#ifdef ExecTransactionBus
				/* Command charge and discharge */
				case '*':  flagRc522=1;                             rxAndroid = 1; break; //0x2A				
				/* Command  discharge */
				case '#':  decreaseFlag=1;                          rxAndroid = 1; break; //0x23
				/* Command sefr krdn etebar card */
				case 'Z':  ziroEtebarCheck();                       rxAndroid = 1; break;	//0x5A				
			  case 0x07: rwGroupPrice(1,fromAndroid[1],sernum);   rxAndroid = 1; break; //0x07
				#endif
				
				/* Command cansel kardan charg/discharge */
				case 'c':  canselCheck();                rxAndroid = 1; break; //0x63				
				#ifdef CoordinatedOldBCU			
					case 0x04: SendConfig();                 rxAndroid = 1; break;
				#endif
				#if defined (ExecTransactionBCU) || defined(ExecTransactionAPARK)
				case 0x05: receiveConfig();                rxAndroid = 1; break;
				case 0x06: sendConfig();                   rxAndroid = 1; break;
				case 'S':  processCardFlag();              rxAndroid = 1; break;
				case 'O':  processCardFlagOff();           rxAndroid = 1; break;
				case 'o':  processReciveTrans();           rxAndroid = 1; break;		
//				case 'M':  ExecTransactionWithNet();       rxAndroid = 1; break;	//4D
//				case 0x08: sendTransactionFromMemory();    rxAndroid = 1; break;
//				//case 0x09: saveTransactionInMem();         rxAndroid = 1; break;
//				case 0x10: TranactionWithOutSerial = 0;    rxAndroid = 1; break;
//				case 0x11: deleteTransactionInMem();       rxAndroid = 1; break;
				#endif
			#endif		
					
			/* Command daryafte Serial Card */			
			case 's':  serialSendCheck();            rxAndroid = 1; break; //0x73
			#if !defined(GATEROADBLOCK)	
			/* Command on krdn relay 1 */
			case 'n':  relayState('n','1');          rxAndroid = 1; break; //0x6E
			/* Command off krdn relay 1 */
			case 'f':  relayState('f','1');          rxAndroid = 1; break; //0x66 
			/* Command on krdn relay 2 */
			case 'N':  relayState('n','2');          rxAndroid = 1; break; //0x4E 
			/* Command on krdn relay 2 */
			case 'F':  relayState('f','2');          rxAndroid = 1; break; //0x46
			#endif
			
			#if defined(GATEROADBLOCK)
			case 'o':  gateRoadBlockProcess();       rxAndroid = 1; break; //0x6f
      #endif	
			
			#ifdef atlas
			case 'A':  atlasSendIDProcess(1);        rxAndroid = 1; break; //0x41
			case 'a':  atlasSendIDProcess(0);        rxAndroid = 1; break; //0x61
			#endif
			
			#ifdef Max30102
			case 'M': testConnectionMax30102();     rxAndroid = 1; break;
			#endif
			//--------------------------------------------finger	
			#ifdef finger
				/* Command Delete all template finger */
				case 'D':  DeleteAllTemplateProcess();   rxAndroid = 1; break; //0x44
				/* Command enroll finger */
				case 'e':  enrollFlag = 1;               rxAndroid = 1; break; //0x65
				/* Command Delete template by ID */
				case 'd':  deleteTemplateFlag = 1;       rxAndroid = 1; break; //0x64
				/* Command identifyOff */
				case 'I':  identifyOffProcess();         rxAndroid = 1; break; //0x49
				/* Command identify */
				case 'i':  identifyProcess();            rxAndroid = 1; break; //0x69
				/* Command Cansel Finger */
				case 'C':  cancelProcess();              rxAndroid = 1; break; //0x43
				/* Command empty ID */
				case 'G':  FPRN_GetEmptyID();            rxAndroid = 1; memset(fromAndroid,'\0',sizeof(fromAndroid)); break; //0x47
				/* Command Test connection finger */
				case 'T':  testConnection();             rxAndroid = 1; memset(fromAndroid,'\0',sizeof(fromAndroid)); break; //0x54
				/* Command Read template by ID */
				case 'R':  readTemplateFlag =1;          rxAndroid = 1; break; //0x52 // Get Empty	sprintf(reciveStatus,"Recive Status:%d\n",received); send(reciveStatus);for (char i=0;i<20;i++) reciveStatus[i] = 0;
				case 'v':  GetFrimware();                rxAndroid = 1; break; 
		#endif	
			
			#ifdef DS7
			case '@': case 0xAF :  ds7Proccess();  rxAndroid = 1; break; //0x40
			#endif
			
			#ifdef ReadWrite
			case 'g':  rwGroupParking(1,fromAndroid[1],sernum);   rxAndroid = 1; break; //0x67
			//case 'm':  DeviceEnteringExitingMode(fromAndroid[1]); rxAndroid = 1; break; //0x6d
			#ifdef remote
			case 't':  remoteCode();                              rxAndroid = 1; break;
			#endif
			
			#ifdef virtualRemote
			case 0x26:  genrateVirtualRemoteCode();      rxAndroid = 1; break;
			#endif
			
			#endif
			
			#if (deviceType==AutoPark)
			case 'm':  DeviceEnteringExitingMode(fromAndroid[1]); rxAndroid = 1; break; //0x6d
			case 'd':  sendDeviceMode();                          rxAndroid = 1; break; 
			case 'e':  WriteExitMode();                           rxAndroid = 1; break; 
			case 0x12: exitEnterWithDis();                        rxAndroid = 1; break; 
			case 0x13: Invalid_ValidUser();                       rxAndroid = 1; break; 
			//== Acc
			case 0x33: CardAcceptorMoveToRearProccess();          rxAndroid = 1; break; 
			case 0x30: CardAcceptorMoveToFrontProccess();         rxAndroid = 1; break; 
			case 0x31: CardAcceptorStatusProccess();              rxAndroid = 1; break;
			//== Dis
			case 0x44: DispenserToReaderProccess();               rxAndroid = 1; break;
			case 0x52: GetCardDispenserStatusProccess();          rxAndroid = 1; break;
			case 0x45: DispenserToCustomerProccess();             rxAndroid = 1; break;
			case 0x43: DispenserToRecyclebinProccess();           rxAndroid = 1; break;
			#endif
//ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss			
			case 'Q':  reSendLastTransaction();	                  rxAndroid = 1; break;
			case 'q':  loadTransactionRecord();	                  rxAndroid = 1; break;
      case 'w':  trHeadSend();	                            rxAndroid = 1; break;
			//case 'a':  preSernum[0] = 0x00; 											rxAndroid = 1; break;
			case 0xfe: DeletetrHead(); 														rxAndroid = 1; break; 
			case 0x10: lastTransactionSent = 0; 									rxAndroid = 1; break;

			#if (deviceType==Appointment)
				#ifdef remote
				case 't':  remoteCode();                              rxAndroid = 1; break;
				#endif			
			#endif
			default: /* Invaid Command */ 				
				send("{\"Receive\":{\"Status\":19}}\n");
				state 		= 19; 
				rxAndroid = 0; 
			
				#ifndef MaxReciveBuf
				memset(fromAndroid,'\0',sizeof(fromAndroid));
				#endif
			
				#ifdef MaxReciveBuf
				CDC_FlushRxBuffer_FS();
				#endif
			
				CDCReceiveLen = 0;
				break;
		  }//*************************************switch end*********************
			
			state 				= 19;
			rxAndroid 		= 0;	
			CDCReceiveLen = 0;

			#ifdef MaxReciveBuf
				rxBufferHeadPos = 0;
				rxBufferTailPos = 0;
			#endif
			HAL_Delay(1);
		}//if
		
		//-----------------------------------finger
		#ifdef finger
			if (IdentifyFlag)		
				FPRN_Identify(); 
			
			if (deleteTemplateFlag)
				deleteTemplateProcess();
			
			if (enrollFlag)
				enrollProcess();

			if (readTemplateFlag)
				readTempalteProcess();
		#endif	
		//---------------------------------------
		#ifdef ExecTransaction
			if (flagRc522)
				RC522ExecTransactionCheck();
			if (decreaseFlag) {
				RC522ExecDecreaseTransactionCheck();
				decFlag = 1;
			}
		#endif
		
		#ifdef ExecTransactionBus
			if (flagRc522) 
				RC522ExecTransactionCheck();
		  if (decreaseFlag)
			  RC522ExecDecreaseTransactionCheck();
		#endif
			
		#if defined(ExecTransactionBCU) || defined (ExecTransactionAPARK)
			if (Tick>5000) //5 sec
			{
			  #if defined LCD
					HAL_RTC_GetTime(&hrtc,&time,RTC_FORMAT_BIN);
					if (time.Minutes != oldMin)
						showDateTime();					
			  #endif
				Tick = 0;
				fromAndroid[0] = 0;
				send("{\"Request\":{\"Status\":0}}\n");
				rTick = HAL_GetTick();
				while (1)
				{
					WDTR
					if (HAL_GetTick() - rTick > 2000) 
					{
						ack = 0;
						break;
					}
					#if defined(MaxReciveBuf) 
					if (rxBufferHeadPos)
					#else
					if (CDCReceiveLen)
					#endif
					{
						if (fromAndroid[0] == 'v')
						{
							ack = 1;
							fromAndroid[0] = 0;
							CDCReceiveLen = 0;
							
							#if defined(MaxReciveBuf) 
							rxBufferHeadPos = 0;
							#endif
							
							send("{\"Request\":{\"Status\":1}}\n");
							break;
						}
					}
					ack = 0;
				}
				if (flagRc522 == 0)
					send("{\"Config\":{\"Status\":30}}\n");	
//				if (TranactionWithOutSerial)
//				{
//					sprintf(buffRC522,"{\"Transaction\":{\"Type\":\"Offline\":%d}}\n",TranactionWithOutSerial); // tedad transasction hai k dar zaman connect nabodane serial zade shode ast 
//					send(buffRC522);
//				}
			}	
			
			#ifdef CoordinatedOldBCU
				if (uart1Len>2047)
				{	
					ProcessPacket(0);
					memset(uart1Buff,0,uart1Len);
					HAL_Delay(1000);
					uart1Len=0;
					HAL_Delay(1000);
					__HAL_UART_ENABLE_IT(&huart3, UART_IT_RXNE);
				}
				__HAL_UART_ENABLE_IT(&huart3, UART_IT_RXNE);
			#endif
		#endif
		
		#ifdef ExecTransactionBD
			if (flagRc522)
				RC522ExecTransactionCheck();
		#endif
			
		#ifdef atlas
			if (rx && atlasSendFlag) 
			{
				HAL_Delay(6);
				send(globalBuff);
				rx = 0;
			}
			if (!atlasSendFlag) 
			{
				wiegandLen = 0;
				faceID = 0;
				memset(wiegandRead,'\0',sizeof(wiegandRead));
			}
				
		#endif
			
		#ifdef pos		
			#if (deviceType == Sport) || defined(pos)
				__HAL_UART_ENABLE_IT(&huart3, UART_IT_RXNE);
				if (uart3Len) {
					HAL_Delay(300);
					sendResponsePos(uart3buf, uart3Len);
					#ifdef DEBUG_HNA
					unsigned char hna = 0;
					for (hna = 0; hna < uart3Len; hna++)
						pos_SendSerialData(&uart3buf[hna]);
					#endif
					uart3Len = 0;
					memset(uart3buf,'\0', sizeof(uart3buf));
				}		
			#else
				if (bufferLen) {
					
					HAL_Delay(300);
				  sendResponsePos(GetResponse,bufferLen);
					bufferLen = 0;
					memset(GetResponse,'\0',sizeof(GetResponse));
				}
			#endif
		#endif
			
		#ifdef tarazo
			if (uart1Len) {
				#ifdef DEBUG_HNA
				sprintf(globalBuff, "Tarazo : %s\n", uart1Buff);
				sendDebug(globalBuff);
				#endif
				HAL_Delay(10);
				WeightProccess();
				uart1Len = 0;
			}
		#endif		
		
		#if defined(heightHum)
		/*
		if (uart3Len)
		{
			HAL_Delay(10);
			heighHumProccess();
		}*/
		if (SoftUartRxAlavailable(0)>0)
		{
			HAL_Delay(10);
			heighHumProccess();
		}
		#endif

		#if (deviceType==AutoPark)

#ifdef    NoDispenser


											if (NOdispenser){
											NOdispenser=0;
											exitEnterWithDis();//sssssssssss
												  continue;
											}
											else{
												NOdispenser=1;
													MFRC522_Init();//sssssssssss
		                  }//ssssssssssssss
			
#endif	

		if (EnteringExitingMode==0)
			if (READKey == 1)
			{
				send("{\"Receive\":{\"Status\":85}}\n");
				HAL_Delay(100);
				/*
				giveCardWithKey = 1;
				exitEnterWithDis();
				HAL_Delay(10);
				giveCardWithKey=0;
				*/
			}
			
		if (uart3Len)
		{
			__HAL_UART_ENABLE_IT(&huart3, UART_IT_RXNE);
		}			
		if (Tick>500) //0.5 sec
		{
			Tick = 0;
			if (configFlag==0)
				send("{\"Config\":{\"Status\":30}}\n");	
//			if (reciveAck)
//				send("{\"Config\":{\"Status\":30}}\n");	
			if ((configFlag == 1) && (EnteringExitingMode == 1) && (reciveAck == 0))				
				switch (Get_CardAcceptorStatus())
				{					
					case 0x46: send("{\"ACC\":{\"Status\":70}}\n"); break; // 
					case 0x47: send("{\"ACC\":{\"Status\":71}}\n"); break;
					case 0x48: send("{\"ACC\":{\"Status\":72}}\n"); break;
					case 0x49: send("{\"ACC\":{\"Status\":73}}\n"); break;
					case 0x4A: exitEnterWithDis(); 									break;	// card reader 
					case 0x4B: send("{\"ACC\":{\"Status\":75}}\n"); break;
					case 0x4C: send("{\"ACC\":{\"Status\":76}}\n"); break;
					case 0x4D: send("{\"ACC\":{\"Status\":77}}\n"); break;
				}
		}		
		#endif
		
		#ifdef remote		
			if (DetectTruePacket)
			{
				HAL_NVIC_DisableIRQ(TIM2_IRQn);
				HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
				sprintf(globalBuff,"{\"Remote\":{\"Code\":%u,\"button\":%d}}\n",learncode,press);
				send(globalBuff);
				DetectTruePacket=0;
				tickLearnCode=0;
				memset(Tmp,0,sizeof(Tmp));
				enableIRQ = 1;
			}			
			if ((tickLearnCode>75) && (enableIRQ))
			{
				enableIRQ = 0;
				HAL_NVIC_EnableIRQ(TIM2_IRQn);
				HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
			} 
		#endif
		#if defined UHF
		//------- UHF 1 
			if (!waitToReadUHF1)
			{
				if (tickForUHF1>WaitTimeReadUHF) //10 sec
				{
					tickForUHF1 = 0;
					waitToReadUHF1 = 1;
				}
			}
			
			if (waitToReadUHF1)
			{
				tickForUHF1 = 0;
				if (!serialSendFlag) 	
					readCardUHF(1);
			}
			//----------UHF 2
			if (!waitToReadUHF2)
			{
				if (tickForUHF2>WaitTimeReadUHF) //10 sec
				{
					tickForUHF2 = 0;
					waitToReadUHF2 = 1;
				}
			}			
			if (waitToReadUHF2)
			{
				tickForUHF2 = 0;
				if (!serialSendFlag) 	
					readCardUHF(2);
			}					
		#endif
		
		if (toggleLed > 50) {
			#if defined LCD
			 MX_USB_DEVICE_Init();
			if (refreshPage)
			{
				showWellCome();
				showMainPrice(GrouhPrice[0],1);
				refreshPage = 0;
			}
			#endif
//			MX_USB_DEVICE_Init();
			toggleLed = 0;
			ToggleLED
		}
//****************************************************************

    if (ISO14443_SingleTagSelect(sernum) 	 == HAS_ERROR)
      if (ISO14443_SingleTagSelect(sernum) == HAS_ERROR) { //اگر کارتي بر روي دستگاه نيست
				if (cardReadStep == 1) { //آيا کاهش اعتبار با موفقيت انجام شد؟
					now  = HAL_GetTick();
					cardReadStep = 2; // اگر کارت تازه عمليات شده، از روي دستگاه برداشته شد از اين به بعد فقط تا ? ثانيه بعد مي تواند دوباره کارت بزند
				}
				else if (cardReadStep == 0) //آيا در مرحله اول خواندن کارت هستيم؟
					preSernum[0] = preSernum[1] = preSernum[2] = preSernum[3] = 0;

				continue;
			}

		#ifdef ONEREADCARD
			if ((sernum[0] == preSernum[0]) && 
					(sernum[1] == preSernum[1]) && 
					(sernum[2] == preSernum[2]) && 
					(sernum[3] == preSernum[3])) {

					if (cardReadStep == 2) {// آيا کارت يکبار کاهش داده شده، از دستگاه برداشته شده و اکنون دوباره زده شده؟
						if ((HAL_GetTick() - now) <= 200) { //اگر کارت تازه اعتبار کم شده بيش از 4 ثانيه دور نگه داشته شد
							now = HAL_GetTick();
							continue;
						}
						else
							cardReadStep = 0;
					}
					else
						continue;
			}
			cardReadStep = 0;
		#endif

		#ifdef NFC
		if (PN532_Ntag2xxReadBlock(&pn532,NtagReadZiroBlock,0) == 0)
		{
			for (unsigned char count = 0;count<4;count++)
				preSernum[count] = sernum[count];
			ProcessNFCCard();
			continue;
		}	
			
		#endif

		
//   	sprintf(buffRC522,"{\"C\":{\"continue\":%u,\"1\":%u,\"P\":%u}}\n",state,etebar,preetebar);//sssssssssssssssssssssss
//										      send(buffRC522);
		if (ISO14443_LoginE2(0,0x10)) {
//			                  	sprintf(buffRC522,"{\"C\":{\"sadegh\":%u,\"3\":%u,\"P\":%u}}\n",state,etebar,preetebar);//sssssssssssssssssssssss
//										      send(buffRC522);
			if (ISO14443_SingleTagSelect(sernum))
				if (ISO14443_SingleTagSelect(sernum))
					continue;
			if (ISO14443_LoginE2(0, 0x10))                                                                                                                                                                                                                     
				continue;
    }	
		//----------------------------------------------//if read block 
		//----------------------------------------------
    if (!ISO14443_ReadBlock(1, buffRC522)) {
			for (unsigned char count=0; count<4; count++)
				preSernum[count] = sernum[count];
				
			for (unsigned char i=0; i<16; i++)
					dataCard[i] = buffRC522[i];
			
			crcCard = 0xC9;
			for (unsigned char i=0; i<15; i++)
					crcCard ^= dataCard[i];
			
		if (0) { //HNA
			//if (crcCard == dataCard[15]) {
				for (unsigned char i=0; i<15; i++) 
					dataCard[i] ^= KeyTable[sernum[i % 4] / ((i / 4) + 1)] ^ sernum[3 - (i % 4)];
				
				crcCard = 0xE2;  
				for (unsigned char i=0; i<14; i++)
					crcCard ^= dataCard[i];
				
				if (crcCard != dataCard[14]) {
					for (unsigned char i=0; i<16; i++)
						dataCard[i] = buffRC522[i];
					crcCard = dataCard[15] + 1; 
				}
				else {
					uc  = dataCard[1]; uc <<= 8;
					uc += dataCard[0];
					cardId  = dataCard[5]; cardId <<= 8;
					cardId += dataCard[4]; cardId <<= 8;
					cardId += dataCard[3]; cardId <<= 8;
					cardId += dataCard[2];
					crcCard = dataCard[15]; 
				}
		}
		else if (1) { //HNA
		//else if (crcCard != dataCard[15]) {
			uc  = buffRC522[0];
			uc += (buffRC522[1] << 8);
			cardId  =  buffRC522[2];
			cardId += (buffRC522[3] << 8);
			cardId += (buffRC522[4] << 16);
			cardId += (buffRC522[5] << 24);
			if ((uc & 0x8000) == 0x8000) {
				uc = uc & 0x7FFF;
				uc = uc ^ 0x6A29;
				cardId = cardId ^ 0xB2A6;
			}
		}
		if (serialConnectWithTab) {
					#ifdef ExecTransactionBD
						ProcessCard(2,sernum);
						if (serialSendFlag)
						{
							sprintf(buffRC522,"{\"C\":{\"ID\":%d,\"UC\":%d}}\n",cardId,uc); // ID card , ersal UC card
							send(buffRC522);  //ersal ID card
							serialSendFlag = !serialSendFlag;
						}
					#endif
					#if defined(ExecTransactionBCU) || defined(ExecTransactionAPARK)
						if (flagRc522 == 0)
						{
							send("{\"Config\":{\"Status\":30}}\n"); // no config 
						}
						else if ((flagRc522) && (etebarFlag) && (zeroEtebarFlag))
						{
							if (readerTick>1000)
							{
								readerTick = 0;
								if (GpFlag)
								{
									if (ack)
										ProcessCard(sernum);
									else
										Tick = 15000;
								}
								else 
									send("{\"C\":{\"Type\":\"ProcessCardOff\"}}\n");
							}
						}
						if (serialSendFlag)
						{
							sprintf(buffRC522,"{\"C\":{\"ID\":%d,\"UC\":%d}}\n",cardId,uc); // ID card , ersal UC card
							send(buffRC522);  //ersal ID card
							serialSendFlag = !serialSendFlag;
						}
					#endif						
					#ifndef ExecTransaction
					#ifndef ExecTransactionBus
					#ifndef ExecTransactionBCU
					#ifndef ExecTransactionAPARK
						if (!serialSendFlag) { 
							#if (deviceType==Parking)
							rwGroupParking(0,0,sernum);
							
							#elif (deviceType==AutoPark) 
//rwGroupParking(0,0,sernum);
							if (configFlag)
								rwGroupParking(0,0,sernum);
//ProcessCard(sernum);	
							else if (configFlag==0)
								send("{\"Config\":{\"Status\":30}}\n");
							#else // ersal id , uc for IO And Shop
							sprintf(buffRC522,"{\"C\":{\"ID\":%d,\"UC\":%d}}\n",cardId,uc); // ID card , ersal UC card
							send(buffRC522);  //ersal ID card	
							#endif
						}
						else {
							#if (deviceType==AutoPark) 
							infoCardForParking();
							#endif
							//send("{\"C\":{\"Type\":\"ProcessCardOff\"}}\n");
						}
					#endif
					#endif
					#endif
					#endif
					
					#ifdef ExecTransaction
						if (!serialSendFlag) {/* Send etebar,hedie,id,uc,model */
							etebarFlag =!etebarFlag;
							//if (decFlag) 
								if ((HAL_GetTick() - ti) >= 1000) { //HNA
									decFlag = 0;
									ti = HAL_GetTick();
									readGroupOfCard(sernum);	//ecard+-
								}
						}

						if ((!zeroEtebarFlag)) { /* sefr kardan etebar */ 
//							 sprintf(buffRC522,"{\"C\":{\"zeroEtebarFlag\":%u,\"2\":%u,\"P\":%u}}\n",cardId,uc,preetebar);//sssssssssssssssssssssss
//																send(buffRC522);
							MINEtebar = 0;
							if (cardId == ziroEtebarID) {
								if (!GetCardEtebar(&etebar)) {
									state = Old_ExecTransaction(19,&ziroEtebarID,&ziroEtebarUC,etebar,MINEtebar,MAXEtebar,deviceID,&etebar,&preetebar,&lastDevice,&LastoP,&oPIndex);
									if ((state == 9) || (state == 1)) state = Old_ExecTransaction(19,&ziroEtebarID,&ziroEtebarUC,etebar,MINEtebar,MAXEtebar,deviceID,&etebar,&preetebar,&lastDevice,&LastoP,&oPIndex);
									if (!GetCardEtebar(&etebar)) {
										sprintf(buffRC522,"{\"C\":{\"Status\":%u,\"Etebar\":%u,\"PreEtebar\":%u}}\n",state,etebar,preetebar);
										send(buffRC522);
									}
								}
							}
							else {
								sprintf(buffRC522,"{\"C\":{\"Status\":10,\"Etebar\":%u,\"PreEtebar\":%u}}\n",etebar,preetebar);
								send(buffRC522);
							}
							zeroEtebarFlag=!zeroEtebarFlag;
							serialSendFlag = 0;
						}
					#endif

					#if defined (ExecTransactionBus) || defined (ExecTransactionBCU) || defined(ExecTransactionBD) || defined(ExecTransactionAPARK)
					  #if defined (ExecTransactionBus)
							if (!GetCardEtebar(&etebar)) {
								send("Here to show the result:\n");
								sprintf(buffRC522, "{\"C\":{\"ID\":%d,\"UC\":%d,\"Etebar" \
									"\":%u}}\n", cardId, uc, etebar);
								send(buffRC522); 
							}
						#endif
						if (!etebarFlag) /* Send etebar */
						{
							#if !defined(ExecTransactionAPARK)
							if (!GetCardEtebar(&etebar)) {							 
								memset(buffRC522,'\0', sizeof(buffRC522));
								sprintf(buffRC522,"{\"C\":{\"ID\":%d,\"UC\":%d,\"Etebar\":%u}}\n",cardId,uc,etebar);				
								send(buffRC522); 
							}
							#else
							if (!GetCardEtebar1(&etebar,&Hetebar))
							{									
								sprintf(globalBuff,"{\"C\":{\"ID\":%d,\"UC\":%d,\"Etebar\":%u,\"Hedie\":%u}}\n",cardId,uc,etebar,Hetebar); 					
								send(globalBuff);
								readerTick = 0;
							}						
							#endif
							etebarFlag =!etebarFlag;
						} 
						if ((!zeroEtebarFlag))  /* sefr kardan etebar */ 
						{
							MINEtebar = 0;
							if (!GetCardEtebar(&etebar))
							{
								state = Old_ExecTransaction(2,&cardId,&uc,etebar,MINEtebar,MAXEtebar,deviceID,&etebar,&preetebar,&lastDevice,&LastoP,&oPIndex);
								if (state == 9) state = Old_ExecTransaction(2,&cardId,&uc,etebar,MINEtebar,MAXEtebar,deviceID,&etebar,&preetebar,&lastDevice,&LastoP,&oPIndex);
								if (!GetCardEtebar(&etebar))
								{
									sprintf(buffRC522,"{\"C\":{\"Status\":%u,\"Etebar\":%u}}\n",state,etebar);
									send(buffRC522);
									zeroEtebarFlag=!zeroEtebarFlag;
									readerTick = 0;
								}
							}
						}
					#endif
//					#if (deviceType==BCU) || (deviceType==BD)
//					HAL_Delay(1000);
//					#endif
					#if (deviceType!=BCU) && (deviceType!=BD)	
						#ifdef PN532_Routine 
							HAL_Delay(100);
						#else
							HAL_Delay(250);
						#endif				
					#endif

				}	
				else 
					ledBlik(10,50);
				//MFRC522
			//MFRC522_Init();
		}//if read block 

	}//while 
} //main 

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE
                              |RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_USB;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
  /** Enables the Clock Security System
  */
  HAL_RCC_EnableCSS();
}

/**
  * @brief IWDG Initialization Function
  * @param None
  * @retval None
  */
static void MX_IWDG_Init(void)
{

  /* USER CODE BEGIN IWDG_Init 0 */

  /* USER CODE END IWDG_Init 0 */

  /* USER CODE BEGIN IWDG_Init 1 */

  /* USER CODE END IWDG_Init 1 */
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_256;
  hiwdg.Init.Reload = 4095;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN IWDG_Init 2 */

  /* USER CODE END IWDG_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef DateToUpdate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */
  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
  hrtc.Init.OutPut = RTC_OUTPUTSOURCE_ALARM;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
//  sTime.Hours = 0;
//  sTime.Minutes = 0;
//  sTime.Seconds = 0;

//  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
//  {
//    Error_Handler();
//  }
//  DateToUpdate.WeekDay = RTC_WEEKDAY_MONDAY;
//  DateToUpdate.Month = RTC_MONTH_JANUARY;
//  DateToUpdate.Date = 1;
//  DateToUpdate.Year = 0;

//  if (HAL_RTC_SetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BIN) != HAL_OK)
//  {
//    Error_Handler();
//  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */
	/*
	
	force define 
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
	
	*/
  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW; 
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
	#ifdef LCD
	hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
	#endif
	
	#ifdef RC522_Routine
	hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
	#endif


  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;

	
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
	
  #ifdef LCD
	hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi1.Init.CRCPolynomial = 10;	
	#else
	
	hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_ENABLED;
	hspi1.Init.CRCPolynomial = 10-1;
	#endif
  
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */
	/*
	
	force define 
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
	
	*/
  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_HIGH; 
  hspi2.Init.CLKPhase = SPI_PHASE_2EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
	hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi2.Init.CRCPolynomial = 10;	
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}
/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
#ifdef remote 
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = (72000000/1000000)-1;   
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 1000;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}
#endif
#ifdef virtualRemote 
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 72-1;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 0xffff-1;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

#endif
/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */
	 // Prescaler = ((0.2/bound)*72*(1000000)) - 1; 
  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = ((0.2/bound)*72*(1000000)) - 1;   
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 1;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
#ifndef atlas
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
	#ifdef tarazo
  huart1.Init.BaudRate = 9600;
	#endif
	
	#if (deviceType==BCU) || (deviceType == BD)
  huart1.Init.BaudRate = 57600;
	#endif
	
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}
#endif
/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void) {
  huart2.Instance = USART2;
	#if (deviceType == Shop) || (deviceType == BCU) // for finger 
  huart2.Init.BaudRate = 9600; // for finger 
	#else
		#ifdef finger
		huart2.Init.BaudRate = 115200;//r finger 
		#else
		huart2.Init.BaudRate = 19200; // for pos 
		#endif
	#endif	
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
    Error_Handler();
}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void) {
		
  huart3.Instance = USART3;
	#ifdef Printer
  huart3.Init.BaudRate = 19200;
	#endif
	
	/*
	#ifdef heightHum
  huart3.Init.BaudRate = 9600;
	#endif */	
	
	#ifdef cardAccDis
	huart3.Init.BaudRate = 9600;
	#endif
//	           	#ifdef serialport
//		             huart3.Init.BaudRate = 9600;  
//				      #endif
	#if (deviceType == Shop || deviceType == Sport || deviceType == AmusementPark)
	huart3.Init.BaudRate = 19200; // for pos 
	#endif
	
	#ifdef UHF
	huart3.Init.BaudRate = 115200;
	#endif
	
  huart3.Init.WordLength 	 = UART_WORDLENGTH_8B;
  huart3.Init.StopBits 		 = UART_STOPBITS_1;
  huart3.Init.Parity 			 = UART_PARITY_NONE;
  huart3.Init.Mode 				 = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl 	 = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK) 
    Error_Handler();
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}



/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, SPI1_CS_Pin|relay1_Pin|led_Pin|relay2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, readetRst_Pin, GPIO_PIN_RESET);
  /*Configure GPIO pins : SPI1_CS_Pin relay1_Pin led_Pin relay2_Pin */
  GPIO_InitStruct.Pin = SPI1_CS_Pin|relay1_Pin|led_Pin|relay2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : readetRst_Pin ds7data_Pin */
	
  GPIO_InitStruct.Pin = readetRst_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	#if defined DS7
	HAL_GPIO_WritePin(GPIOB, ds7data_Pin, GPIO_PIN_RESET);	
	GPIO_InitStruct.Pin = ds7data_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);	
	#endif
	
	#if defined heightHum
	GPIO_InitStruct.Pin = ds7data_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	#endif
	
	
	#ifdef atlas
  /*Configure GPIO pins : PA0 PA1 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);
	#endif
	
//	#ifdef Max30102
#if (deviceType == BCU) || (deviceType == BD)
	GPIO_InitStruct.Pin = Door_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(Door_GPIO_Port, &GPIO_InitStruct);

	HAL_NVIC_SetPriority(EXTI9_5_IRQn , 1, 1);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
	
//	#endif
  HAL_GPIO_WritePin(GPIOA, DERE_Pin, GPIO_PIN_RESET);
  GPIO_InitStruct.Pin = DERE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
#endif

#if defined LCD
	HAL_GPIO_WritePin(TFTRS_GPIO_Port, TFTRS_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(TFTRST_GPIO_Port, TFTRST_Pin, GPIO_PIN_RESET);
	
  GPIO_InitStruct.Pin = TFTRS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(TFTRS_GPIO_Port, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = TFTRST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(TFTRST_GPIO_Port, &GPIO_InitStruct);
#endif

#if (deviceType == AutoPark) 
  HAL_GPIO_WritePin(GPIOA, CS2_Pin, GPIO_PIN_SET);
  GPIO_InitStruct.Pin = CS2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
//  HAL_GPIO_WritePin(GPIOA, CS2_Pin, GPIO_PIN_RESET);
  GPIO_InitStruct.Pin = Key_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(Key_GPIO_Port, &GPIO_InitStruct);	
#endif

#if defined AutoStart
  HAL_GPIO_WritePin(PowerKey_GPIO_Port , PowerKey_Pin, GPIO_PIN_SET);
  GPIO_InitStruct.Pin   = PowerKey_Pin;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(PowerKey_GPIO_Port, &GPIO_InitStruct);
#endif

#ifdef remote
	GPIO_InitStruct.Pin = GPIO_PIN_8;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/* EXTI interrupt init*/
	HAL_NVIC_SetPriority(EXTI9_5_IRQn, 3, 0);
	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
#endif

#ifdef virtualRemote
  HAL_GPIO_WritePin(GPIOB , GPIO_PIN_9, GPIO_PIN_SET);
  GPIO_InitStruct.Pin = GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
#endif

#if defined GATEROADBLOCK

  GPIO_InitStruct.Pin = SNSUP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(SNSUP_GPIO_Port, &GPIO_InitStruct);	
	
	GPIO_InitStruct.Pin = SNSDOWN_Pin | IR2_Pin ;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin =  Relay_State_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(Relay_State_GPIO_Port, &GPIO_InitStruct);
	
	
#endif

  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);
  GPIO_InitStruct.Pin   = GPIO_PIN_4;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);

  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
  GPIO_InitStruct.Pin   = GPIO_PIN_12;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
}

/* USER CODE BEGIN 4 */
#if defined (Max30102) || defined(PN532_Routine)
static void MX_I2C1_Init(void) {

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
	hi2c1.Instance = I2C1;
	
	#ifdef PN532_Routine 
  hi2c1.Init.ClockSpeed = 400000;
	#else
	hi2c1.Init.ClockSpeed = 400000;
	#endif
	
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
	
	#ifdef PN532_Routine 
  hi2c1.Init.OwnAddress1 = 144;
	#else
	hi2c1.Init.OwnAddress1 = 0;
	#endif
	
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE; 
	hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}
#endif
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void) {
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1) {
		HAL_NVIC_SystemReset();
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line) {
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
