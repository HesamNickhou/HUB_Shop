#ifndef ECARD_H
#define ECARD_H



#define Increment   1
#define Decrement   2

//===== RC522
#define S_Success          0
#define S_TimeOut          1
#define S_Restore          2
#define S_OutOfRange       3
#define S_InvalidSerial    4
#define S_InvalidType      5
#define S_IsOldFormat      6
#define S_BlackList        7
#define S_SuspendOperation 8
#define S_InvalidCardID    9
#define S_InvalidCardUC    10
#define S_SettingCard      11
#define S_TimeOut2         12
#define S_CardSuspend      13
#define S_RestoreOld       14
#define S_RestoreIndex     15
#define S_BajedarCard      16
#define S_ChargeCard       17
#define S_Cancel           18
#define S_Error           255

#define S_InvalidReciveData 19

//========Finger
#define placeFingerOnSensor1 65521 // angosht bezatid,bare aval
#define placeFingerOnSensor2 65522 // angosht bezatid,bare dovom
#define placeFingerOnSensor3 65523 // angosht bezatid,bare sevom
#define pickUpFinger 65524 //angosht ro bardarid
#define badFingerPrintImageQuality 33 // keyfiat pain ast
#define notFindTemplateTryAgin 22 //fingerprint in angosht mojod nis
#define fingerprintTemplateEmpty 21 // hafeze khali ast
#define numberInvalid 96 // shomare namotabar ya vojod nadarad 
#define numberDuplicate 20 //shomare tekrari ast
#define Canceled 65 
#define error 35
#define inequalityFingerprint 48 //adam yeksan bodan asar angoshtha
#define fingerprintRegistered 25 //asar angosht sabt shode ast
#define SoftwareError 81
#define NoDataInTemplate 19     // dar shomare olgo moshakhas shode asare angoshti vojod nadare 
#define UnSuccess 100     // write template UnSuccess
#define UnSuccess 112     // parameter namotabar ast 

//======Relay
#define on 23
#define off 24

#define received 26
#define S_InvalidReciveData 19
#define resetMicro 70

#define errorSetTime 128
#define errorSetDate 129 

#define ErrorMax30102 130

typedef struct {
  unsigned char 		CardSerialNumber[4];
  unsigned long int CardID;
  unsigned int 			UC;
  unsigned char 		OPIndex;
	unsigned char 		OP;
  unsigned int  		DeviceID;  
  unsigned char  		Hour, Min;  
  unsigned int  		Payment;  
  signed long int 	Etebar;  
  signed long int 	PreEtebar;  
  unsigned char 		EtebarBlock[16];  
  unsigned char 		OPBlock[16];  
} TECardInfo;

typedef struct {
  unsigned char 	OPIndex;
  unsigned char 	OP;
  unsigned int  	DeviceID;  
  unsigned char 	Date[3];  
  unsigned char 	Time[2];  
  unsigned int  	Payment;  
  signed long int Etebar;  
  signed long int PreEtebar;
  unsigned char 	BCC;
} TCardOP;

typedef struct { 
	 signed  int temp;  
} CardInfo4Send; 
extern unsigned char CreditSector;
extern unsigned char CreditBlock;
extern unsigned char CreditLogin;
extern unsigned char SOLUTION; 

//extern unsigned char ExecTransaction(unsigned char Type, unsigned long int *CardID, unsigned int *UC, unsigned int Payment,
//                                    signed long int MinEtebar, unsigned long int MaxEtebar, unsigned int DeviceID, 
//                                    signed long int *Etebar, signed long int *PreEtebar, unsigned short *LastDevice, unsigned char *LastOP, unsigned char *OPIndex);
																		
//extern unsigned char Old_ExecTransaction(unsigned char Type, unsigned long int *CardID, unsigned int *UC, unsigned int Payment,
//                                    signed long int MinEtebar, unsigned long int MaxEtebar, unsigned int DeviceID, 
//                                    signed long int *Etebar, signed long int *PreEtebar, int *LastDevice, unsigned char *LastOP, unsigned char *OPIndex);

unsigned char Old_ExecTransaction(unsigned char Type, unsigned long int *CardID, unsigned int *UC, unsigned int Payment,
                                    signed long int MinEtebar, unsigned long int MaxEtebar, unsigned int DeviceID, 
                                    signed long int *Etebar, signed long int *PreEtebar, int *LastDevice, unsigned char *LastOP, unsigned char *OPIndex);

extern unsigned char GetCardEtebar(signed int *Etebar);
char Write_GroupOfCard(char SabaGroup);
void ReadCardTable(unsigned char Row);
unsigned char IncrementCreditToCard(unsigned long ID, signed long int Payment,unsigned int HPayment, unsigned char rqid);
																
#endif //#ifndef ECARD_H

