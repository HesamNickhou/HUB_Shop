//==========================================================================
//                         FINGERPRINT SENSOR
//
//==========================================================================

#include "main.h"
#include "stdbool.h"
#include "finger.h"
#include "softuart.h"
#include "string.h"
#include "stm32f1xx_hal.h"
#include "usbd_cdc_if.h"

//==========================================================================
//                        PerDefined Conditions
//==========================================================================
#ifdef finger
extern UART_HandleTypeDef huart2;
extern unsigned char GetResponse[600];//,GetResponseForTp[600];
extern _Bool IdentifyFlag;
extern int bufferLen;
extern IWDG_HandleTypeDef hiwdg;
extern _Bool rxITForFinger;
extern _Bool fingerITflag;
unsigned int lastTick = 0;
unsigned char step = 0;
#define size 600
extern unsigned char fromAndroid[size];
#ifdef MaxReciveBuf
extern uint16_t rxBufferHeadPos;
#endif
#define WDTR  __HAL_IWDG_RELOAD_COUNTER(&hiwdg);
//========================================================================
void FP_SendSerialData(unsigned char *buff) {
	 HAL_UART_Transmit(&huart2,(unsigned char*)buff ,1,1);
	return;
}
//========================================================================FPRN_Send_Command(0x0103,2,ResBuff);
void FPRN_Send_Command(unsigned short Command, unsigned short Len, 
	unsigned char *Data) {
	
	unsigned char BUF[50]={0};	
	unsigned char STR[200];
	unsigned char i;
	unsigned short FP_CheckSum=0;
	
	BUF[0] = 0x55;
	FP_SendSerialData(&BUF[0]);// *******HAL_UART_Transmit(&huart2,(unsigned char*)buff ,1,1);
	FP_CheckSum += 0x55; 
	//******************************
	BUF[1] = 0xAA;
	FP_SendSerialData(&BUF[1]);	
	FP_CheckSum += 0xAA; 
//***********************************Command Code
	BUF[2] = Command%256;           //L
	FP_SendSerialData(&BUF[2]);	
	FP_CheckSum += Command%256;  
	
	BUF[3] = Command / 256;            //H
	FP_SendSerialData(&BUF[3]);	
	FP_CheckSum += (Command / 256);
	//********************************Length of Command Parameter
	BUF[4] = Len % 256;
	FP_SendSerialData(&BUF[4]);   
	FP_CheckSum += (Len % 256); 
	
	BUF[5] = Len / 256;	
	FP_SendSerialData(&BUF[5]);  	
	FP_CheckSum += (Len / 256); 
	//*******************************
	for (i=0; i<Len; i++) {
		BUF[6 + i] = Data[i];
		FP_SendSerialData(&Data[i]);
		FP_CheckSum += Data[i]; 
	}
					
	for (; i<16; i++) {
		BUF[6 + i] = 0;
		FP_SendSerialData(&BUF[6 + i]);
	}
	//******************************Check Sum:
	BUF[22] = FP_CheckSum % 256;   																							
	FP_SendSerialData(&BUF[22]);
	BUF[23] = FP_CheckSum / 256;
	FP_SendSerialData(&BUF[23]);
	
	//FP_SendSerialData(BUF); 
	i = 0;
}
//==================================================================================
void FPRN_Send_TemplateDataPacket(unsigned short Command, unsigned int Len, 
	unsigned int ID, unsigned char *Data) {

	unsigned char BUF[50];
	unsigned char STR[200];
	unsigned int i;
	unsigned int FP_CheckSum = 0;

	BUF[0] = 0x5A;
	FP_SendSerialData(&BUF[0]);
	FP_CheckSum += 0x5A;    

	BUF[1] = 0xA5;
	FP_SendSerialData(&BUF[1]);
	FP_CheckSum += 0xA5; 

	BUF[2] = Command % 256;
	FP_SendSerialData(&BUF[2]);
	FP_CheckSum += Command % 256; 
		
	BUF[3] = Command / 256;
	FP_SendSerialData(&BUF[3]);
	FP_CheckSum += (Command / 256);
	BUF[4] = ((Len + 2) % 256);
	FP_SendSerialData(&BUF[4]);   
	FP_CheckSum += ((Len + 2) % 256); 
	
	BUF[5] = ((Len + 2) / 256);
	FP_SendSerialData(&BUF[5]);  
	FP_CheckSum += ((Len + 2) / 256);
	
	BUF[6] = (ID % 256); 
	FP_SendSerialData(&BUF[6]);   
	FP_CheckSum += (ID % 256); 
	
	BUF[7] = (ID / 256); 
	FP_SendSerialData(&BUF[7]);  
	FP_CheckSum += (ID / 256);
	
	for(i=0; i<Len; i++) {
		FP_SendSerialData(&Data[i]); 
		FP_CheckSum += Data[i]; 
	}

	BUF[47] = FP_CheckSum % 256; 
	FP_SendSerialData(&BUF[47]);
	
	BUF[48] = FP_CheckSum / 256; 
	FP_SendSerialData(&BUF[48]);
	i = 0;
}
//*******************************************************
char getResponse(unsigned short cmd) {	
	unsigned int checkSum = 0;
	unsigned int li = 0;
	HAL_Delay(50);
	li = HAL_GetTick();
	while (1) {
		if (HAL_GetTick() - li > 7000)
			 return 1; // timeout 			
			 WDTR
		if (0 != GetResponse[23])
			break;
		
		if (GetResponse[0] == 0xA5) {
			if (cmd == 0x010B) {
				if (bufferLen >= 12)
					return 0;
			}
		}
	}
	bufferLen = 0;
	checkSum = 0;	
	for(unsigned char cnt = 0; cnt < 22 ; cnt++)
		checkSum+=GetResponse[cnt];

	if ((cmd == ((GetResponse[3] << 8) + GetResponse[2])) || (cmd == 0x0130)) {
			if (checkSum == ((GetResponse[23] << 8) + GetResponse[22])) {
				if (cmd == 0x0130) {
					if (0x41==GetResponse[8]+(GetResponse[9]*256))
						return 0;
					else 
						return 1;
				}
				return 0;
			}
			else
				return 1;
	}
	else
		return 1;	
}	
//************************************************************************
char FPRN_CancelCmd(void) {
	
//	                                	unsigned char ResBuff[50]={0};
//																		_Bool bufIdentifyFlag = IdentifyFlag;
//																		bufferLen = 0;
//																		IdentifyFlag = 0;
//																		FPRN_Send_Command(0x0130,0,ResBuff);
//																		HAL_Delay(100);
//																		//getResponse(0x0130);
//																		memset(GetResponse,'\0',sizeof(GetResponse));
//																		bufferLen = 0;
//																		IdentifyFlag = bufIdentifyFlag;
	
//	unsigned char ResBuff[50]={0};
//	_Bool bufIdentifyFlag = IdentifyFlag;
//	bufferLen = 0;
//	IdentifyFlag = 0;
////	FPRN_Send_Command(0x0130,0,ResBuff);
////	getResponse(0x0130);
  memset(GetResponse, '\0', sizeof(GetResponse));
	bufferLen = 0;
//	IdentifyFlag = bufIdentifyFlag;
}
//*************************************************************************
char FPRNCancelCmd(void) {
	unsigned char ResBuff[50] = {0};
	_Bool bufIdentifyFlag = IdentifyFlag;
	bufferLen 	 = 0;
	IdentifyFlag = 0;
	FPRN_Send_Command(0x0130, 0, ResBuff);
	HAL_Delay(100);
	//getResponse(0x0130);
  memset(GetResponse, '\0', sizeof(GetResponse));
	bufferLen = 0;
	IdentifyFlag = bufIdentifyFlag;
}
//=================================================================================
char FPRN_IdentifyFree(void) {
	unsigned char ResBuff[50] = {0};
// unsigned char Status; 
// FPRN_CancelCmd();
// HAL_Delay(100);
	FPRN_Send_Command(0x0125, 0, ResBuff);
// Status=GetResponse[8]+(GetResponse[9]*256);
// sprintf(ResBuff,"Status:%d\n",Status); 
// send(ResBuff);
	return 0;
}
//==================================================================================
unsigned char FPRN_DeleteAllTemplate(void) {
	unsigned char 
		ResBuff[100] = {0},
		RetValue = 0,
		BUF[50] = {
			0x55, 0xAA, 0x06, 0x01, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x06, 0x01
		},	//DeleteALL command
		
		LEDON[50] = {
			0x55 ,0xAA ,0x24 ,0x01 ,0x02 ,
			0x00 ,0x01 ,0x00 ,0x00 ,0x00 ,
			0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,
			0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,
			0x00 ,0x00 ,0x27 ,0x01
		}, // LED ON cammand
	
		LEDOFF[50] = {
			0x55 ,0xAA ,0x24 ,0x01 ,0x02 ,
			0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,
			0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,
			0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,
			0x00 ,0x00 ,0x26 ,0x01
		}; // LED OFF cammand	 
		
	FPRN_CancelCmd();
	//HAL_Delay(100);
	bufferLen = 0;
				
	HAL_UART_Transmit(&huart2, (unsigned char*)BUF, 24, 100); //DeleteALL command

	if (getResponse(0x0106) == 1) {
		send("{\"Receive\":{\"Status\":26},\"F\":{\"Type\":\"DeleteALL\",\"Status\":-1}}\n"); // try again
		memset(GetResponse,'\0',sizeof(GetResponse));
		return 1;
	}

	sprintf(ResBuff,"{\"Receive\":{\"Status\":26},\"F\":{\"Type\":\"DeleteALL\",\"Status\":%d}}\n",GetResponse[6]);	
	send(ResBuff);

	HAL_UART_Transmit(&huart2, (unsigned char*)LEDON, 24, 100); //LED ON
	HAL_Delay(500);
	HAL_UART_Transmit(&huart2, (unsigned char*)LEDOFF, 24, 100); //LED OFF
	HAL_Delay(1);
	memset(GetResponse, '\0', sizeof(GetResponse));
	IdentifyFlag = 0;
	return 0;
}

