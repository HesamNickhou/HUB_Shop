#ifndef CONFIG_H
#define CONFIG_H
#include "main.h"
#include "stdio.h"
#include "stdlib.h"
#include "define.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "pn532_stm32f1.h"

#define SIZEDATA 9
#define SIZEDATADEVICE 5
#define size 600
#define bound 9600
#define STX 2
#define ETX 3
#define WDTR __HAL_IWDG_RELOAD_COUNTER(&hiwdg);
 
#define addFirmware           0
#define addConfig             0x80000
#define addIndicators         addConfig+4096
#define addCourses            addIndicators+4096
#define addResources          0x100000
#define addTransactions       0x200000
#define addPersonels          0x300000
#define addMessages           0x400000
#define addCalculationRules   0x500000
#define addVoicesEx           0x600000
#define addVoices             0x700000


#ifdef Sport
#define LEN_TRANSACTIONS      36
#endif

//sssss
/*
						 [ MEMORY STRUCTURE ]
									 W25Q64
									 
		 START			END         DESCRIPTION
		--------  --------		-------------------- 
		0x00000   0x80000     Firmware area
		0x80000	  0x100000    Configuration data	
		0x100000  0x200000		Resources area							 
		0x200000  0x300000    Transactions records
		0x300000  0x400000    Locations records
		0x400000  0x500000    Transaction backup
		0x500000  0x600000    Travel info
		0x600000  0x800000    Free memory 		

	*/

#ifdef NFC
uint8_t NtagRead[4];  
uint8_t NtagWrite[4] = {0xab,0xcd,0xef,0x12};
uint8_t NtagReadZiroBlock[4]; 

#endif
unsigned int trHead;
unsigned int trTail = 0;

unsigned char buffTransaction[300] = {0};
unsigned char sendingResult[300] 	 = {0};

unsigned char serialConnect=0;
unsigned char serialConnectWithTab=0;
unsigned char resetSystem=0;
unsigned char reset_cause=0;

	#ifdef finger
		 unsigned int idFR;
		 _Bool enrollFlag = 0;
		 _Bool deleteTemplateFlag = 0;
		 _Bool IdentifyFreeFlag = 0;
		 _Bool readTemplateFlag=0;
		 _Bool fingerITflag = 0;
	#endif
	
	#ifdef MaxReciveBuf
		 uint16_t rxBufferHeadPos = 0; // Receive buffer write position
		 uint16_t rxBufferTailPos = 0; // Receive buffer read position		
	#endif
	
	#ifdef UHF
	unsigned char waitToReadUHF1=1;
	unsigned char waitToReadUHF2=1;
	unsigned int  tickForUHF1=0;
	unsigned int  tickForUHF2=0;
	#define WaitTimeReadUHF 10000
	#endif
	
	#ifdef ReadWrite
	struct {
		unsigned char year;           
		unsigned char month;        
		unsigned char day;      
		unsigned char hour;      
		unsigned char min;
		unsigned char sec;
    unsigned char Entering_Exiting_Mode;		
		}ParkTime;
	//unsigned char EnteringExitingMode = 2;
	int len=0;
	unsigned int Tick;
	#endif
	
	#if(deviceType==AutoPark)
	unsigned char EnteringExitingMode = 0;
	unsigned char EnteringExitingModeMember = 0;
	unsigned int	_EE_SIZE = 64;
	unsigned int _EE_ADDR_INUSE   =  0x08005440;
	unsigned char _EE_FLASH_BANK   = FLASH_BANK_1;
	unsigned char _EE_PAGE_OR_SECTOR = 0;
	unsigned char config[3] = {0}; 
	unsigned char configFlag=0;
	unsigned int Tick=0;
	unsigned char enableReader2=0;
	unsigned char gpOfCard=0;
	unsigned char giveCardWithKey=0;
	unsigned char readCardInfoFlag=0;
	
	#define LEN_TRANSACTIONS      17
	#endif
	
	#ifdef Max30102
	int32_t hr_old = 1, spo2_old = 1;
	unsigned char ID;
	unsigned char HrSpo2Buffer[32];
	unsigned char max30102Flag = 1;
	#endif
	#ifdef remote 
		unsigned int HTime=0,LTime=0;
		unsigned int Tm=0;
		volatile unsigned char Tmp[24]={0};		
		unsigned char DetectTruePacket=0,enableIRQ=0;;
		unsigned int Time=0;
	  volatile unsigned int learncode = 0;
		//volatile unsigned int shiftDate = 0;
		unsigned char press = 0xff;
		#define RDATA     HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_8)
		unsigned int tickLearnCode=0;
		volatile unsigned char bufer[12]={0};
	#endif


	 _Bool rxITForFinger = 1;
	 _Bool IdentifyFlag=0;
	 
	#ifdef tarazo
		unsigned char uart1Buff[10];
		unsigned char uart1Len=0;
		int weight = 0;	
	#endif
