#include "main.h"
#if defined(UHF)
#include "UHF.h"

void clearUart3Buf(void)
{
	memset(uart3buf,0,40);
	uart3Len = 0;
}

void clearUart2Buf(void)
{
	memset(GetResponse,0,40);
	bufferLen = 0;
}
//------------------------------------------------
unsigned char WriteBufferUHf(unsigned char *Buffer,unsigned char mode,unsigned char selectUHF)
{
	unsigned char paket[24];
	unsigned short BCC=0;
	unsigned char buf[16];
	unsigned short BCCC=0;
	paket[0] = 0x0a;
	paket[1] = 0xff;
	paket[2] = 0x15;
	paket[3] = 0x85;
	paket[4] = 0x03;
	paket[5] = 0x00;
	paket[6] = 0x08;
	
	if(mode == 0)
	{
		for(unsigned char i=7; i<=22 ;i++)
			paket[i] = Buffer[i-7];
	}
	
	if(mode == 1)
	{
		buf[0] = Buffer[4];
		buf[1] = Buffer[5];
		buf[2] = Buffer[6];
		buf[3] = Buffer[7];
		buf[4] = Buffer[8];
		buf[5] = Buffer[9];
		buf[6] = Buffer[10];
		buf[7] = Buffer[14];
		
		BCCC   = buf[7] + buf[6] + buf[5] + buf[4] + buf[3] + buf[2] + buf[1] + buf[0];
		buf[8] = BCCC & 0xff;
		buf[9] = 0;
		if(buf[0] >= 22 )
			buf[9] = buf[0] - 22;
		buf[9] <<= 4;
		buf[9] |= buf[1];					
		buf[10] = buf[2];
		buf[11] = buf[3];
		buf[12] = buf[4];
		buf[13] = buf[5];					
		buf[14] = buf[6];
		buf[14] <<= 4;
		buf[14] |= buf[7];
		buf[15] = BCCC&0xff;		
		
		for(unsigned char i=7; i<=22 ;i++)
			paket[i] = buf[i-7];
	}
	for(unsigned char i=0;i<=23;i++)
		BCC +=paket[i];
	BCC ^= 0xff;
	BCC++;
	BCC &= 0xff;
	paket[23] = BCC;	
	//-------------------
	if(selectUHF ==1)
	{
		clearUart3Buf();
		for(unsigned char i=0;i<TryToWrite;i++)
		{		
			HAL_UART_Transmit(&huart3,paket,24,500);
			HAL_Delay(delayForWrite);
			WDTR	
			if(waitForResponse(3,1))
			{
				waitToReadUHF1 = 0;
				tickForUHF1 = 0;
				if(mode == 0)
					return 0;
				if(mode == 1)
				{
					send("{\"C\":{\"Status\":0,\"Type\":\"ChangeGroup\"}}\n");
					return 0;
				}
			}
		clearUart3Buf();
		}
	}
	
	//-------------
	if(selectUHF == 2)
	{
		clearUart2Buf();
		for(unsigned char i=0;i<TryToWrite;i++)
		{		
			HAL_UART_Transmit(&huart2,paket,25,500);
			HAL_Delay(delayForWrite);
			WDTR	
			if(waitForResponse(3,2))
			{
				waitToReadUHF2 = 0;
				tickForUHF2 = 0;
				if(mode == 0)
					return 0;
				if(mode == 1)
				{
					send("{\"C\":{\"Status\":0,\"Type\":\"ChangeGroup\"}}\n");
					return 0;
				}
			}
		clearUart2Buf();
		}
	}
	return 1;
}
//----------------------------------------------------------------------------------
unsigned char rwParkingUHF(unsigned char rw,unsigned gp,unsigned char *SourceBuffer,unsigned char selectUHF)
{
//  unsigned int tick;  
	#define PARKING_EXIT_MODE  1
	#define PARKING_ENTER_MODE 0
	unsigned char GroupOfCard=0;
	unsigned char index=0;	
	unsigned char buff[200],bufer[16],buf[9];
	unsigned short Readed_BCC=0;
	unsigned short Readed_BCCC=0;
	unsigned char byte=0;
	unsigned char validData=0;
	
	for(unsigned char i =0;i<16;i++)
		bufer[i] = SourceBuffer[i];
	
	switch (rw)
	{
		case 0:
			while(1)
			{
				WDTR
				Readed_BCC = 0;
				
				for(index=0; index<8; index++){Readed_BCC+=bufer[index];}
				Readed_BCC &= 0xff;
				if(Readed_BCC == bufer[8])
					validData = 1;
				
				if(validData == 0)
				{
					buf[0] = (bufer[9] >> 4) + 22 ; // year 
					buf[1] =  bufer[9] & 0x0f; // month 
					buf[2] =  bufer[10]; // day 
					buf[3] =  bufer[11]; // hour 
					buf[4] =  bufer[12]; // min
					buf[5] =  bufer[13]; // sec
					buf[6] =  (bufer[14] >> 4); // EEM
					buf[7] =  bufer[14] & 0x0f; // gp 
					buf[8] =  (bufer[15]) ; // CRC
					Readed_BCCC =0;
					for(index=0; index<8; index++){Readed_BCCC+=buf[index];}
						Readed_BCCC &= 0xff;	
					if(Readed_BCCC == buf[8])
					{
						validData = 2;
						for(unsigned char cnt =0;cnt<9;cnt++)
							bufer[cnt] = buf[cnt];
					}						
				}
								
				if(validData)
				{
					GroupOfCard  = bufer[7]; 
					if(bufer[7]>10)
					{
						send("{\"C\":{\"Status\":8}}\n");
						return 255; 
					}
					if(bufer[3]>24 || bufer[4]>59 || bufer[5]>59 ||  bufer[2] >31 ||  bufer[1] > 12 || bufer[0] > 32)
					{
						send("{\"C\":{\"Status\":6}}\n");
						return 255; 
					}
										
					if(!((bufer[6] == PARKING_EXIT_MODE) ||  (bufer[6] == PARKING_ENTER_MODE)))
					{
						bufer[6] = PARKING_ENTER_MODE; 						
					}
				
					if(bufer[6] == PARKING_EXIT_MODE) 
					{
						parkTime.Entering_Exiting_Mode = PARKING_EXIT_MODE;
						bufer[6] = PARKING_ENTER_MODE; 	
					}	
					else
					{
						parkTime.Entering_Exiting_Mode = PARKING_ENTER_MODE;
						bufer[6] = PARKING_EXIT_MODE; 
					}				
					Readed_BCC=0;						
					parkTime.year  = bufer[0];
					parkTime.month = bufer[1];
					parkTime.day   = bufer[2];
					parkTime.hour  = bufer[3];
					parkTime.min   = bufer[4];
					parkTime.sec   = bufer[5];				
					HAL_RTC_GetTime(&hrtc,&time,RTC_FORMAT_BIN);
					HAL_RTC_GetDate(&hrtc,&date,RTC_FORMAT_BIN);	
					bufer[0]=date.Year;
					bufer[1]=date.Month;
					bufer[2]=date.Date;
					bufer[3]=time.Hours;
					bufer[4]=time.Minutes;		
					bufer[5] = time.Seconds;                  
					bufer[7]= GroupOfCard;   
					
					Readed_BCC=0;
					
					for(index=0; index<8; index++){Readed_BCC+=bufer[index];}
					bufer[8] = Readed_BCC&0xff;	
					
					bufer[9] = bufer[0] - 22;
					bufer[9] <<= 4;
					bufer[9] |= bufer[1];					
					bufer[10] = bufer[2];
					bufer[11] = bufer[3];
					bufer[12] = bufer[4];
					bufer[13] = bufer[5];					
					bufer[14] = bufer[6];
					bufer[14] <<= 4;
					bufer[14] |= bufer[7];
					bufer[15] = Readed_BCC&0xff;
					if(!WriteBufferUHf(bufer,0,selectUHF))
					{
						sprintf(buff,"{\"C\":{\"ID\":%d,\"UC\":%d,\"Gp\":%d,\"Y\":%d,\"M\":%d,\"D\":%d,\"H\":%d,\"m\":%d,\"S\":%d,\"EEM\":%d}}\n",cardId,uc,GroupOfCard,parkTime.year,parkTime.month,parkTime.day,parkTime.hour,parkTime.min,parkTime.sec,parkTime.Entering_Exiting_Mode); 
						send(buff);  //ersal ID card	
						HAL_Delay(2000);
						return 0;
					}
					else {send("{\"C\":{\"Status\":2}}\n"); HAL_Delay(500); return 255;}								
				}
				else {send("{\"C\":{\"Status\":4}}\n"); return 255;}								
			}
	}
}

