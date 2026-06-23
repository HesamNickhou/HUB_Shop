
#include "main.h"

#if defined(cardAccDis)
#define ACK 0x06
#define NAK 0x15
#define ENQ 0x05
#define STX 0x02
#define ETX 0x03

extern	unsigned char uart3buf[300];
extern  unsigned int uart3Len;

void clearUart3Buf(void)
{
	memset(uart3buf,0,30);
	uart3Len = 0;
}
extern UART_HandleTypeDef huart3;
//void Acceptor_SendSerialData(unsigned char *buff) 
//{
//	HAL_UART_Transmit(&huart3,(unsigned char*)buff ,1,1);
//	return;
//}

unsigned char waitForAckNak(void)
{
unsigned int Li;
unsigned char Byte = 0x05;	

	HAL_Delay(5);
	for(unsigned char idx=0;idx<30;idx++)
		if((uart3buf[idx]==ACK)||(uart3buf[idx]==NAK))
		{
			Li = uart3buf[idx];
			clearUart3Buf();
			HAL_UART_Transmit(&huart3,&Byte,1,10); //Send ENQ
			return(Li);
		}
		
}

unsigned int waitForResponse(unsigned char cmd, unsigned char pm)
{
unsigned int Time=100;
unsigned int Li;
unsigned char Byte;	
		
	if(uart3Len==0)
		return(0);
	
	if(uart3Len<8)
		return(0);
	
	if(uart3buf[0]!=STX)
		return(0);
	
	if(uart3buf[1]!=0x00)
		return(0);

//	if(uart3buf[2]!=0x05)
//	return(0);
	
	if(uart3buf[3]!=cmd)
	return(0);
	
	if(uart3buf[4]!=pm)
	return(0);	
	
	Byte=0;
	
	if(cmd == 0x31)
	{
		for(Li=0; Li<9; Li++)
			Byte^=uart3buf[Li];
		if(Byte!=uart3buf[9])
			return(0);
	}
	
	if(cmd == 0x32)
	{
		for(Li=0; Li<7; Li++)
			Byte^=uart3buf[Li];
		if(Byte!=uart3buf[7])
			return(0);
	}
	
	Li = uart3buf[5]; 
	clearUart3Buf();
	return(Li);
}

unsigned char Get_CardAcceptorStatus(void)
{
	unsigned char pk[] = {0x02,0x00,0x02,0x31,0x30,0x03,0x02};
	unsigned char Byte=0;
	unsigned char response=0;
	HAL_UART_Transmit(&huart3,(unsigned char*)pk ,7,100);
	HAL_Delay(10);	
	Byte=waitForAckNak();
	if(Byte!=ACK)
		return(121);
	HAL_Delay(20);
	return (waitForResponse(0x31,0x30));	
}

unsigned char CardAcceptorMoveToRear(void)
{
	unsigned char pk[] = {0x02,0x00,0x02,0x32,0x33,0x03,0x02};
	unsigned char Byte=0;
	unsigned char response=0;
	HAL_UART_Transmit(&huart3,(unsigned char*)pk ,7,100);
	HAL_Delay(10);	
	Byte=waitForAckNak();
	if(Byte!=ACK)
		return(121);
	HAL_Delay(500);
	return (waitForResponse(0x32,0x33));	
}

unsigned char CardAcceptorMoveToFront(void)
{
	unsigned char pk[] = {0x02,0x00,0x02,0x32,0x30,0x03,0x01};
	unsigned char Byte=0;
	unsigned char response=0;
	HAL_UART_Transmit(&huart3,(unsigned char*)pk ,7,100);
	HAL_Delay(10);	
	Byte=waitForAckNak();
	if(Byte!=ACK)
		return(121);
	HAL_Delay(500);
	return (waitForResponse(0x32,0x30));	
}
#endif