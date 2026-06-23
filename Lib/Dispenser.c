
#define ACK 0x06
#define NAK 0x15
#define ENQ 0x05
#define STX 0x02
#define ETX 0x03

#include "main.h"

#if defined(cardAccDis)
extern	unsigned char uart3buf[300];
extern  unsigned int uart3Len;

extern void clearUart3Buf(void);


extern UART_HandleTypeDef huart3;
void cardDis_SendSerialData(unsigned char *buff) 
{
	 HAL_UART_Transmit(&huart3,(unsigned char*)buff ,1,1);
	return;
}

unsigned char WaitForAckNak(void)
{
unsigned int Li;
unsigned char Byte = 0x05;	

//  Li=HAL_GetTick();
//  while(1)	
//	{
//		if(HAL_GetTick()-Li>500)
//			return(0);
		HAL_Delay(20);
		for(unsigned char idx=0;idx<30;idx++)
			if((uart3buf[idx]==ACK)||(uart3buf[idx]==NAK))
			{
				Li = uart3buf[idx];
			  clearUart3Buf();
				HAL_UART_Transmit(&huart3,&Byte,1,10); //Send ENQ
				return(Li);
			}
	//}
}
//=========================================================================

unsigned int WaitForResponse(void)
{
unsigned int Time=100;
unsigned int Li;
unsigned char Byte;	
	
//  
//	uart3Len=0;
//	Li=HAL_GetTick();
//  while(1)	
//	{
//		if(HAL_GetTick()-Li>Time)
//			break;
//		
//		if(GSM_GCF(&Byte))
//		{
//			Li=HAL_GetTick();
//			if((uart3Len==0)&&(Byte!=0x02)) continue;
//			Time=300;
//			uart3buf[DispenserBufferLen++]=Byte;	
//		}
//	}
	HAL_Delay(50);
	
	if(uart3Len==0)
		return(0);
	
	if(uart3Len<8)
		return(0);
	
	if(uart3buf[1]!='S')
		return(0);
	if(uart3buf[2]!='F')
		return(0);
	if(uart3buf[6]!=0x03)
		return(0);
	Byte=0;
	for(Li=0; Li<7; Li++)
	  Byte^=uart3buf[Li];
	if(Byte!=uart3buf[7])
		return(0);
	Li=uart3buf[3]; Li<<=8;
	Li+=uart3buf[4]; Li<<=8;
	Li+=uart3buf[5]; 
	return(Li);
}

