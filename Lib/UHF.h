#ifndef UHF_H
#define UHF_H
#include "main.h"
#include "string.h"
#define TryToWrite     50
#define delayForWrite  110 
#define delayForRead   60 
#define delayForSelect 60 
extern unsigned char uart3buf[100];
extern unsigned char GetResponse[600];
extern unsigned char uart3Len;
extern unsigned int bufferLen;
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart2;
extern volatile unsigned long int cardId;
extern volatile unsigned int uc;
extern IWDG_HandleTypeDef hiwdg;
extern RTC_HandleTypeDef hrtc;
extern RTC_TimeTypeDef time;
extern RTC_DateTypeDef date;
extern unsigned char waitToReadUHF1,waitToReadUHF2;
extern unsigned int tickForUHF1,tickForUHF2;
extern void send(unsigned char *buf);
struct {
		unsigned char year;           
		unsigned char month;        
		unsigned char day;      
		unsigned char hour;      
		unsigned char min;
		unsigned char sec;
    unsigned char Entering_Exiting_Mode;		
		}parkTime;
#define WDTR __HAL_IWDG_RELOAD_COUNTER(&hiwdg);
void clearUart3Buf(void);
void clearUart2Buf(void);
unsigned char readCardUHF(unsigned char selectUHF);
unsigned char waitForResponse(unsigned char pk,unsigned char selectUHF);
unsigned char rwParkingUHF(unsigned char rw,unsigned gp,unsigned char *SourceBuffer,unsigned char selectUHF);
unsigned char WriteBufferUHf(unsigned char *Buffer,unsigned char mode,unsigned char selectUHF);
#endif