//===================================================================================
unsigned int FPRN_Identify(void) {	
  unsigned char ResBuff[100] = {0}; 
  unsigned int Status;
  bufferLen = 0;
	if ((HAL_GetTick() - lastTick) >= 150) {
		step = 1 - step;
		
		if (step == 0)
			FPRN_Send_Command(0x0102, 0, ResBuff);
		
		else if (step == 1) {
			HAL_Delay(150);
			
			Status = GetResponse[8] + (GetResponse[9] * 256);
			if ((GetResponse[0] == 0xAA) && (GetResponse[1] == 0x55) && 
					(GetResponse[2] == 0x02) && (GetResponse[3] == 0x01) && 
					(fingerITflag == 1)) {
				fingerITflag = 0;
				if (!GetResponse[6]) {
					if (Status != 0xFFF4)
						if (Status != 0x0) {
							sprintf(ResBuff,"{\"F\":{\"Type\":\"Identify\",\"Template\":%d}}\n",
								Status);
							send(ResBuff);
						}
				}
				else {
					switch (Status) {
						case 0x15:
							sprintf(ResBuff, "{\"F\":{\"Type\":\"Identify\",\"Status\":%d}}\n", 
								Status);
							IdentifyFlag = 0;
							send(ResBuff);
							break;
						case 0x12:
							sprintf(ResBuff, "{\"F\":{\"Type\":\"Identify\",\"Status\":%d}}\n", 
								Status + 4);
							send(ResBuff);
							break;
					} //Switch
				} //else
			} // if (!getResponse[6])
			memset(GetResponse, '\0', sizeof(GetResponse));
		}
		
		lastTick = HAL_GetTick();
	}
	return 0;
}