//============================================================================== 
char Get_CardDispenserStatus(void)
{
unsigned char Byte;
unsigned int Response;	
unsigned char sendpk[] = {0x02,0x52,0x46,0x03,0x15};

		HAL_UART_Transmit(&huart3,(unsigned char*)sendpk ,5,100);
//		SendData(0x02);
//		SendData(0x52);
//		SendData(0x46);
//		SendData(0x03);
//		SendData(0x15);
		Byte=WaitForAckNak();
		if(Byte!=ACK)
			return(121);

		Response=WaitForResponse();
		
		switch(Response)
		{
			case 0x303030: return 100; // ready - ok
			case 0x303130: return 110; // low card
			case 0x313030: return 111; // Recycling bin is full
			case 0x303038: return 101; // empty
			case 0x303034: return 102; // card in taking away place
			case 0x303032: return 103; // card in reader area
			case 0x303031: return 104; // not ready for pre-read before issuing
			case 0x303230: return 106; // blocking
			case 0x383030: return 107; // the machine in issuing card
			case 0x323030: return 108; // error in issuing
			case 0x343030: return 109; // the machine in returning back the card
			case 0x303430: return 110; // -
			case 0x303830: return 111; // -
			default: return 120; //Error
		}
		
}
//==================================================================
unsigned char Dispenser_To_Reader(void)
{
unsigned char Byte;	
unsigned char sendpk[] = {0x02,0x44,0x48,0x03,0x0d};	
		
	  clearUart3Buf();
//	  EmptyGSMRXBuffer();
	  HAL_UART_Transmit(&huart3,(unsigned char*)sendpk ,5,100);
	  Byte=WaitForAckNak();
		
		switch(Byte)
		{
			case 0x06: // ok (ACK)
				return 100;
			case 0x15: // err (NAK)
				return 105;
		}
		
		return(105);
}
//=========================================================
unsigned char Dispenser_To_Recyclebin(void)
{
unsigned char Byte;
unsigned int Li=0;
unsigned char sendpk[] = {0x02,0x43,0x50,0x03,0x12};		  
//	if(ReaderSelect==1)
//		return(0);
	
//	EmptyGSMRXBuffer();

	clearUart3Buf();
	Li=HAL_GetTick();
//	while(1)
//	{
//		if(HAL_GetTick()-Li>2000)
//			break;
//		Byte=Get_CardDispenserStatus();
//		if(Byte==103)
//			break;
//		if(Byte==102)
//			break;
//	  Dispenser_To_Reader();
//	}
	
//	HAL_Delay(10);	    
//	USART_SendData(USART2,0x02);	    
//	USART_SendData(USART2,0x43);	    
//	USART_SendData(USART2,0x50);	    
//	USART_SendData(USART2,0x03);    
//	USART_SendData(USART2,0x12);		
	HAL_UART_Transmit(&huart3,(unsigned char*)sendpk ,5,100);	
  Byte=WaitForAckNak();

	if(Byte==ACK)
		return 100;
	else
		return 105;
		
}
//==========================================================
unsigned char Dispenser_To_Customer(void)
{
unsigned int Li=0, Time;
unsigned char cnt=0, Byte;
unsigned char sendpk[] = {0x02,0x45,0x53,0x03,0x17};
//	if(ReaderSelect==1)
//		return(0);
	HAL_UART_Transmit(&huart3,(unsigned char*)sendpk ,5,100);
	Byte=Get_CardDispenserStatus();
	for(cnt=0; cnt<2; cnt++)
	{
    uart3Len = 0;
    HAL_Delay(10);	    
//    USART_SendData(USART2,0x02);
//    USART_SendData(USART2,0x45);
//    USART_SendData(USART2,0x53);
//    USART_SendData(USART2,0x03);
//    USART_SendData(USART2,0x17);
		HAL_UART_Transmit(&huart3,(unsigned char*)sendpk ,5,100);	
	
    Byte=WaitForAckNak();
		switch(Byte)
		{
			case 0x06: // ok (ACK)
				return 100;
			case 0x15: // err (NAK)
				return 105;
		}
		
		if(Byte==ACK)
			break;
	}
//	SendTextDOTMatrix("لطفا کارت را برداريد.",0);
//	send("get card");
//	PlayVoice(0,"0:GetCard.wav");
	
//	if(AutoDispenseCard==1)
//		return(0);
	
//  Li=HAL_GetTick();
//	Time=HAL_GetTick();
//	cnt=10; //wait not need
//  while(1)	
//	{
//		if(HAL_GetTick()-Li>20000)
//		{
//			Dispenser_To_Recyclebin();
//			return(1);
//		}
//		Byte=Get_CardDispenserStatus();
//		//printf("S:%d ", Byte);
//  	switch(Byte)
//	  {
//		  case 102:
//				if(HAL_GetTick()-Time>=5000)
//				{
//					Time=HAL_GetTick();
//        	//PlayVoice(0,"0:GetCard.wav");
//				}
//				cnt=10; //no wait need
//				break;
//			case 103: //Card in reader area
////        USART_SendData(USART2,0x02);
////        USART_SendData(USART2,0x45);
////        USART_SendData(USART2,0x53);
////        USART_SendData(USART2,0x03);
////        USART_SendData(USART2,0x17);
//			  HAL_UART_Transmit(&huart3,(unsigned char*)sendpk ,5,100);	
//        WaitForAckNak();
//			  cnt=0; //wait for true status
//        break;				
//		  case 100:
//				return(0);
//		  case 110: //low card
//		  case 107: //empty
//				if(++cnt>5) //after dispense_to_customer command device return 102 status after few seconds
//				  return(0);
//				break;
//	  }
//	}
	
}
#endif
//=======================================================