//-----------------------------------------------

//------------------------------------------------
unsigned char waitForResponse(unsigned char pk,unsigned char selectUHF)
{
unsigned char Source_Buffer[16]={0};
unsigned short BCC =0;	
//unsigned char pkk [] = {0x00,0x00,0x00,0x00,0x22,0x08,0x11,0x0f,0x39,0x35,0x01,0x00,0x00,0x00,0x02,0x3b};		
	if(selectUHF == 1)
	{
		if(uart3Len<4)
			return(0);
	
		if(uart3buf[0]!=0x0b)
			return(0);
	
		BCC=0;
		for(unsigned char cnt = 0; cnt < uart3Len-1 ; cnt++)
			BCC+=uart3buf[cnt];
		BCC = BCC ^ 0xff;
		BCC++;
		BCC &=0xff;
		if(BCC != uart3buf[uart3Len-1])
			return 0;
	
		if(pk == 1) // select tag 
		{
			if(uart3buf[5] !=0)
				return uart3buf[5];
			else return 0;
		}
	
		if(pk == 3) // write tag 
		{
			if(uart3buf[4])
				return uart3buf[4];
			else return 0;
		}	
	
		if(pk == 2) // read tag
		{
			memset(Source_Buffer,0,15);
			for(unsigned char i=0,cnt = uart3Len - 17 ; cnt < uart3Len -1 ; cnt++,i++)
				Source_Buffer[i] = uart3buf[cnt];
			uc=uart3buf[7];
			uc+=(uart3buf[8]<<8);
			cardId=uart3buf[9];
			cardId+=(uart3buf[10]<<8);
			rwParkingUHF(0,0,Source_Buffer,1);
			return 0;
		}
	}
	//-------------
	if(selectUHF == 2)
	{
		if(bufferLen<5)
			return(0);
	
		if(GetResponse[0]!=0x0b)
			return(0);
	
		BCC=0;
		for(unsigned char cnt = 0; cnt < bufferLen-1 ; cnt++)
			BCC+=GetResponse[cnt];
		BCC = BCC ^ 0xff;
		BCC++;
		BCC &=0xff;
		if(BCC != GetResponse[bufferLen-1])
			return 0;
	
		if(pk == 1) // select tag 
		{
			if(GetResponse[5] !=0)
				return GetResponse[5];
		}
	
		if(pk == 3) // write tag 
		{
			if(GetResponse[4] !=0)
				return GetResponse[4];
			
		}	
	
		if(pk == 2) // read tag
		{
			for(unsigned char i=0,cnt = uart3Len -17; cnt < bufferLen -1 ; cnt++,i++)
				Source_Buffer[i] = GetResponse[cnt];
			
			uc=GetResponse[7];
			uc+=(GetResponse[8]<<8);
			cardId=GetResponse[9];
			cardId+=(GetResponse[10]<<8);
			rwParkingUHF(0,0,Source_Buffer,2);
			return 0;
		}
	}
}