//===================================================================================
char FPRN_ChangeBaudRate(unsigned int Baud) {
	unsigned char _ID[4];
	unsigned char ResBuff[50];	
  FPRN_CancelCmd();	
  ResBuff[0] = 1;
  ResBuff[1] = 3;
	ResBuff[2] = Baud;	
	FPRN_Send_Command(0x013A, 3, ResBuff);
  return 0;
}
//===================================================================================
char FPRN_DeleteTemplate(unsigned int ID) {
	bufferLen = 0;
	unsigned char ResBuff[100];
	unsigned int Status;
  FPRN_CancelCmd();
//	HAL_Delay(100);
  ResBuff[0] = ID % 256;
  ResBuff[1] = ID / 256;
	FPRN_Send_Command(0x0105, 2, ResBuff);
	if (getResponse(0x0105) == 1) {
		send("{\"Receive\":{\"Status\":26},\"F\":{\"Type\":\"Delete\",\"" \
			"Status\":-1}}\n"); // try again
		return 1;
	}

//	HAL_Delay(1000);
	if (GetResponse[0] == 0xAA) {
		if (!GetResponse[6])
			sprintf(ResBuff, "{\"Receive\":{\"Status\":26},\"F\":{\"Type\":\"" \
				"Delete\",\"Status\":%d}}\n", GetResponse[6]);//OK delete template    status=0
		else {
			Status = GetResponse[8] + (GetResponse[9] * 256);
			sprintf(ResBuff, "{\"Receive\":{\"Status\":26},\"F\":{\"Type\":\"" \
				"Delete\",\"Status\":%d}}\n", Status); //0x60:invalid no.status=96   
				//0x13:No fingerprint found in the required no.status=19
		}
		send(ResBuff);			
		FPRN_CancelCmd();
		//HAL_Delay(200);
		memset(GetResponse, '\0', sizeof(GetResponse));
		return 0;
	}
	//HAL_Delay(200);
  return 1;
}
//===================================================================================
void FPRN_GetTemplateByID(unsigned int FPID, unsigned char inHEX) {
	unsigned int 
		FP_CheckSum  = 0,
		FP_CheckSum1 = 0,
		idx 				 = 0, 
		cnt 				 = 0,
		K 					 = 0,
		i, 
		Len, 
		ID, 
		RetValue,
		li;
	
	unsigned char 
		ResBuff[100],
		Buffer[570],
		temp[64], 
		j = 0;
	
	rxITForFinger = 0;
	bufferLen 		= 0;
	
	FPRN_CancelCmd();	
	bufferLen = 0;
	memset(GetResponse, '\0', sizeof(GetResponse));

	ResBuff[0] = FPID % 256;
	ResBuff[1] = FPID / 256;
 //HAL_Delay(100);	
	FPRN_Send_Command(0x010A,2,ResBuff);//   send Read Template
	HAL_Delay(50);

	li = HAL_GetTick();	
	while (1) {
		if (HAL_GetTick() - li > 5000) {		
			send("{\"F\":{\"Type\":\"ReadTemplate\",\"Status\":-1}}\n");
			memset(GetResponse,'\0',sizeof(GetResponse));
			rxITForFinger =1;
			bufferLen = 0;
			return;
		}			
		WDTR
		if (bufferLen >= 606)
			break;		

		if ((GetResponse[0] == 0xAA) && (GetResponse[1] == 0x55) && 
				(GetResponse[2] == 0x0A) && (GetResponse[6] == 1) 	 && 
				(bufferLen == 24)) {//55 AA 0A 01 

				RetValue=GetResponse[8]+(GetResponse[9]*256);
				sprintf(ResBuff,"{\"F\":{\"Type\":\"ReadTemplate\",\"Status\":%d}}\n",RetValue);
				send(ResBuff);
				memset(GetResponse,'\0',sizeof(GetResponse));
				rxITForFinger =1;
				bufferLen = 0;			
				return;
		}
	}//*********


	for (i=0; i<600; i++)
		if ((GetResponse[i] == 0xA5) && (GetResponse[i + 1] == 0x5A) &&
			  (GetResponse[i+2]==0x0A) && (GetResponse[i + 3] == 0x01)) {	// A5 5A 0A 01 
			K = i;
			break;
		}

	Len = (GetResponse[K + 4] + GetResponse[K + 5] * 256 - 4);
	ID  =  GetResponse[K + 8] + GetResponse[K + 9] * 256;

	if ((Len == 0) || (ID == 0)) {
		HAL_Delay(10);
		send("{\"F\":{\"Type\":\"ReadTemplate\",\"Status\":-1}}\n"); // try again 
		memset(GetResponse, '\0', sizeof(GetResponse));
		rxITForFinger = 1;
		bufferLen 		= 0;
		return;
	}
	
	memset(Buffer, '\0', 570);
	HAL_Delay(10);
	for(i=0; i<(int)Len; i++)// fingerprint data:(570byte)+ Data packet(2byte)
		Buffer[i] = GetResponse[K + 10 + i];
	rxITForFinger = 1;



	//                                                       for(i=2;i<(int)Len-2;i++)// fingerprint data:(570byte)+ Data packet(2byte)
	//																													 {  
	//																													          	FP_CheckSum += Buffer[i];
	//																													 }
	//                                      
	//                                                           
	//                                                           FP_CheckSum1 +=Buffer[(int)Len-2];
	//																													 FP_CheckSum1 +=Buffer[(int)Len-1]*256;

	if (!GetResponse[6]) {												 												 												 
	//							if(FP_CheckSum ==	FP_CheckSum1 ){	

		sprintf(ResBuff, "{\"F\":{\"Type\":\"ReadTemplate\",\"ID\":%d,\"DataSize" \
			"\":%d,\"Data\":%s", FPID, Len, inHEX == 1 ? "[" : "\"");
		
		FP_CheckSum = 0;
		for (i=0; i<strlen(ResBuff); i++)
			FP_CheckSum ^= ResBuff[i];
		
		send(ResBuff);
		HAL_Delay(1000);
		WDTR
		
		for(j=1; j*63<Len; j++) {
			idx = (j - 1) * 63;
			for (cnt=0; idx<63*j; idx++ , cnt++) {
				temp[cnt] = Buffer[idx];
				if (inHEX == 1) {
					sprintf(ResBuff, "%02X,", temp[cnt]);
					
					FP_CheckSum ^= ResBuff[0];
					FP_CheckSum ^= ResBuff[1];
					FP_CheckSum ^= ResBuff[2];
					
					send(ResBuff);
				}
			}
			
			if (inHEX == 0)
				CDC_Transmit_FS(temp,63);
			
			HAL_Delay(100);
			WDTR
		}

	idx = (j - 1) * 63;
	for (cnt = 0; idx<Len; idx++ , cnt++) {
		temp[cnt] = Buffer[idx];
		if (inHEX == 1) {
			if (cnt > 0) {
				send(",");
				FP_CheckSum ^= ',';
			}
			sprintf(ResBuff, "%02X", temp[cnt]);
			
			FP_CheckSum ^= ResBuff[0];
			FP_CheckSum ^= ResBuff[1];
			FP_CheckSum ^= ResBuff[2];
			
			send(ResBuff);
		}
	}

	if (inHEX == 0)
		CDC_Transmit_FS(temp, cnt);
	HAL_Delay(5);
	send(inHEX == 1 ? "]}}\n" : "\"}}\n");

	FP_CheckSum ^= ']';
	FP_CheckSum ^= '}';
	FP_CheckSum ^= '\n';
	
	if (inHEX == 1) {
		uint8_t t = FP_CheckSum & 0x000000FF;
		CDC_Transmit_FS(&t, 1);
	}
	//																						 }
	//							
	//                                      else {
	//																																 send("{\"F\":{\"Type\":\"ReadTemplate\",\"Status\":100}}\n"); // try again 
	//												                                         memset(GetResponse,'\0',sizeof(GetResponse));
	//												                                         rxITForFinger =1;
	//												                                         bufferLen = 0;
	//												                                         return;
	//																		    	}

	}
	else {
		RetValue = GetResponse[8] + (GetResponse[9] * 256);
		sprintf(ResBuff, "{\"F\":{\"Type\":\"ReadTemplate\",\"Status\":%d}}\n",
			RetValue);
		send(ResBuff);
	}
	memset(GetResponse, '\0', sizeof(GetResponse)); 
}
//===================================================================================FPRN_WriteTemplateByID
unsigned int FPRN_WriteTemplateByID(unsigned int Ftamplate_ID, 
	unsigned int FTemplate_Len,unsigned char *_FTemplate) {//FPRN_WriteTemplateByID(100,570,fromAndroid); 

	unsigned int i,RetValue = 100;
	unsigned char 
		ResBuff[100] = {0},
		buffer[570]  = {0};
	rxITForFinger = 1;
	Ftamplate_ID  = (_FTemplate[1] * 0x100) + _FTemplate[2];//home finger
	FTemplate_Len = (_FTemplate[3] * 0x100) + _FTemplate[4];// FTemplate_Len=570

	for (int cnt=6; cnt<(FTemplate_Len) + 6; cnt++)// byte finger  FTemplate_Len=570
		buffer[cnt - 6] = _FTemplate[cnt];

	FPRN_CancelCmd();
	ResBuff[0] = (FTemplate_Len) % 256;
	ResBuff[1] = (FTemplate_Len) / 256;

	WDTR
	FPRN_Send_Command(0x010B, 2, ResBuff);// WRITE Data:
	getResponse(0x010B);//READ Data

	memset(GetResponse,'\0',sizeof(GetResponse));
	bufferLen = 0;
	FPRN_Send_TemplateDataPacket(0x010B, FTemplate_Len, Ftamplate_ID, buffer);//( Command,Len,ID,Data)READ Data
	WDTR
	getResponse(0x010B);
	if (GetResponse[0] == 0xA5) {//PREFIX
		if (!GetResponse[6]) {   //Result code (0 : OK  ; 1 : Fail)
			if (((_FTemplate[1] * 0x100) + _FTemplate[2]) == (GetResponse[8] + (GetResponse[9] * 256))) {
				sprintf(ResBuff,"{\"F\":{\"Type\":\"WriteTemplate\",\"WriteTemplateID\":%d,\"Status\":0}}\n",Ftamplate_ID);
				send(ResBuff);
				IdentifyFlag = 1;
			}
			else  
				send("{\"F\":{\"Type\":\"WriteTemplate\",\"Status\":100}}\n"); 
		}
		else {
			if (GetResponse[8] == 0x60) send("{\"F\":{\"Type\":\"WriteTemplate\",\"Status\":96}}\n");  //0x70:Invalid dat
			if (GetResponse[8] == 0x70) send("{\"F\":{\"Type\":\"WriteTemplate\",\"Status\":112}}\n"); //invalid Template No.
		}
	}
	else
		send("{\"F\":{\"Type\":\"WriteTemplate\",\"Status\":-1}}\n");
	bufferLen = 0;
	memset(GetResponse, '\0', sizeof(GetResponse));
	return 1;
}
//===================================================================================FPRN_GetEmptyID
unsigned int FPRN_GetEmptyID(void) {
	unsigned char ResBuff[100];
	unsigned int RetValue;
	bufferLen = 0;
	FPRN_CancelCmd();
	bufferLen = 0;
	HAL_Delay(10);
	FPRN_Send_Command(0x0107, 0, ResBuff);
 //HAL_Delay(350);
	if (getResponse(0x0107) == 1) {
		send("{\"Receive\":{\"Status\":26},\"F\":{\"Type\":\"GetEmptyID\",\"Status\":-1}}\n"); // try again
		return 1;
	}

	if ((GetResponse[0] == 0xaa) && (GetResponse[1] == 0x55) && 
			(GetResponse[2] == 0x07) && (GetResponse[3] == 0x01)) {
		RetValue = GetResponse[8] + (GetResponse[9] * 256);
		if (RetValue != 0) {
			if (!GetResponse[6])
				sprintf(ResBuff,"{\"Receive\":{\"Status\":26},\"F\":{\"Type\":\"GetEmptyID\",\"ID\":%d}}\n",RetValue); //sprintf(ResBuff,"Min EmptyID:%d\n",RetValue);
			else
				sprintf(ResBuff,"{\"Receive\":{\"Status\":26},\"F\":{\"Type\":\"GetEmptyID\",\"Status\":%d}}\n",RetValue); //sprintf(ResBuff,"EmptyID Status:%d\n",RetValue); 
			send(ResBuff);
			HAL_Delay(1);
		}
	}
	memset(GetResponse, '\0', sizeof(GetResponse));
	return 0;
}
//===================================================================================FPRN_AddNewUser
char FPRN_AddNewUser(unsigned int ID) {
	unsigned char 
		status = 0,
		ResBuff[100] = {0};
	unsigned int value = 0;
	char retu = 0;
	
	_Bool Exit_While = 0;
	_Bool buffIdentifyFlag;
	bufferLen = 0;	
	
	buffIdentifyFlag = IdentifyFlag;	
	
	for (unsigned char Try=0; Try<2; Try++) {
		bufferLen = 0;
		FPRNCancelCmd();
		HAL_Delay(100); 
		ResBuff[0] = ID % 256;
		ResBuff[1] = ID / 256;
		bufferLen  = 0;
		FPRN_Send_Command(0x0103, 2, ResBuff);
		if (getResponse(0x0103) == 1) {
			send("{\"Receive\":{\"Status\":26},\"F\":{\"Type\":\"Enroll\",\"Status\":-1}}\n"); // try again
			return 1;
		}
		//	HAL_Delay(200);
		if ((GetResponse[0] == 0xAA) && (GetResponse[1] == 0x55) && 
			  (GetResponse[2] == 0x03) && (GetResponse[3] == 0x01)) {
			if (!GetResponse[6]) { //OK
				while (!Exit_While) {
					value = GetResponse[8] + (GetResponse[9] * 256);
		//-----------------------------------------------------
					if (value == 0xFFF1) { //65521
						status = 1;
						if (fromAndroid[0] == 'C') {
							FPRNCancelCmd();
							IdentifyFlag = buffIdentifyFlag;
							HAL_Delay(1);
							#ifdef MaxReciveBuf
							rxBufferHeadPos = 0;
							#endif
							fromAndroid[0] = 0;
							send("{\"Receive\":{\"Status\":26},\"F\":{\"Type\":\"Cansel\"}}\n");  
							return 0;
						}
						IdentifyFlag = 0;
						sprintf(ResBuff, "{\"Receive\":{\"Status\":26},\"F\":{\"Type\":\"Enroll\",\"Status\":%d}}\n", value);
						send(ResBuff);
						memset(ResBuff, '\0', sizeof(ResBuff));
						HAL_Delay(400);
						retu++;
						WDTR
						bufferLen = 0;
					}
		//-------------------------------------------------------
					if (value == 0xFFF2) { //65522
						status = 2;
						if (fromAndroid[0] == 'C') {
							FPRNCancelCmd();
							IdentifyFlag = buffIdentifyFlag;
							HAL_Delay(1);
							#ifdef MaxReciveBuf
							rxBufferHeadPos = 0;
							#endif
							fromAndroid[0] = 0;
							send("{\"Receive\":{\"Status\":26},\"F\":{\"Type\":\"Cansel\"}}\n");  
							return 0;
						}
						IdentifyFlag=0; 
						sprintf(ResBuff,"{\"Receive\":{\"Status\":26},\"F\":{\"Type\":\"Enroll\",\"Status\":%d}}\n",value);				
						send(ResBuff);
						memset(ResBuff,'\0',sizeof(ResBuff));
						HAL_Delay(400);
						bufferLen=0;
						retu++;
						WDTR
					}
					//--------------------------------------------------------
					if (value == 0xFFF3) { //65523
						status = 3;
						if (fromAndroid[0] == 'C') {
							FPRNCancelCmd();
							IdentifyFlag = buffIdentifyFlag;
							HAL_Delay(1);
							#ifdef MaxReciveBuf
							rxBufferHeadPos = 0;
							#endif
							fromAndroid[0] = 0;
							send("{\"Receive\":{\"Status\":26},\"F\":{\"Type\":\"Cansel\"}}\n");  
							return 0;
						}
						IdentifyFlag = 0;
						sprintf(ResBuff, "{\"Receive\":{\"Status\":26},\"F\":{\"Type\":\"Enroll\",\"Status\":%d}}\n",value);
						send(ResBuff);
						memset(ResBuff, '\0', sizeof(ResBuff));
						HAL_Delay(400);
						bufferLen = 0;
						retu++;
						WDTR
					}
					//------------------------------------------------------
					if (value == 0xFFF4) { //65524
						retu++;
						IdentifyFlag = 0;
						sprintf(ResBuff, "{\"Receive\":{\"Status\":26},\"F\":{\"Type\":\"Enroll\",\"Status\":%d}}\n", value);
						send(ResBuff);
						memset(ResBuff,'\0',sizeof(ResBuff));
						HAL_Delay(400);
						bufferLen = 0;
						value = GetResponse[8] + (GetResponse[9] * 256);	
						WDTR
					}
					//------------------------------------------------------																		 
					if ((value != 0xFFF3) && (value != 0xFFF2) && (value != 0xFFF1) && 
							(value != 0xFFF4) && (value != 0x00)) {
						Exit_While = 1;
						retu = 0;
						Try  = 2;
					}
					//-----------------------------------------------------
					if (retu == 60) { 
						retu = 0; 
						FPRNCancelCmd(); 
						memset(GetResponse,'\0',sizeof(GetResponse)); 
						IdentifyFlag = buffIdentifyFlag;  
						send("{\"F\":{\"Type\":\"Enroll\",\"Status\":18}}\n"); 
						return 0;
					}
				} //While
			}// if(!GetResponse[6]
		}
		else { 
			HAL_Delay(100);
			memset(GetResponse, '\0', sizeof(GetResponse));		
			bufferLen = 0;
			break;
		}
	}
	//----------------------------------------------------------------------------
	if ((GetResponse[0] == 0xAA) && (GetResponse[1] == 0x55))
		if (GetResponse[6]) {//Fail
			value = GetResponse[8] + (GetResponse[9] * 256); 
			memset(ResBuff, '\0', sizeof(ResBuff));

			if ((value == 0x60) || (value == 0x14) || (value == 0x41) || 
					(value == 0x23) || (value == 0x21) || (value == 0x30) || 
					(value == 0x19) || (value == 0x51))
				sprintf(ResBuff,"{\"F\":{\"Type\":\"Enroll\",\"Status\":%d}}\n",value);
			else
				sprintf(ResBuff,"{\"F\":{\"Type\":\"Enroll\",\"Status\":%d}}\n",33);

		send(ResBuff);
		bufferLen = 0;
		IdentifyFlag = buffIdentifyFlag;
		FPRNCancelCmd();
		HAL_Delay(100);
		memset(GetResponse,'\0',sizeof(GetResponse));
		return 0;
	}

	if (Exit_While == 1)  
		send("{\"F\":{\"Type\":\"Enroll\",\"Status\":0}}\n");
	HAL_Delay(1);
	memset(GetResponse, '\0', sizeof(GetResponse));
	bufferLen = 0;
	IdentifyFlag = 1;
	return 0;
}
//===================================================================================
unsigned int FPRN_GetEnrollCount(void) {
	unsigned char ResBuff[1] = {0};
	FPRN_CancelCmd();
  HAL_Delay(100);
	FPRN_Send_Command(0x0128, 0, ResBuff);
	HAL_Delay(100);
	if (GetResponse[0] == 0xAA)
		if (!GetResponse[6])
	    return (GetResponse[8] + (GetResponse[9] * 256));
		else 
			return 0;
	else 
		return 0;
}
	