//	unsigned char test =0;
	#if (deviceType==BCU) 		 
		unsigned int Tick=0,rTick=0,ack=0;		
		unsigned int GrouhPrice[10]; 
		unsigned int	_EE_SIZE = 64;
		unsigned int _EE_ADDR_INUSE   =  0x08005440;
    unsigned char _EE_FLASH_BANK   = FLASH_BANK_1;
    unsigned char _EE_PAGE_OR_SECTOR = 0;
		_Bool GpFlag=1;
		_Bool sendTransactionFlag=1;
		uint32_t virtualAddress = 0;
		_Bool saveTransactionInMemFlag=0;
		unsigned char TranactionWithOutSerial=0;
		const int grgSumOfDays[2][13] = {
                              {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365},
                              {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366}
                            };
		const int hshSumOfDays[2][13] = {
                              {0, 31, 62, 93, 124, 155, 186, 216, 246, 276, 306, 336, 365},
                              {0, 31, 62, 93, 124, 155, 186, 216, 246, 276, 306, 336, 366}
                            };
    typedef struct {	
				unsigned int  DeviceID;
				unsigned int  UC;
				unsigned int  BusID;
				unsigned int  OperatorID;
				unsigned long int MaxEtebar;
				signed char MinEtebar;
			  #if defined(ExecTransactionAPARK)
			  unsigned char hTransaction;
			  #endif
		}TConfig;
		TConfig Config;
		#define LEN_TRANSACTIONS      29
		#ifdef CoordinatedOldBCU
		  #define LastPacketsNum   40
			#define LABKHANDDOOR       164
			#define LastPacketsNum   40
			unsigned char uart1Buff[2048];
	    unsigned int uart1Len=0;
			unsigned char LastPackets1[LastPacketsNum][25];
			unsigned char LastPackets2[LastPacketsNum][25];
			unsigned int CurCardID=0;
			char RecoredsLen=32;
			unsigned int TransactionUID;
			unsigned long int Transactions,Transactions2;
      unsigned long int OffTransactions,OffTransactions2;
		  unsigned int DeviceID;
			unsigned int OperatorID;
			unsigned char Door1Statuses,Door2Statuses;
			unsigned short Door1Ver, Door1Release; 
      unsigned short Door2Ver, Door2Release; 
			unsigned char Door1Model,Door2Model;
			unsigned int DoorPrice;
			unsigned char BD1Ready=0,BD2Ready=0;
			signed char BD1ReadyTimer=0;
			signed char BD2ReadyTimer=0;
			unsigned int Door1Num;
      unsigned int Door2Num;
			unsigned int Door1CurNum;
      unsigned int Door2CurNum;
			unsigned char LastPacketsPos1=0,LastPacketsPos2=0;
			unsigned char ExtraStatus=0;
			unsigned char LastPacketsPos=0;
			unsigned char LastPackets[LastPacketsNum][25];
			unsigned long int OfflineStart;
		#endif
	#endif
	
	#if (deviceType == BD)
	
	#endif
	unsigned short memID = 0;
	char flag;
	char state=20;
  unsigned int CDCReceiveLen=0;
  unsigned char GetResponse[size];  // response from uart1
	unsigned char fromAndroid[size];	// data recive from android 
	short crc=0;
	//unsigned char crcBuff[SIZEDATADEVICE];
	
	unsigned int bufferLen=0;
	
	unsigned char sendTimeDate[60];
	
	/* // bit field 
	struct {
   unsigned char widthValidated : 3;
   unsigned char heightValidated : 3;
} status;   
	*/
	
	int type=3;
	unsigned int payMent,lastDevice=0;              
	signed long int MINEtebar;
	unsigned long int MAXEtebar;
	unsigned int deviceID;
	unsigned long int etebar=0;
	signed int Hetebar=0; 
	unsigned long int preetebar=0;
	unsigned char LastoP=0;
	unsigned char oPIndex=0;
	unsigned char Month=10,Day=2,Hour=17,Min=48,Sec=32;
	int Year=0;
	unsigned int configUc;
	extern unsigned char HediyeBuf[16];
	
	#ifdef atlas
	unsigned char wiegandRead[34]={0};
	unsigned char wiegandLen=0;
	unsigned int faceID=0;
	_Bool atlasSendFlag=1;
	_Bool rx;
	#endif
	
	#if defined(heightHum) || defined(cardAccDis) || (deviceType == Sport) || defined(UHF) || defined(pos)
	unsigned char uart3buf[300];
	unsigned int uart3Len = 0;
	
	#endif

	unsigned char globalBuff[400];

	_Bool etebarFlag = 1;
	_Bool serialSendFlag = 0;
	_Bool reciveDataFlag = 0;
	_Bool zeroEtebarFlag = 1;
	_Bool flagRc522 = 0;
	_Bool rxAndroid = 0;
	_Bool reciveAck = 0;
	_Bool execTransactionFlag=0;
	
  #if (deviceType==AmusementPark)
	unsigned long int ziroEtebarID=0,ziroEtebarUC=0,ziroEtebarTimeOut=0;
	unsigned char decreaseFlag=0;
	#define LEN_TRANSACTIONS      36
	#endif
	
	unsigned short toggleLed = 0;
	
	#if (deviceType == BusEtebar)	
	unsigned char decreaseFlag=0;
	#define LEN_TRANSACTIONS      28
	#endif
	
	unsigned char dataCard[32];
	unsigned char crcCard=0;
	const unsigned char KeyTable[256]=
	{
    0xf4, 0x41, 0x17, 0x27, 0xab, 0x9d, 0xfa, 0xe3, 0x30, 0x76, 0xcc, 0x02, 0xe5, 0x2a, 0x35, 0x62,
    0xb1, 0xba, 0xea, 0xfe, 0x2f, 0x4c, 0x46, 0xd3, 0x8f, 0x92, 0x6d, 0x52, 0xbe, 0x74, 0xe0, 0xc9,
    0xc2, 0x8e, 0x58, 0xb9, 0xe1, 0x88, 0x20, 0xce, 0xdf, 0x1a, 0x51, 0x53, 0x7c, 0x42, 0x84, 0x00,
    0x64, 0x6b, 0x81, 0x08, 0xa4, 0x3f, 0xa5, 0xa2,
    0x48, 0x45, 0xde, 0x7b, 0x73, 0x4b, 0x1f, 0x55, 0xeb, 0xb5, 0xc5, 0x37, 0x28, 0xbf, 0x03, 0x16,
    0xcf, 0x79, 0x07, 0x69, 0xda, 0x05, 0x34, 0xa6, 0x2e, 0xf3, 0x8a, 0xf6, 0x83, 0x60, 0x71, 0x6e,
    0x21, 0xdd, 0x3e, 0xe6, 0x54, 0xc4, 0x06, 0x50, 0x98, 0xbd, 0x40, 0xd9, 0xe8, 0x89, 0x19, 0xc8,
    0x80, 0x2b, 0x11, 0x5a, 0x0e, 0x85, 0xae, 0x2d, 0x0f, 0x5c, 0x5b, 0x36, 0x0a, 0x57, 0xee, 0x9b,
    0xc0, 0xdc, 0x77, 0x12, 0x93, 0xa0, 0x22, 0x1b, 0x09, 0x8b, 0xb6, 0x1e, 0xf1, 0x75, 0x99, 0x7f,
    0x43, 0x23, 0xed, 0xe4, 0x01, 0x72, 0x66, 0xfb, 0x31, 0x63, 0x97, 0xc6, 0x4a, 0xbb, 0xf9, 0x29,
    0x9e, 0xb2, 0x86, 0xc1, 0xb3, 0x70, 0x94, 0xe9, 0xfc, 0xf0, 0x7d, 0x33, 0x49, 0x38, 0xca, 0xd4, 
    0x3a, 0x78, 0x5f, 0x7e, 0xf5, 0x7a, 0xb7, 0xad, 0x8d, 0xd8, 0x39, 0xc3, 0x5d, 0xd0, 0xd5, 0x25,
    0xac, 0x18, 0x9c, 0x3b, 0x26, 0x59, 0x9a, 0x4f, 0x95, 0xff, 0xbc, 0x15, 0xe7, 0x6f, 0x9f, 0xb0, 
    0x4e, 0x82, 0x90, 0xa7, 0x04, 0xec, 0xcd, 0x91, 0x4d, 0xef, 0xaa, 0x96, 0xd1, 0x6a, 0x2c, 0x65,
    0x5e, 0x8c, 0x87, 0x0b, 0xcb, 0x32, 0x6c, 0xb8, 0x67, 0xdb, 0x10, 0xd6, 0xd7, 0xa1, 0xf8, 0x13,
    0xa9, 0x61, 0x1c, 0x47, 0xd2, 0xf2, 0x14, 0xc7, 0xf7, 0xfd, 0x3d, 0x44, 0xaf, 0x68, 0x24, 0xa3,
    0x1d, 0xe2, 0x3c, 0x0d, 0xa8, 0x0c, 0xb4, 0x56,
	};		
  volatile unsigned char sernum[5];	
	volatile unsigned char preSernum[5] = {0,0,0,0,0};
  volatile unsigned char buffRC522[100];
	volatile unsigned long int cardId;
	volatile unsigned int uc;
	_Bool justOneRead = 1;
	unsigned int readerTick = 0;
	unsigned char Byte = 1;
	#if defined (PN532_Routine )
  int32_t uid_len = -1;
	PN532 pn532;
	HAL_StatusTypeDef i2cStatus = HAL_OK;
	unsigned char versionPn532[4];
	#endif
	
	#if defined LCD
		unsigned int 	FgColor = 0x000000;	//Text fg color, 5-6-5 RGB
		unsigned int 	BkColor = 0xFCFCFC;	//Text bk color, 5-6-5 RGB
		unsigned int _color;
	  unsigned char oldMin = 0;
	  unsigned char refreshPage = 0;
	#endif
	#ifdef pos
	unsigned char posType = 0;
	unsigned int amount = 0;
  #endif
	
	#if defined GATEROADBLOCK
	unsigned int BridgeOpenTime = 0,Time = 0,tickOpenClose = 0;
	unsigned char DeviceType = 0;	
  #define BRIDGE_OPEN 1
	#define BRIDGE_CLOSE 2
	#define BRIDGE_OPENING 3
	#define BRIDGE_CLOSING 4
  #define BRIDGE_OPENED  5
	#define BRIDGE_CLOSED  6
	#define Idle 7
	#define RELAY_REST_TIME 100
  unsigned char CurBridgeState = BRIDGE_CLOSE;
	unsigned int TimeKeeper=0;
	unsigned int timeDelay = 0;	
	unsigned char onceSend = 0, enterExit = 0;
	#endif
	unsigned int tick = 0,tickTime=0;
  unsigned int timePassive = 0,timeAuth=0,timeRead=0,time1=0,time2=0,time3=0,time4=0,time5=0,timeWrite=0;
	#define MAX_TRANSACTIONS      ( addPersonels - addTransactions ) / LEN_TRANSACTIONS
#endif