unsigned char readCardUHF(unsigned char selectUHF)
{
	unsigned char pk[]  = {0x0A ,0xFF ,0x0B ,0x84 ,0x0A ,0x00 ,0x00 ,0x00 ,0x02 ,0x00 ,0x00 ,0x00 ,0x08 ,0x54};	
	unsigned char pk1[] = {0x0A ,0xFF ,0x03 ,0x41 ,0x01 ,0xb2}; 
	//---------------uhf 1
	if(selectUHF == 1)
	{
		clearUart3Buf();
		HAL_UART_Transmit(&huart3,pk,14,10);
		HAL_Delay(delayForSelect);
		if(waitForResponse(1,1))
		{
			clearUart3Buf();
			HAL_UART_Transmit(&huart3,pk1,6,10);
			HAL_Delay(delayForRead);
			waitForResponse(2,1);
		}
	}
	//---------------uhf 2
	if(selectUHF == 2)
	{
		clearUart2Buf();
		HAL_UART_Transmit(&huart2,pk,14,10);
		HAL_Delay(delayForSelect);
		if(waitForResponse(1,2))
		{
			clearUart2Buf();
			HAL_UART_Transmit(&huart2,pk1,6,10);
			HAL_Delay(delayForRead);
			waitForResponse(2,2);
		}		
	}
	return 0;
}

#endif