void FPRN_GetMemoryFilled(void) {
	unsigned char ResBuff[10] = {0};
	char Status;
	short ID = 0, cnt = 0, cntID = 0;
	
	ID = cnt = cntID = 0;
	cntID = FPRN_GetEnrollCount();
	bufferLen = 0;
	send("Sector Filled,");
	FPRN_CancelCmd();
  HAL_Delay(100); 
  FPRN_Send_Command(0x0107, 0, ResBuff);
  HAL_Delay(100);	
	if (GetResponse[0] == 0xAA)
		if (!GetResponse[6])
			ID = GetResponse[8] + (GetResponse[9] * 256);
		else 
			ID = 1;
  else 
		ID = 1;
	if (ID > 1)
		for (short cntt = 1; cntt<ID; cntt++) {
			sprintf(ResBuff,"%d,",cntt); 
			send(ResBuff);
			cnt++;
		}

	for (; ID<=5000; ID++) {	
		__HAL_IWDG_RELOAD_COUNTER(&hiwdg);
		bufferLen = 0;
	  FPRN_CancelCmd();
		HAL_Delay(40); 
		ResBuff[0] = ID % 256;
		ResBuff[1] = ID / 256;
		FPRN_Send_Command(0x0103, 2, ResBuff);
		HAL_Delay(70);
		Status = GetResponse[8] + (GetResponse[9] * 256);
		if (GetResponse[6])
			if (Status == 20) {
				sprintf(ResBuff,"%d,",ID); 
				send(ResBuff);
				memset(ResBuff,'\0',sizeof(ResBuff));
				cnt++;
			}
		__HAL_IWDG_RELOAD_COUNTER(&hiwdg);
		if (cnt == cntID) 
			break;
	}
	send("END");
	sprintf(ResBuff, "\nCount:%d\n", cnt);
	send(ResBuff);
}

void testConnection(void) {
	unsigned char ResBuff[1] = {0};
//	FPRN_CancelCmd();
//  HAL_Delay(100);
	FPRN_Send_Command(0x0150, 0, ResBuff);
	HAL_Delay(100);
	if ((GetResponse[0] == 0xAA) && (GetResponse[1] == 0x55) && 
			(GetResponse[2] == 0x50) && (GetResponse[3] == 0x01) && 
			(GetResponse[4] == 0x04))
		send("{\"F\":{\"Type\":\"TestConnection\",\"Status\":0}}\n");
	else 
		send("{\"F\":{\"Type\":\"TestConnection\",\"Status\":18}}\n");	
	HAL_Delay(2);
	memset(GetResponse,'\0',sizeof(GetResponse));
}
//=====================================================
unsigned char FPRN_GetSecurityLevel(void) {
	unsigned char ResBuff[50] = {0}, RetValue = 0; 
	FPRN_CancelCmd();
  HAL_Delay(100);
	FPRN_Send_Command(0x010D, 0, ResBuff);
  HAL_Delay(100);
//	return ResBuff[8];
  memset(GetResponse, '\0', sizeof(GetResponse));
}
//==============================================================================
unsigned char FPRN_SetSecurityLevel(unsigned char level) {
	unsigned char ResBuff[50] = {0}, RetValue = 0;   
	FPRN_CancelCmd();
  HAL_Delay(100);
  ResBuff[0] = 1;
  ResBuff[1] = 1;
	ResBuff[2] = level;	
	FPRN_Send_Command(0x013A, 3, ResBuff);
 	memset(GetResponse, '\0', sizeof(GetResponse));
}
unsigned char GetFrimware(void) {
	unsigned char ResBuff[100] = {0};
	FPRN_CancelCmd();
  HAL_Delay(100);
	unsigned char pk[] = {0x55,0xAA,0x12,0x01,0x00,0x00,0x00,0x00,0x00,0x00
		,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x12,0x01};
	HAL_UART_Transmit(&huart2,(unsigned char*)pk ,25,50);
  HAL_Delay(100);
  if ((GetResponse[0] == 0xaa) && (GetResponse[1] == 0x55)) {
		sprintf(ResBuff,"{\"Receive\":{\"Status\":26},\"F\":{\"Type\":\"Frimware\",\"Status\":%d.%d}}\n",GetResponse[8],GetResponse[9]);	
    send(ResBuff);		
	}
	memset(GetResponse,'\0',30);
}
#endif