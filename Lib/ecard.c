//------------------------------------------------------------------------------
//         Headers
//------------------------------------------------------------------------------
//#ifdef ECard

#include "ecard.h"
#include <stdio.h>
#include "string.h"
#include "main.h"
#include "softuart.h"
#include "define.h"
#include "flash.h"
#define WDTR __HAL_IWDG_RELOAD_COUNTER(&hiwdg);
#define u8  unsigned char
#define u16 unsigned short
#define size 600
extern unsigned char useGift;
extern SoftUart_S SUart;
extern void send(unsigned char *buff);
extern unsigned int deviceID;
extern signed long int MINEtebar;
extern unsigned long int MAXEtebar;
extern unsigned char Year;
extern unsigned char Month;
extern unsigned char Day;
extern unsigned char Hour;
extern unsigned char Min;
extern unsigned char Sec;
extern 	_Bool flagRc522;
extern unsigned char buffTransaction[300];
extern unsigned char sendingResult[300];
extern unsigned int	requestID, preRequestID;
typedef struct {	
	unsigned short DeviceID;
	unsigned int  UC;
	unsigned int  BusID;
	unsigned int OperatorID;
	unsigned long int MaxEtebar;
	signed char MinEtebar;
} TConfig;
extern TConfig Config;
extern unsigned int CDCReceiveLen;
extern unsigned char fromAndroid[size];
char DontChangeDebit=0,ResetCardFlag=0,ResetCardFlag2=0;
extern _Bool zeroEtebarFlag;
extern unsigned int configUc;
extern volatile unsigned long int cardId;
extern volatile unsigned int uc;
//unsigned int Temp=0;
extern unsigned char globalBuff[50];
extern IWDG_HandleTypeDef hiwdg;

int InfoCounter=0,Li=0;
CardInfo4Send tempSend;
//sssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss
extern unsigned char SaveTransactionRecord(unsigned char *Buf,unsigned char len);
extern void SaveRingDetail(void);
extern void sendCRC(const char* buffer);
extern unsigned char getCRC(const char* buffer);
extern void sendDebug(unsigned char* data, unsigned int len);
typedef  unsigned int  size_t;
unsigned char HediyeBuf[16];
unsigned char changeGp = 0;
#if  defined finger
extern unsigned int rxBufferHeadPos;
#endif
#ifdef MaxReciveBuf
extern unsigned int rxBufferHeadPos; 
#endif

//------------------------------------------------------------------------------
//         Internal constants
//------------------------------------------------------------------------------
unsigned char CreditSector = 5;
unsigned char CreditBlock  = 20;
unsigned char CreditLogin  = 0x37;
unsigned char SOLUTION 		 = 0; 
#define ctNone     0
#define ctSABA     1
#define ctTaxi     2
#define ctParyaft  3

#if (deviceType==AmusementPark) || defined(ExecTransactionAPARK)
unsigned char CoderType=ctSABA;
#else
unsigned char CoderType=ctNone;
#endif
//------------------------------------------------------------------------------
//         Exported functions
//------------------------------------------------------------------------------
//#include <utility/eiaes.h>

#define MAXSUSPENDCARDS 2
#define OLD_MAXSUSPENDCARDS 5

TECardInfo CurECardInfo;
TECardInfo SuspendECardsInfo[MAXSUSPENDCARDS];
TECardInfo Old_SuspendECardsInfo[OLD_MAXSUSPENDCARDS];
TCardOP CardOPs[15];

unsigned long int CardIDThatProcessed = 0;
unsigned char Old_RestoreIndex(void);
unsigned char Old_RestoreEtebar(void);  
//extern void ShowCardHoldMessage(void);

char LastStep = 0;

//==========================================================================
//    Prototypes
//==========================================================================
unsigned char Old_GetBlockIndex(unsigned char Idx) {
  switch (Idx) {
    case 0:  return 25; 
    case 1:  return 26; 
    case 2:  return 28; 
    case 3:  return 29; 
    case 4:  return 30; 
    case 5:  return 32; 
    case 6:  return 33; 
    case 7:  return 34; 
    case 8:  return 36; 
    case 9:  return 37; 
    case 10: return 38; 
    case 11: return 40; 
    case 12: return 41; 
    case 13: return 42; 
		default: return 36;
  }
}
//==========================================================================
unsigned char GetCardEtebar(signed int *Etebar) {
	signed long int Payment, PreEtebar;
	unsigned char BCC, i, OP;
	unsigned char snr[8];
	unsigned char buf[32], str[500];
	unsigned char IsOldFormat;
	signed long int Value;
	signed long int TempEtebar=0;

	*Etebar = 0;
	switch (CoderType) {
		case ctTaxi:
			CreditSector = 5;
			CreditBlock  = 21;
			CreditLogin  = 0x37;
			SOLUTION 		 = 0x20; 
		break;
		case ctSABA:    //(deviceType==AmusementPark) || defined(ExecTransactionAPARK)
			CreditSector = 4;
			CreditBlock  = 16;
			CreditLogin  = 0x39;
			SOLUTION 		 = 0;
		break;
		case ctParyaft:
			CreditSector = 5;
			CreditBlock  = 21;
			CreditLogin  = 0x37;
			SOLUTION 		 = 0x40; 
		break;
		default:
			CreditSector = 5;
			CreditBlock  = 20;
			CreditLogin  = 0x37;
			SOLUTION 		 = 0; 
		break;
	}

	if (ISO14443_SingleTagSelect(snr)) 
		if (ISO14443_SingleTagSelect(snr)) 
			return S_TimeOut;

	for (i=0; i<4; i++)
		CurECardInfo.CardSerialNumber[i] = snr[i];

	if (ISO14443_LoginE2(0, 0x10)) 
		return(S_TimeOut);

	if (ISO14443_ReadBlock(1, buf))
		return S_TimeOut;

  CurECardInfo.UC   = buf[1];
	CurECardInfo.UC <<= 8;
  CurECardInfo.UC  += buf[0];
  CurECardInfo.CardID  = buf[5];
	CurECardInfo.CardID <<= 8;
  CurECardInfo.CardID += buf[4];
	CurECardInfo.CardID <<= 8;
  CurECardInfo.CardID += buf[3];
	CurECardInfo.CardID <<= 8;
  CurECardInfo.CardID += buf[2];
	
	if ((CurECardInfo.UC & 0x8000) == 0x8000) {
		CurECardInfo.UC 		= CurECardInfo.UC 		& 0x7FFF;
		CurECardInfo.UC 		= CurECardInfo.UC 		^ 0x6A29;
		CurECardInfo.CardID = CurECardInfo.CardID ^ 0xB2A6;
	}
  CardIDThatProcessed = CurECardInfo.CardID;
	
	if (ISO14443_LoginE2(CreditSector, CreditLogin)) 
		return 10;
	if (ISO14443_ReadBlock(CreditBlock, buf)) 
		return 20;  
	for (i=0; i<16; i++)
		CurECardInfo.EtebarBlock[i] = buf[i];
    
//  DecryptBlock(CurECardInfo.CardSerialNumber, CurECardInfo.EtebarBlock);
    
  IsOldFormat = 1;  
  for (i=0; i<4; i++) 
		if (buf[i] != buf[i+8]) 
			IsOldFormat = 0;  
  for (i=0; i<4; i++) 
		if (buf[i+4] != (buf[i] ^ 0xFF)) 
			IsOldFormat = 0;  
		
  if ((buf[12] != 0) || 
			(buf[13] != 0xFF) || 
			(buf[14] != 0) || 
			(buf[15] != 0xFF))
		IsOldFormat = 0;
	
	if (IsOldFormat) {                                      
		CurECardInfo.Etebar  = buf[3]; CurECardInfo.Etebar <<= 8;
		CurECardInfo.Etebar += buf[2]; CurECardInfo.Etebar <<= 8;
		CurECardInfo.Etebar += buf[1]; CurECardInfo.Etebar <<= 8;
		CurECardInfo.Etebar += buf[0]; 
		if (ISO14443_LoginE2(6, 0x14)) 
			return 12;
		if (ISO14443_ReadValueBlock(24, &Value)) 
			return 13;
		CurECardInfo.OPIndex = Value;		
		*Etebar = CurECardInfo.Etebar;
		return 0;
	}

  BCC=0x4B;
  for (i=0; i<15; i++)
    BCC^=CurECardInfo.EtebarBlock[i];
	if (BCC!=CurECardInfo.EtebarBlock[15])
		return (3);
	
  CurECardInfo.Etebar=CurECardInfo.EtebarBlock[3]; CurECardInfo.Etebar<<=8;    
  CurECardInfo.Etebar+=CurECardInfo.EtebarBlock[2]; CurECardInfo.Etebar<<=8;    
  CurECardInfo.Etebar+=CurECardInfo.EtebarBlock[1]; CurECardInfo.Etebar<<=8;    
  CurECardInfo.Etebar+=CurECardInfo.EtebarBlock[0];

  Payment=CurECardInfo.EtebarBlock[9]; Payment<<=8;    
  Payment+=CurECardInfo.EtebarBlock[8];

  PreEtebar=CurECardInfo.EtebarBlock[7]; PreEtebar<<=8;    
  PreEtebar+=CurECardInfo.EtebarBlock[6]; PreEtebar<<=8;    
  PreEtebar+=CurECardInfo.EtebarBlock[5]; PreEtebar<<=8;    
  PreEtebar+=CurECardInfo.EtebarBlock[4];
  
  OP = CurECardInfo.EtebarBlock[10];
  
	switch(OP) {
		case Increment:
			TempEtebar = PreEtebar + Payment;
			if (TempEtebar != CurECardInfo.Etebar)
				return 5;     
			break;
			
		case Decrement:
			TempEtebar = PreEtebar - Payment;
		if (TempEtebar != CurECardInfo.Etebar)
				return 6;
			break;
		default:
			return 7;
	}
  
  CurECardInfo.OPIndex=CurECardInfo.EtebarBlock[11] & 0x1F;    
  *Etebar=CurECardInfo.Etebar;
  return 0;        
}

//==========================================================================
unsigned char EmptyECardInfo(TECardInfo *CardInfo) {
	unsigned char i;
  (*CardInfo).CardID 		= 0;
  (*CardInfo).UC 				= 0;
  (*CardInfo).Etebar 		= 0;
  (*CardInfo).Payment 	= 0;
  (*CardInfo).PreEtebar = 0;
  (*CardInfo).OP 				= 0;
  (*CardInfo).OPIndex 	= 0;
  (*CardInfo).DeviceID 	= 0;
  (*CardInfo).Hour 			= 0;
  (*CardInfo).Min			 	= 0;
	//(*CardInfo).Sec       = 0;
  for (i=0; i<16; i++) {
    (*CardInfo).EtebarBlock[i] = 0;
    (*CardInfo).OPBlock[i] 		= 0;  
	}
  for (i=0; i<4; i++)
    (*CardInfo).CardSerialNumber[i] = 0;
    
  return 0;
}
//==========================================================================
unsigned char GetOpBlockByIndex(unsigned char idx) {
  switch (idx) {
    case 0: return(25);
    case 1: return(26);
    case 2: return(28);
    case 3: return(29);
    case 4: return(30);
    case 5: return(32);
    case 6: return(33);
    case 7: return(34);
    case 8: return(36);
    case 9: return(37);
    case 10: return(38);
    case 11: return(40);
    case 12: return(41);
    case 13: return(42);
  }
  return(0);
}

//==========================================================================
unsigned char Old_RestoreEtebar(void) {
	int i;
	signed long int loc, Etebar;
	unsigned char buf[32];
	unsigned char Buffer[16];
	unsigned char Res, j, BCC = 0, Block;

  //Recovery
  if (ISO14443_LoginE2(6, 0x14))
    return 1;
  if (Res = ISO14443_ReadValueBlock(24, &loc)) {
    if (Res == 2) {
      Old_RestoreIndex();
      return 1;   
    }
  }
	else {
    if (loc > 13) {
      Old_RestoreIndex();
      return 1;
    }
    if (loc == 0)
      loc = 13;
    else
      loc--;  
  }//if (MSR_ReadVa  
  
  
  //SendStr("Loc="); SendNum(loc); SendStrLn("");
  Block = GetOpBlockByIndex(loc);
  //SendStr("Block="); SendNum(Block); SendStrLn("");
  if (ISO14443_LoginE2(Block / 4, 0x35))
    return 8;
  //SendStrLn("Read");
  if (ISO14443_ReadBlock(Block, buf))
    return 9;
  //SendStrLn("OK");
  BCC = 0;
  for (i=0; i<15; i++)
    BCC ^= buf[i]; 
  //SendStr("BCC="); SendNum(ReaderReceiveBuf[18]); SendStrLn("");
  //SendStr("CalcBCC="); SendNum(BCC); SendStrLn("");
  if (buf[15] != BCC) {
     Old_RestoreIndex();
     return 10;
	}

  Etebar  = buf[10]; Etebar <<= 8;
  Etebar += buf[9];  Etebar <<= 8;
  Etebar += buf[8]; 
              
  //SendStr("Etebar="); SendNum(Etebar); SendStrLn("");
  if (ISO14443_LoginE2(CreditSector, CreditLogin)) 
		return 11;
  if (ISO14443_WriteValue(CreditBlock, Etebar)) 
		return 12;
  return 0;
}

//==========================================================================
unsigned char Old_RestoreIndex(void) {
	int i;
	unsigned char buf[32];
	unsigned char loc;
	unsigned char CurYear, CurMonth, CurDay, CurHour, CurMin;
	unsigned char OldYear=0, OldMonth=0, OldDay=0, OldHour=0, OldMin=0;
	unsigned char j, BCC=0, Block; 
	unsigned long int CurEtebar, OldRemEtebar=0; 

  loc=200;
  for (i=0; i<14; i++)
  {              
    Block=Old_GetBlockIndex(i);
    if (ISO14443_LoginE2(Block/4,0x35)) return(3);                
    if (ISO14443_ReadBlock(Block, buf)) return(4);
    BCC=0;
    for (j=0; j<15; j++)
      BCC^=buf[j]; 
    if ((buf[15]==BCC) || (BCC==0) || (BCC==0xFF)) //Invalid OP 
      continue;

    CurYear=(buf[6]>>4)+86;
    CurMonth=buf[6] & 0x0F;
    CurDay=buf[7];
    CurHour=buf[13];
    CurMin=buf[14];   

    CurEtebar=buf[4]; CurEtebar<<=8;
    CurEtebar+=buf[3]; CurEtebar<<=8;
    CurEtebar+=buf[2];
    
    if ((CurMonth>12) || (CurDay>31) || (CurHour>23) || (CurMin>59))
      continue;

    if (CurYear>OldYear) //Year
      loc=i;
    else if (CurYear==OldYear) //Year
    {
      if (CurMonth>OldMonth)  //Month
        loc=i;
      else if (CurMonth==OldMonth)
      {
        if (CurDay>OldDay) //Day
          loc=i;
        else if (CurDay==OldDay)
        {
          if (CurHour>OldHour)
            loc=i;          
          else if (CurHour==OldHour)
          {
            if (CurMin>OldMin)
              loc=i;
            else if (CurMin==OldMin)
            {
              if (OldRemEtebar==CurEtebar)
                loc=i;
            }  
          }  
        }
      }  
    }    
    
    if (i==loc)
    {
      OldYear=CurYear;
      OldMonth=CurMonth;
      OldDay=CurDay;
      OldHour=CurHour;
      OldMin=CurMin;  
      OldRemEtebar=buf[10]; OldRemEtebar<<=8;
      OldRemEtebar+=buf[9]; OldRemEtebar<<=8;
      OldRemEtebar+=buf[8];
    }   
  }//for 

  if (loc==200)
    loc=13;   
    
  loc++;
  if (loc>13) loc=0;  

  if (ISO14443_LoginE2(6,0x35))
    return(1);                
  if (ISO14443_WriteValue(24, loc))
    return(1); 

  return(0);
}



//==========================================================================
unsigned char Old_ReadECardInfo(void) {
	unsigned char BCC, i;
	unsigned char buf[32];
	signed long int Value;

//   SetTX485(); printf("\n\rOld_ReadECardInfo"); SetRX485();
  if (ISO14443_LoginE2(0, 0x10)) 
		return 1;
  if (ISO14443_ReadBlock(1, buf)) 
		return 2; 
	//if (buf[6] != 0xab) return(5); 
  
	CurECardInfo.UC  = buf[1];  		 //UC
	CurECardInfo.UC <<= 8;
  CurECardInfo.UC += buf[0];
	
  CurECardInfo.CardID   = buf[5];  //CardID
	CurECardInfo.CardID <<= 8;
  CurECardInfo.CardID  += buf[4]; 
	CurECardInfo.CardID <<= 8;
  CurECardInfo.CardID  += buf[3]; 
	CurECardInfo.CardID <<= 8;
  CurECardInfo.CardID  += buf[2];
	
  if ((CurECardInfo.UC & 0x8000) == 0x8000) {
    CurECardInfo.UC 		= CurECardInfo.UC 		& 0x7FFF;
    CurECardInfo.UC 		= CurECardInfo.UC 		^ 0x6A29;
    CurECardInfo.CardID = CurECardInfo.CardID ^ 0xB2A6;
  }
  CardIDThatProcessed = CurECardInfo.CardID;
  
  if (ISO14443_LoginE2(6, 0x14)) 
		return 10;                
  if (ISO14443_ReadValueBlock(24, &Value)) 
		return 11;
  CurECardInfo.OPIndex = Value;

  if (Value == 0)
    Value = 13;
  else
    Value--;
	
  WDTR 
  i = Old_GetBlockIndex(Value);
  if (ISO14443_LoginE2(i / 4, 0x35)) 
		return 13;                
  if (ISO14443_ReadBlock(i, buf)) 
		return 14;
	
  CurECardInfo.OP = buf[5]; //OP
	
  CurECardInfo.DeviceID   = buf[12];  //Device ID
	CurECardInfo.DeviceID <<= 8;
  CurECardInfo.DeviceID  += buf[11];     

  if (ISO14443_LoginE2(CreditSector, CreditLogin)) 
		return 12;
  if (ISO14443_ReadValueBlock(CreditBlock, &CurECardInfo.Etebar)) {       
    //SendStrLn("Restore...");


    //if (Old_RestoreEtebar()) //ali 14040730
    //  return 3;
    //else 
    //  return 4; //Restore Succeed
		return 15;
  }
	
	
  return 0;      
}

//==========================================================================
unsigned char Old_PrepairECardBuffer(void) {
	unsigned char i, BCC;
  CurECardInfo.OPBlock[0]  = 	Sec & 0x00FF;
  CurECardInfo.OPBlock[1]  = (Sec >> 8);
  CurECardInfo.OPBlock[2]  =  CurECardInfo.PreEtebar 				& 0x000000FF;
  CurECardInfo.OPBlock[3]  = (CurECardInfo.PreEtebar >> 8)  & 0x000000FF;
  CurECardInfo.OPBlock[4]  = (CurECardInfo.PreEtebar >> 16) & 0x000000FF;
  CurECardInfo.OPBlock[5]  =  CurECardInfo.OP;
  CurECardInfo.OPBlock[6]  = ((Year) << 4) + (Month & 0x0F); // shift left 
  CurECardInfo.OPBlock[7]  =  Day;
  CurECardInfo.OPBlock[8]  =  CurECardInfo.Etebar 			 & 0x000000FF;
  CurECardInfo.OPBlock[9]  = (CurECardInfo.Etebar >> 8)  & 0x000000FF;
  CurECardInfo.OPBlock[10] = (CurECardInfo.Etebar >> 16) & 0x000000FF;
  CurECardInfo.OPBlock[11] =  CurECardInfo.DeviceID 		  & 0xFF;
  CurECardInfo.OPBlock[12] = (CurECardInfo.DeviceID >> 8) & 0xFF;  
  CurECardInfo.OPBlock[13] =  Hour;  
  CurECardInfo.OPBlock[14] =  Min;  
  BCC = 0;
  for (i=0; i<15; i++)
    BCC ^= CurECardInfo.OPBlock[i];
  CurECardInfo.OPBlock[15] = BCC;    
  return 0;
}
//==========================================================================
unsigned char Old_WriteTransaction(void) {
	unsigned char Block, snr[10];
	signed long int Etebar;

  Block = Old_GetBlockIndex(CurECardInfo.OPIndex);
  switch (LastStep) {  //ali

		case 0: {
			//============================ STEP 0 =========================
			LastStep = 0;			
			//Write the main credit into the Card
			if (ISO14443_SingleTagSelect(snr))
			if (ISO14443_SingleTagSelect(snr))
				return 1;
				
			if (ISO14443_LoginE2(CreditSector, CreditLogin))
			if (ISO14443_LoginE2(CreditSector, CreditLogin)) 
				return 1;

			if (ISO14443_WriteValue(CreditBlock, CurECardInfo.Etebar)) 
				return 2;
			//=============================================================
			
			//========================== STEP 1 ===========================
			LastStep = 1;
			//Write OP record
			if (ISO14443_LoginE2(Block / 4, 0x35)) 
				return 3;
			if (ISO14443_WriteBlock(Block, CurECardInfo.OPBlock))  
				return 4;
			
			//Write the last index into the Block24
			if (ISO14443_SingleTagSelect(snr))
				if (ISO14443_SingleTagSelect(snr))
					return 5;
			if (ISO14443_LoginE2(6, 0x35))
				if (ISO14443_LoginE2(6, 0x35)) 
					return 5;
				
			Block = CurECardInfo.OPIndex + 1;
			if (Block > 13)
				 Block = 0;  
			if (ISO14443_WriteValue(24, Block)) 
				return 6;
			//==============================================================
			
			//=========================== STEP 2 ===========================
			LastStep = 2;
			//Write the Gift credit into the Card
			if (CoderType == ctSABA) {
				if (ISO14443_LoginE2(4, 0x39)) 
					return 7;
				if (ISO14443_WriteBlock(18, HediyeBuf))  
					return 8;
				if (ISO14443_LoginE2(CreditSector, CreditLogin)) 
					return 9;
			}
			//=============================================================
			
			//=========================== STEP 3 ==========================
			LastStep = 3;
			//Check the wrote value
			if (ISO14443_ReadValueBlock(CreditBlock, &Etebar)) 
				return 10;
			//if (ISO14443_ReadValueBlock(18, HediyeBuf))  //ali  ezaafeh shavad
			//	return 7;

			if (Etebar != CurECardInfo.Etebar)
				return 11;
			//if (HediyeBuf[0] != CurECardInfo.Etebar)
      //return 8;      //ali  ezaafeh shavad
			//=============================================================
			
			LastStep = 0;
			break;
		}
		
		case 1: {
			//============================ STEP 1 ===========================
			LastStep = 1;
			//Write OP record
			if (ISO14443_LoginE2(Block / 4, 0x35)) 
				return 3;
			if (ISO14443_WriteBlock(Block, CurECardInfo.OPBlock))  
				return 4;
			
			//Write the last index into the Block24
			if (ISO14443_SingleTagSelect(snr))
				if (ISO14443_SingleTagSelect(snr))
					return 5;
			if (ISO14443_LoginE2(6, 0x35))				
				if (ISO14443_LoginE2(6, 0x35)) 
					return 5;
			Block = CurECardInfo.OPIndex + 1;
			if (Block > 13)
				 Block = 0;  
			if (ISO14443_WriteValue(24, Block)) 
				return 6;
			//==============================================================
			
			//============================ STEP 2 ==========================
			LastStep = 2;
			//Write the Gift credit into the Card
			if (CoderType == ctSABA) {
				if (ISO14443_LoginE2(4, 0x39)) 
					return 7;
				if (ISO14443_WriteBlock(18, HediyeBuf))  
					return 8;
			}
			//==============================================================
			//=========================== STEP 3 ===========================
			LastStep = 3;
			//Check the wrote value
			if (ISO14443_LoginE2(CreditSector, CreditLogin)) 
					return 9;
			if (ISO14443_ReadValueBlock(CreditBlock, &Etebar)) 
				return 10;
			//if (ISO14443_ReadValueBlock(18, HediyeBuf))  //ali  ezaafeh shavad
			//	return 7;

			if (Etebar != CurECardInfo.Etebar)
				return 11;
			//if (HediyeBuf[0] != CurECardInfo.Etebar)
      //return 8;      //ali  ezaafeh shavad
			//==============================================================
			LastStep = 0;
			break;
		}
		
		case 2: {
			//=========================== STEP 2 ===========================
			//Check the wrote value
			LastStep = 2;
			//Write the Gift credit into the Card
			if (CoderType == ctSABA) {
				if (ISO14443_LoginE2(4, 0x39)) 
					return 7;
				if (ISO14443_WriteBlock(18, HediyeBuf))  
					return 8;
			}
			//=============================================================
			
			//=========================== STEP 3 ==========================
			LastStep = 3;
			if (ISO14443_LoginE2(CreditSector, CreditLogin)) 
					return 9;
			if (ISO14443_ReadValueBlock(CreditBlock, &Etebar)) 
				return 10;
			//if (ISO14443_ReadValueBlock(18, HediyeBuf))  //ali  ezaafeh shavad
			//	return 7;

			if (Etebar != CurECardInfo.Etebar)
				return 11;
			//if (HediyeBuf[0] != CurECardInfo.Etebar)
      //return 8;      //ali  ezaafeh shavad
			//=============================================================
			
			LastStep = 0;
			break;
		}
		
		case 3: {
			//=========================== STEP 3 ==========================
			//Check the wrote value
			LastStep = 3;
			if (ISO14443_LoginE2(CreditSector, CreditLogin)) 
					return 9;
			if (ISO14443_ReadValueBlock(CreditBlock, &Etebar)) 
				return 10;
			//if (ISO14443_ReadValueBlock(18, HediyeBuf))  //ali  ezaafeh shavad
			//	return 7;

			if (Etebar != CurECardInfo.Etebar)
				return 11;
			//if (HediyeBuf[0] != CurECardInfo.Etebar)
      //return 8;      //ali  ezaafeh shavad
			//=============================================================
			
			LastStep = 0;
			break;
		}
		
	}
  return 0;    
} 

//==============================================================================
unsigned char Old_ExecTransaction(unsigned char 		Type, 
																	unsigned long int *CardID, 
																	unsigned int 			*UC, 
																	unsigned int 			Payment,
                                  signed long int 	MinEtebar, 
																	unsigned long int MaxEtebar, 
																	unsigned int 			DeviceID, 
																	signed long int 	*Etebar, 
																	signed long int 	*PreEtebar, 
																	int 							*LastDevice, 
																	unsigned char 		*LastOP, 
																	unsigned char 		*OPIndex) {
	unsigned int 
		SelectTimeout = 0, 
		Grouh, 
		GiftCredit;
	
	unsigned char 
		Res, 
		j, 
		i, 
		CardIsSuspend = 0,
		Try 					= 0,
		snr[10], 
		buf[32];
	
	signed int Diff;
	unsigned long int TempEtebar = 0;
	char str[100];	
	
  switch (CoderType)  {
		
    case ctTaxi:
      CreditSector = 5;
      CreditBlock  = 21;
      CreditLogin  = 0x37;
      SOLUTION 		 = 0x20; 
      break;
		
    case ctSABA:
      CreditSector = 4;
      CreditBlock  = 16;
      CreditLogin  = 0x39;
      SOLUTION 		 = 0; 
      break;
		
    case ctParyaft:
      CreditSector = 5;
      CreditBlock  = 21;
      CreditLogin  = 0x37;
      SOLUTION 		 = 0x40; 
      break;
		
    default:  
      CreditSector = 5;
      CreditBlock  = 20;
      CreditLogin  = 0x37;
      SOLUTION 		 = 0; 
      break;
  }
  LastStep 	 = 0;
  *Etebar    = 0;                                                 
  *PreEtebar = 0;

	j = 46;
  while (SelectTimeout < 140) {
		//sprintf(buf, "[%02X] ", SelectTimeout);
		//for (i=0; i<strlen(buf); i++)
		//pos_SendSerialData(&j);
		if (ISO14443_SingleTagSelect(snr))
			if (ISO14443_SingleTagSelect(snr)) {
				SelectTimeout++;
				continue;
			}
		WDTR
		SelectTimeout++;
		for (i=0; i<4; i++)
			CurECardInfo.CardSerialNumber[i] = snr[i];
		//*CardID = CurECardInfo.CardID;
		Res 		= Old_ReadECardInfo();
		if (Try > 7)
			return S_Error;

		if (Res) {
			//*CardID = CurECardInfo.CardID;
			switch(Res) {
				case 4:
					return S_RestoreOld;
				case 11: 
					Try++; 
					continue;
				default:
					continue;
			}
		}

		if (*CardID > 0)
			if (CurECardInfo.CardID != *CardID)
				return S_InvalidCardID;
    //*CardID = CurECardInfo.CardID;
			
    if (CurECardInfo.UC != *UC) {
      *UC = CurECardInfo.UC;        
      if (CurECardInfo.UC == 251)
        return S_SettingCard;
      else  
        return S_InvalidCardUC;
    }
    *UC = CurECardInfo.UC;
    
    if (ISO14443_LoginE2(CreditSector, CreditLogin)) 
			continue;
    if (ISO14443_ReadBlock(CreditBlock, buf)) 
			continue;
    Grouh  = buf[1];  Grouh <<= 8;
    Grouh += buf[0];         
    
    *LastOP 		= CurECardInfo.OP;
    *LastDevice = CurECardInfo.DeviceID;
    *OPIndex		= CurECardInfo.OPIndex;
    //if (*CardID == 0)
      //continue;    

    //Checking for suspended card 
    for (j=0; j<OLD_MAXSUSPENDCARDS; j++) {
      for (i=0; i<4; i++)
        if (CurECardInfo.CardSerialNumber[i] != 
					Old_SuspendECardsInfo[j].CardSerialNumber[i])
          break;  
      
      if (i >= 4) {
        Diff = ((Hour * 60) + Min) - 
					((Old_SuspendECardsInfo[j].Hour * 60) + Old_SuspendECardsInfo[j].Min);

        if (Diff < 0) Diff *= -1; 
        if (Diff < 10) { //Retry write card   
          CardIsSuspend = 1;
          CurECardInfo = Old_SuspendECardsInfo[j];
        }
        //EmptyECardInfo(&Old_SuspendECardsInfo[j]);   //ali
        break;
      }     
    }
    break; //Normally done while
  }

  if (SelectTimeout >= 140)
    return S_TimeOut;

  if (CardIsSuspend == 0) {
		if (changeGp)
			Type = 82;
    *Etebar 	 						 = CurECardInfo.Etebar; 
		*PreEtebar 						 = CurECardInfo.Etebar; //HNA
    CurECardInfo.PreEtebar = CurECardInfo.Etebar;  
    CurECardInfo.Payment 	 = Payment;  
    CurECardInfo.OP 			 = Type;  
    CurECardInfo.Hour 		 = Hour;
    CurECardInfo.Min 			 = Min;
// second ezafeh shavad   
		CurECardInfo.DeviceID  = DeviceID;
    if (CurECardInfo.OPIndex > 13) 
			CurECardInfo.OPIndex = 0;
    if (Type == 19)
				Type = 2;
		
		GiftCredit  = HediyeBuf[0];   // NikKhu
		GiftCredit += HediyeBuf[1] << 8;
		GiftCredit += HediyeBuf[2] << 16;
		GiftCredit += HediyeBuf[3] << 24;
		
    switch (Type & 0x0F) {
      case Increment: {
        TempEtebar = CurECardInfo.Etebar + Payment;
        if (TempEtebar > MaxEtebar)
          return S_OutOfRange;
        CurECardInfo.Etebar += Payment;
        break;
			}

      case Decrement: {
				TempEtebar = ((CurECardInfo.Etebar + (useGift == 1 ? GiftCredit : 0))) - Payment;
				*PreEtebar = CurECardInfo.Etebar;

        if (TempEtebar < MinEtebar)
          return S_OutOfRange;
        if (TempEtebar >= 0) {
          if (TempEtebar > MaxEtebar)
            return S_OutOfRange;
				}
				else
					return S_OutOfRange;

				#define FIRST_CREDIT
				//#define FIRST_GIFT If useGift == 1
				
				#ifdef FIRST_GIFT
					if (Payment > GiftCredit) {
						CurECardInfo.Etebar -= (Payment - GiftCredit);
						GiftCredit = 0;					
					}
					else
						GiftCredit -= Payment;
				#endif
				
				#ifdef FIRST_CREDIT
					if ((Payment > CurECardInfo.Etebar) && (useGift)) {
						GiftCredit -= (Payment - CurECardInfo.Etebar);
						CurECardInfo.Etebar = 0;
					}
					else
						CurECardInfo.Etebar -= Payment;
				#endif

				*Etebar = CurECardInfo.Etebar;
        break;
			}

      default:
        return S_InvalidType;
    }
		
		HediyeBuf[0] =  GiftCredit 				& 0xFF;
		HediyeBuf[1] = (GiftCredit >> 8)  & 0xFF;
		HediyeBuf[2] = (GiftCredit >> 16) & 0xFF;
		HediyeBuf[3] = (GiftCredit >> 24) & 0xFF;
		HediyeBuf[4] = 0xAB;
		HediyeBuf[5] = 0xCD;
				
    Old_PrepairECardBuffer();
  }

	
	//Perform transaction
  SelectTimeout = 0;
  while (SelectTimeout < 140) {  // 10 ali ghablan 20 budeh
		//if (SelectTimeout >= 140) 
			//return S_TimeOut;

		if (ISO14443_SingleTagSelect(snr))
			if (ISO14443_SingleTagSelect(snr)) { 
				SelectTimeout++; 
				continue; 
			}

		if ((snr[0] == 0) && 
			  (snr[1] == 0) && 
				(snr[2] == 0) && 
				(snr[3] == 0)) { 
			SelectTimeout++; 
			continue; 
		}

		for (i=0; i<4; i++)
			if (CurECardInfo.CardSerialNumber[i] != snr[i])
				break;
		if (i < 4) {
			if (CardIsSuspend == 0)
				return S_InvalidSerial;
			CardIsSuspend = S_InvalidSerial;
			break;
		}

    SelectTimeout++;
    Res = Old_WriteTransaction();

		if (Res > 0) {
			CardIsSuspend = 1;
			continue;
		}
    CardIsSuspend = 0;
    break;
  }//while

	if (*CardID == 0)
		CardIsSuspend = 1;
	
  *Etebar = CurECardInfo.Etebar;
  //*PreEtebar=CurECardInfo.PreEtebar;
	
	//******************************************************
	/******CardIsSuspend = *LastDevice == 1 ? 0 : 1;*******/
	//******************************************************

  if (CardIsSuspend) {
		for (i=0; i<OLD_MAXSUSPENDCARDS; i++) {
			if (Old_SuspendECardsInfo[i].CardID == 0)
				break;
		}
    if (i >= OLD_MAXSUSPENDCARDS)
      i = 0;

		if (Res <= 2)
			Old_SuspendECardsInfo[i] = CurECardInfo;
    if (CardIsSuspend == S_InvalidSerial)
      return S_InvalidSerial;
    else
      return S_CardSuspend + (Res <= 2 ? 0 : (Type == Increment ? 101 : 100));
  }

	//HNA
  if (SelectTimeout >= 140)
    return (S_TimeOut2 + CardIsSuspend);

  for (j=0; j<OLD_MAXSUSPENDCARDS; j++)
    if (CurECardInfo.CardID == Old_SuspendECardsInfo[j].CardID)
      EmptyECardInfo(&Old_SuspendECardsInfo[j]);

  //EmptyECardInfo(&Old_SuspendECardsInfo[CardKharaabShodehDarSuspend]);  //ali

  return S_Success;
}
//=======================================================
void Init_CardInfoBank(void) {
	unsigned char Counter;
 for (Counter=0; Counter<1; Counter++) {} 
}
//===================================================
unsigned char GetCardHedie(signed int *hedie) {
	unsigned char snr[8];
	unsigned char buf[32];
	signed long int TempEtebar=0;
	*hedie=0;
	if (ISO14443_SingleTagSelect(snr)) 
    if (ISO14443_SingleTagSelect(snr)) 
      return(S_TimeOut);
  if (ISO14443_LoginE2(4,0x39)) return(10);
	if (ISO14443_ReadBlock(18, buf)) return(20);  //Hediye
	if ((buf[4]==0xAB) && (buf[5]==0xCD)) // ----| ---> edited
	{
		 TempEtebar=buf[3];  TempEtebar<<=8; 
		 TempEtebar+=buf[2]; TempEtebar<<=8; 
		 TempEtebar+=buf[1]; TempEtebar<<=8; 
		 TempEtebar+=buf[0]; 
	} else {TempEtebar = 0;}           //-----| ---> edited
	if ((buf[0]==0xFF) && (buf[1]==0xFF) && (buf[2]==0xFF) && (buf[3]==0xFF)) //-----|
			 TempEtebar=0; 																									//-----|--> edited
	if (TempEtebar >= 16776215) TempEtebar=0;													  // edited 
	 *hedie=TempEtebar;
	return 0;
}
//==============================================================================
unsigned char GetCardEtebar1(signed int *Etebar,signed int *HEtebar) {
	signed long int Payment, PreEtebar;
	unsigned char BCC, i, OP;
	unsigned char snr[8];
	unsigned char buf[32];
	unsigned char IsOldFormat;
	signed long int Value;
	signed long int TempEtebar=0;

	*Etebar = 0;
	*HEtebar = 0;
	switch(CoderType) {
		case ctTaxi:
			CreditSector=5;
			CreditBlock=21;
			CreditLogin=0x37;
			SOLUTION=0x20; 
			break;
		
		case ctSABA:
			CreditSector=4;
			CreditBlock=16;
			CreditLogin=0x39;
			SOLUTION=0; 
			break;
		
		case ctParyaft:
			CreditSector=5;
			CreditBlock=21;
			CreditLogin=0x37;
			SOLUTION=0x40; 
			break;
		
		default:  
			CreditSector = 5;
			CreditBlock  = 20;
			CreditLogin  = 0x37;
			SOLUTION     = 0; 
			break;
	}

  if (ISO14443_SingleTagSelect(snr)) 
    if (ISO14443_SingleTagSelect(snr)) 
      return S_TimeOut;
	
  for (i=0; i<4; i++)
    CurECardInfo.CardSerialNumber[i]=snr[i];
   
  if (ISO14443_LoginE2(0, 0x10)) 
    return(S_TimeOut);

	if (ISO14443_ReadBlock(1, buf)) 
		return S_TimeOut; 

	CurECardInfo.UC			 = buf[1]; CurECardInfo.UC <<= 8;
	CurECardInfo.UC			+= buf[0];
	CurECardInfo.CardID  = buf[5]; CurECardInfo.CardID <<= 8;
	CurECardInfo.CardID += buf[4]; CurECardInfo.CardID <<= 8;
	CurECardInfo.CardID += buf[3]; CurECardInfo.CardID <<= 8;
	CurECardInfo.CardID += buf[2];

	if ((CurECardInfo.UC & 0x8000) == 0x8000) {
		CurECardInfo.UC = CurECardInfo.UC & 0x7FFF;
		CurECardInfo.UC = CurECardInfo.UC ^ 0x6A29;
		CurECardInfo.CardID = CurECardInfo.CardID ^ 0xB2A6;
	}
	CardIDThatProcessed = CurECardInfo.CardID;
  
  if (ISO14443_LoginE2(CreditSector, CreditLogin)) 
		return 10;
	
	if (CoderType == ctSABA) {
		 if (ISO14443_ReadBlock(18, buf)) 
			 return 20;  //Hediye
			
		 if ((buf[4]==0xAB) && (buf[5]==0xCD)) // ----| ---> edited
		 {
			 TempEtebar=buf[3];  TempEtebar<<=8; 
			 TempEtebar+=buf[2]; TempEtebar<<=8; 
			 TempEtebar+=buf[1]; TempEtebar<<=8; 
			 TempEtebar+=buf[0]; 
		 }
		 else 
			TempEtebar = 0;           //-----| ---> edited
		 if ((buf[0]==0xFF) && (buf[1]==0xFF) && (buf[2]==0xFF) && (buf[3]==0xFF)) //-----|
				 TempEtebar=0; 																									//-----|--> edited
		 if (TempEtebar == 16776215) TempEtebar=0;													  // edited 
		 *HEtebar=TempEtebar;
		}
// B8 0B 00 00 47 F4 FF FF B8 0B 00 00 00 FF 00 FF
// 00 00 00 00 AB CD 00 00 00 00 00 00 00 00 00 00 Hediye													
	
  if (ISO14443_ReadBlock(CreditBlock, buf)) 
		return 20;  
  for (i=0; i<16; i++)
    CurECardInfo.EtebarBlock[i] = buf[i];
    
  //DecryptBlock(CurECardInfo.CardSerialNumber, CurECardInfo.EtebarBlock);
    
  IsOldFormat = 1;  
  for (i=0; i<4; i++) 
		if (buf[i] != buf[i + 8]) 
			IsOldFormat = 0;  
  for (i=0; i<4; i++) 
		if (buf[i + 4] != (buf[i] ^ 0xFF)) 
			IsOldFormat = 0;
  if ((buf[12] != 0) || (buf[13] != 0xFF) || (buf[14] != 0) || (buf[15] != 0xFF)) 
		IsOldFormat = 0;
	
	if (IsOldFormat) {                                      
		CurECardInfo.Etebar  = buf[3]; CurECardInfo.Etebar <<= 8; 
		CurECardInfo.Etebar += buf[2]; CurECardInfo.Etebar <<= 8; 
		CurECardInfo.Etebar += buf[1]; CurECardInfo.Etebar <<= 8; 
		CurECardInfo.Etebar += buf[0]; 

		if (ISO14443_LoginE2(6, 0x14)) 
			return 12;   
		if (ISO14443_ReadValueBlock(24, &Value)) 
			return 13;
		CurECardInfo.OPIndex = Value;		
		*Etebar = CurECardInfo.Etebar;
			
		return 0;
	}

  BCC = 0x4B;
  for (i=0; i<15; i++)
    BCC ^= CurECardInfo.EtebarBlock[i];
  if (BCC != CurECardInfo.EtebarBlock[15])
    return 3;
  
  CurECardInfo.Etebar  = CurECardInfo.EtebarBlock[3]; CurECardInfo.Etebar <<= 8;    
  CurECardInfo.Etebar += CurECardInfo.EtebarBlock[2]; CurECardInfo.Etebar <<= 8;    
  CurECardInfo.Etebar += CurECardInfo.EtebarBlock[1]; CurECardInfo.Etebar <<= 8;    
  CurECardInfo.Etebar += CurECardInfo.EtebarBlock[0];

  Payment  = CurECardInfo.EtebarBlock[9]; Payment <<= 8;
  Payment += CurECardInfo.EtebarBlock[8];

  PreEtebar  = CurECardInfo.EtebarBlock[7]; PreEtebar <<= 8;    
  PreEtebar += CurECardInfo.EtebarBlock[6]; PreEtebar <<= 8;    
  PreEtebar += CurECardInfo.EtebarBlock[5]; PreEtebar <<= 8;    
  PreEtebar += CurECardInfo.EtebarBlock[4];
  
  OP = CurECardInfo.EtebarBlock[10];
  switch (OP) {
    
		case Increment://1
      TempEtebar = PreEtebar+Payment;
      if (TempEtebar != CurECardInfo.Etebar)
        return 5;
      break;
    
		case Decrement://2
      TempEtebar = PreEtebar - Payment;
      if (TempEtebar != CurECardInfo.Etebar)
        return 6;
      break;
			
    default:
      return 7;  
  }

  CurECardInfo.OPIndex = CurECardInfo.EtebarBlock[11] & 0x1F;    
  *Etebar = CurECardInfo.Etebar;
  return 0;
}
//****************************************************************
unsigned char GetOpBlockByIndex2(unsigned char idx) {
  switch (idx) {
    case  0: return 25;
    case  1: return 26;
    case  2: return 28;
    case  3: return 29;
    case  4: return 30;
    case  5: return 32;
    case  6: return 33;
    case  7: return 34;
    case  8: return 36;
    case  9: return 37;
    case 10: return 38;
    case 11: return 40;
    case 12: return 41;
    case 13: return 42;
		default: return  0;
  }
}
//==============================================================================
void ReadCardTable(unsigned char Row) {
	unsigned char 
			 snr[10],
			Temp[20],
		Buffer[16],
			 buf[32],
		key,
		RetValue,
		Head_Loc,		
		idx,		
		Block,
		locCounter,
		BCC 						= 0, 
		Counter 				= 0,
		Card_InfoNumber = 0,
		loc 						= 0,
		OpSize 					= 9,
		Counter2 				= 0;
	
	signed long int 
		Credit,
		Value,
		H,
		HediyeCredit,
		OldLoc,
		Li,
		CardID;
	
	unsigned short 
		Len, 
		i,
		UC,
		Timeout,
		CreditLogin2 = 0x35;	
	
	unsigned int CurrentCardCredit = 0;	
	char str[120];	

 Init_CardInfoBank();
 send("{\"Receive\":{\"Status\":26},\"C\":{\"Type\":\"ReadCardTable\"}}\n");	

 Li = HAL_GetTick();
 
  while (1) {
		
		if ((HAL_GetTick() - Li) > 9000) {
				send("{\"C\":{\"Status\":1}}\n");
				InfoCounter = 1;
       return;
       break;
      }
		WDTR
		#if  defined finger
			rxBufferHeadPos = 0;
		#else			
			if (CDCReceiveLen)
		#endif
		if ((fromAndroid[0] == 'C') || (fromAndroid[0]=='c')) {
			send("{\"Receive\":{\"Status\":26},\"C\":{\"Type\":\"Cansel\"}}\n"); 
			CDCReceiveLen = 0;
			return;
		}
		if (ISO14443_SingleTagSelect(snr))
      if (ISO14443_SingleTagSelect(snr))
				continue;
 
		if (ISO14443_LoginE2(0, 0x10)) 
			continue;
		if (ISO14443_ReadBlock(1, buf)) 
			continue;

		UC  		=  buf[0];
		UC     += (buf[1] << 8);
		CardID  =  buf[2];
		CardID += (buf[3] << 8);
		CardID += (buf[4] << 16);
		CardID += (buf[5] << 24);
		if ((UC & 0x8000) == 0x8000) {
			UC = UC & 0x7FFF;
			UC = UC ^ 0x6A29;
			CardID = CardID ^ 0xB2A6;
		}

		__HAL_IWDG_RELOAD_COUNTER(&hiwdg);
		if (CoderType == ctNone) {
			//Read card operations
			if (ISO14443_LoginE2(6, 0x14))  
				continue;
			if (ISO14443_ReadValueBlock(24, &H)) 
				continue;

			//Read main credit
			if (ISO14443_LoginE2(5, 0x37))            
				continue;
			if (ISO14443_ReadValueBlock(20, &Credit)) 
				continue;
			loc = H;
		}
		else if (CoderType == ctSABA) {
			if (ISO14443_LoginE2(6, 0x14))         
				continue;
			if (ISO14443_ReadValueBlock(24, &H))
				continue;
			loc = H;
		}
		
		__HAL_IWDG_RELOAD_COUNTER(&hiwdg);
		loc = 0;
		RetValue = GetCardEtebar1(&Credit, &HediyeCredit);
		if (RetValue == S_TimeOut)   
			RetValue = GetCardEtebar1(&Credit, &HediyeCredit);	

		if ((RetValue == 0) || (RetValue == 90))
			CurrentCardCredit = Credit; 
		else
			continue;
		
		break;
	} //End of while
  
	__HAL_IWDG_RELOAD_COUNTER(&hiwdg);
  if (CoderType == ctSABA)   
		loc = H;  

  if (loc)  
		loc--;
  else     
		loc = 13;
  Head_Loc = loc;  
  Counter = 0;
  Timeout = 0;

  while (1) {
		WDTR
		++Timeout;
		if (Timeout >= 50)    
			break;  
		if (ISO14443_SingleTagSelect(snr)) 
			if (ISO14443_SingleTagSelect(snr))  
				continue; 
		__HAL_IWDG_RELOAD_COUNTER(&hiwdg);
		Block = GetOpBlockByIndex2(loc);
			
		//.............................................................................
		if (ISO14443_LoginE2(Block / 4, 0x14)) {
			if (ISO14443_SingleTagSelect(snr))
				if (ISO14443_SingleTagSelect(snr))     
					continue;
			 if (ISO14443_LoginE2(Block / 4, 0x14)) 
				 continue;
		}
		//.............................................................................
		if (ISO14443_ReadBlock(Block, buf)) {
			if (ISO14443_LoginE2(Block / 4, 0x14)) {
				if (ISO14443_SingleTagSelect(snr)) {
					if (ISO14443_SingleTagSelect(snr))     
						continue;
				}
				if (ISO14443_LoginE2(Block / 4, 0x14)) 
					continue;
		 }
		 if (ISO14443_ReadBlock(Block, buf))  
			 continue;
		}
		//............................................................................. 
		Timeout = 0;

		for (i=0; i<16; i++)
				Buffer[i] = buf[i];  
		BCC = 0;
		for (i=0; i<15; i++)
			BCC ^= Buffer[i];
		
		if (BCC != Buffer[15])
			for (i=0; i<16; i++)
			 Buffer[i] = 0;
		
		for (i=0; i<15; i++)
			if (Buffer[i] != 0xFF)
				break;
			
		if (i >= 15)
			for (i=0; i<16; i++)
				Buffer[i] = 0;
		//..................................................................................

		sprintf(globalBuff, "Row=%d*", Counter + 1);
		send(globalBuff);
		HAL_Delay(1);
		 
		sprintf(globalBuff, "OP=%d*", Buffer[5]);
		send(globalBuff);
		HAL_Delay(1);
		WDTR 
				
		tempSend.temp  = Buffer[4]; tempSend.temp <<= 8;
		tempSend.temp += Buffer[3]; tempSend.temp <<= 8;
		tempSend.temp += Buffer[2]; 
		 
		sprintf(globalBuff,"Etebar=%u*", tempSend.temp);		
		send(globalBuff);
		HAL_Delay(1);

		if ((Buffer[6] >> 4) < 9) 	
			sprintf(globalBuff, "Year=%d*", (Buffer[6] >> 4) + 1400);
		else 
			sprintf(globalBuff,"Year=%d*", (Buffer[6] >> 4) + 86 + 1300);

		send(globalBuff);
		HAL_Delay(1);

		//Card_Info[Counter].Month=Buffer[6]&0x0F;

		sprintf(globalBuff, "Month=%d*", Buffer[6] & 0x0F);
		send(globalBuff);
		HAL_Delay(1);
		WDTR

		sprintf(globalBuff, "Day=%d*", Buffer[7]);
		send(globalBuff);
		HAL_Delay(1);

		tempSend.temp  = Buffer[10]; tempSend.temp <<= 8;
		tempSend.temp += Buffer[9];  tempSend.temp <<= 8;
		tempSend.temp += Buffer[8]; 

		sprintf(globalBuff, "RemEtebar=%u*", tempSend.temp);
		send(globalBuff);
		HAL_Delay(1);

		tempSend.temp  = Buffer[12]; tempSend.temp <<= 8;
		tempSend.temp += Buffer[11]; 

		sprintf(globalBuff, "DID=%u*", tempSend.temp);
		send(globalBuff);
		HAL_Delay(1);

		sprintf(globalBuff, "Hour=%d*", Buffer[13]);
		send(globalBuff);
		HAL_Delay(1);
		WDTR

		sprintf(globalBuff, "Min=%d*", Buffer[14]);
		send(globalBuff);
		HAL_Delay(1);
		
		sprintf(globalBuff, "Sec=%d,\n", Buffer[0]);
		send(globalBuff);
		HAL_Delay(1);

		//Card_Info[Counter].Price=Card_Info[Counter].RemEtebar-Card_Info[Counter].Etebar;

		if (++Counter >= Row)   
			break; 
		if (loc) 
			loc--;
		else    
			loc = 13;
		WDTR
  }//end of while

  if (Timeout < 50) {
		HAL_Delay(10);
		WDTR
  }
	InfoCounter = 1;
	return;
}


//=======================================
void IntToBytes(unsigned int i, unsigned char *buf) {
  buf[0]=i & 0xFF;
  buf[1]=(i>>8) & 0xFF;
  buf[2]=(i>>16) & 0xFF;
  buf[3]=(i>>24) & 0xFF;
}

//==============================================================================
void BytesToInt(unsigned int *i, unsigned char *buf) {
  *i  = buf[3]; *i <<= 8;
  *i += buf[2]; *i <<= 8;
  *i += buf[1]; *i <<= 8;
  *i += buf[0];
}
//===============================================
unsigned char IncrementCreditToCard(unsigned long IDD, signed long int Payment,unsigned int HPayment, unsigned char rqid) {
	unsigned char Status=200,snr[10];
	unsigned int HEtebar, preHetebar;    
	unsigned char TicketBuffer[32],TicketBuffer2[32],LastOP,OPIndex; 
	unsigned char SubGroup;
	unsigned char BCC;    
	unsigned int UC, Grouh;
	unsigned long int ID, CardID, Li, aLive;
	signed long int Etebar=0, PreEtebar=0; 
	int LastDevice;
	unsigned char buf[100];
	unsigned char idx = 0;
	unsigned int HNPayment=0;
	
	preHetebar = HNPayment;
	if (zeroEtebarFlag == 1) {
		HediyeBuf[0] =  HNPayment 			 & 0xFF;
		HediyeBuf[1] = (HNPayment >> 8)  & 0xFF;
		HediyeBuf[2] = (HNPayment >> 16) & 0xFF;	
		HediyeBuf[3] = (HNPayment >> 24) & 0xFF;	
		HediyeBuf[4] = 0xAB;
		HediyeBuf[5] = 0xCD;
	}			 

  aLive = Li = HAL_GetTick();	
  Status = 0;
  CDCReceiveLen = 0;
  while (1) {
		WDTR
		if (CDCReceiveLen) {
			if ((fromAndroid[0] =='c') || (fromAndroid[0] =='C')) {
				send("{\"Receive\":{\"Status\":26},\"C\":{\"Type\":\"Cansel\"}}\n"); 
				#ifdef MaxReciveBuf
				rxBufferHeadPos = 0;
				#endif
				CDCReceiveLen = 0;
				return 0;
			}	
			else  {
				#ifdef MaxReciveBuf
					rxBufferHeadPos = 0;
				#endif
				CDCReceiveLen = 0; 
			}
		}
		if ((HAL_GetTick() - Li) > 20000) {
			 Status = 1;
			 send("{\"C\":{\"Status\":1}}\n");
       return 1;
		}
		
		if ((HAL_GetTick() - aLive) >= 3000) {
			//send(ALIVE_MESSAGE);
			aLive = HAL_GetTick();
		}
		
		if (ISO14443_SingleTagSelect(snr))
			if (ISO14443_SingleTagSelect(snr))
				continue;
		
		#if (deviceType != BusEtebar)
			if (ISO14443_LoginE2(0, 0x10))
				continue;
			if (ISO14443_ReadBlock(1, TicketBuffer)) 
				continue; 
			//if (TicketBuffer[6] != 0xab) continue; 
			
			UC  = TicketBuffer[0];
			UC += (TicketBuffer[1] << 8);
			CardID  = TicketBuffer[2];
			CardID += (TicketBuffer[3] << 8);
			CardID += (TicketBuffer[4] << 16);
			CardID += (TicketBuffer[5] << 24);
			if ((UC & 0x8000) == 0x8000) {
				 UC = UC & 0x7FFF;
				 UC = UC ^ 0x6A29;
				 CardID = CardID ^ 0xB2A6;
			}
//***************Grouh  00 00 00 00 18 08 0D 0C 29 2E 01 00 00 01 00 16
			if (ISO14443_LoginE2(5, 0x37)) {
				if (ISO14443_SingleTagSelect(snr))
					if (ISO14443_SingleTagSelect(snr))
						continue; 
				if (ISO14443_LoginE2(5,0x37))  
					continue; 
			}
			if (ISO14443_ReadBlock(22, TicketBuffer)) {
				if (ISO14443_LoginE2(5, 0x37)) {
					if (ISO14443_SingleTagSelect(snr))
						if (ISO14443_SingleTagSelect(snr))
							continue; 
					if (ISO14443_LoginE2(5,0x37))  
						continue; 
				}
				if (ISO14443_ReadBlock(22,TicketBuffer)) 
					continue; 
			}
			Grouh  = TicketBuffer[1];  Grouh <<= 8;
			Grouh += TicketBuffer[0];
				
			/*if (Grouh != 0) {
				send("{\"C\":{\"Status\":7}}\n"); //Invalid Group flag data
				return 1;
			}*/
//*****************	Get Hediye																		
		if (ISO14443_LoginE2(4, 0x39)) {
			if (ISO14443_SingleTagSelect(snr))
				if (ISO14443_SingleTagSelect(snr))
					continue;
			if (ISO14443_LoginE2(4, 0x39))  
				continue; 
		}
	
		if (CoderType == ctSABA) {  //Get Hediye
			//HEtebar=0xFFFFFF;  edited ---> HEtebar=0x0;
			HEtebar = 0x0;
			if (ISO14443_ReadBlock(18, TicketBuffer)) 
				continue;  //Hediye
			if ((TicketBuffer[4] == 0xAB) && (TicketBuffer[5] == 0xCD)) {
				HEtebar  = TicketBuffer[3]; HEtebar <<= 8; 
				HEtebar += TicketBuffer[2]; HEtebar <<= 8; 	
				HEtebar += TicketBuffer[1]; HEtebar <<= 8; 
				HEtebar += TicketBuffer[0]; 
				preHetebar = HEtebar;
			}
			if ((TicketBuffer[0] == 0xFF) && (TicketBuffer[1] == 0xFF) && 
					(TicketBuffer[2] == 0xFF) && (TicketBuffer[3] == 0xFF)) //-----|
				 HEtebar=0;
			HNPayment = HPayment + HEtebar;
			 
			//if (ResetCardFlag==1) {FHediyeCredit=HEtebar; HNPayment=0;}
		}
//********************SubGroup	00 00 00 00 FF FF FF FF 00 00 00 00 00 FF 00 FF																	
		if (ISO14443_ReadBlock(17, TicketBuffer)) {
			if (ISO14443_LoginE2(4,0x39)) {
				if (ISO14443_SingleTagSelect(snr))
					if (ISO14443_SingleTagSelect(snr))
						continue;
				if (ISO14443_LoginE2(4,0x39))  
					continue; 
			}
			if (ISO14443_ReadBlock(17,TicketBuffer)) 
				continue; 
		}
		BCC = 0;
		for (unsigned char i=0; i<15; i++)              
			BCC ^= TicketBuffer[i];				

		if (BCC == TicketBuffer[15]) {
			SubGroup  = TicketBuffer[0];
			SubGroup += (TicketBuffer[1] << 8);
			if (SubGroup == 22) {
				HNPayment = 0; 
				HPayment  = 0;
			}
		}
	#endif 
//*****************************************  

	if (ResetCardFlag == 0) {
		HediyeBuf[0] = HNPayment 				 & 0xFF;
		HediyeBuf[1] = (HNPayment >> 8)  & 0xFF;
		HediyeBuf[2] = (HNPayment >> 16) & 0xFF;	
		HediyeBuf[3] = (HNPayment >> 24) & 0xFF;	
		HediyeBuf[4] = 0xAB;	
		HediyeBuf[5] = 0xCD;	
	}
	else {
		HNPayment    = 0;
		HediyeBuf[0] = 	HNPayment 			 & 0xFF;
		HediyeBuf[1] = (HNPayment >> 8)  & 0xFF;
		HediyeBuf[2] = (HNPayment >> 16) & 0xFF;	
		HediyeBuf[3] = (HNPayment >> 24) & 0xFF;	
		HediyeBuf[4] = 0xAB;	
		HediyeBuf[5] = 0xCD;	
	}

  ID = IDD;
	#if (deviceType == BCU)
		UC = Config.UC;
	#else
		if (flagRc522)
			UC = configUc;
		else 
			UC = UC;
	#endif
  //MinCharge=0;
	
  #if defined (ExecTransactionBCU) || defined(ExecTransactionBD) 
		ResetCardFlag = 0;
	#endif
		
	LastDevice = rqid;
	Status = Old_ExecTransaction(ResetCardFlag + 1, &ID, &UC, Payment, MINEtebar, 
		MAXEtebar, deviceID, &Etebar, &PreEtebar, &LastDevice, &LastOP, &OPIndex);
	
  /*if ((Status==1) || (Status==9)) {
		Status = Old_ExecTransaction(ResetCardFlag + 1, &ID, &UC, Payment, MINEtebar, MAXEtebar, deviceID, &Etebar, &PreEtebar,&LastDevice,&LastOP,&OPIndex);
		if ((Status==1) || (Status==9)) {
			 Status = Old_ExecTransaction(ResetCardFlag + 1, &ID, &UC, Payment, MINEtebar, MAXEtebar, deviceID, &Etebar, &PreEtebar,&LastDevice,&LastOP,&OPIndex);
		 }
  }
  if ((Status == 1) || (Status == S_CardSuspend) || (Status == 9)) {
    Status=Old_ExecTransaction(ResetCardFlag+1, &ID, &UC, Payment, MINEtebar, MAXEtebar, deviceID, &Etebar, &PreEtebar,&LastDevice,&LastOP,&OPIndex);
		if ((Status == 1) || (Status == S_CardSuspend)|| (Status == 9))  //Is Old Format  
        Status=Old_ExecTransaction(ResetCardFlag+1, &ID, &UC, Payment, MINEtebar, MAXEtebar, deviceID, &Etebar, &PreEtebar,&LastDevice,&LastOP,&OPIndex);
  }*/
	#if (deviceType == BCU) || (deviceType == BusEtebar)
		if (flagRc522) {
			sprintf(buffTransaction,"{\"C\":{\"Status\":%d,\"UC\":%u,\"ID\":%u,\"Etebar\":%u,\"PreEtebar\":%u,\"Price\":%u}}\n",Status,UC,ID,Etebar,PreEtebar,Payment);
			send(buffTransaction);
			buf[idx++] = deviceType;
			
			buf[idx++] = ID & 0xFF;
			buf[idx++] = ( ID >> 8) & 0xFF;
			buf[idx++] = ( ID >> 16) & 0xFF;
			buf[idx++] = ( ID >> 24) & 0xFF;
			
			buf[idx++] =  UC & 0xFF;
			buf[idx++] = ( UC >> 8) & 0xFF;
			buf[idx++] = ( UC >> 16) & 0xFF;
			buf[idx++] = ( UC >> 24) & 0xFF;
			buf[idx++] = Status;
			
			buf[idx++] = CurECardInfo.OP; // type
			
			buf[idx++] =  Etebar & 0xFF;
			buf[idx++] = ( Etebar >> 8) & 0xFF;
			buf[idx++] = ( Etebar >> 16) & 0xFF;
			buf[idx++] = ( Etebar >> 24) & 0xFF;

			buf[idx++] =  PreEtebar & 0xFF;
			buf[idx++] = ( PreEtebar >> 8) & 0xFF;
			buf[idx++] = ( PreEtebar >> 16) & 0xFF;
			buf[idx++] = ( PreEtebar >> 24) & 0xFF;

			buf[idx++] =  Payment & 0xFF;
			buf[idx++] = ( Payment >> 8) & 0xFF;
			buf[idx++] = ( Payment >> 16) & 0xFF;
			buf[idx++] = ( Payment >> 24) & 0xFF;
			
			buf[idx++] = Year;
			buf[idx++] = Month;
			buf[idx++] = Day;
			buf[idx++] = Hour;
			buf[idx++] = Min;
			if (	SPI_Flash_ReadID() == 0XEF16)
			{
				SaveTransactionRecord(buf,idx);
				SaveRingDetail();
			}		
			memset(buf,'\0',strlen(buf));	
	    return 1;
		}			
	#else
	if (flagRc522) {		
		sprintf(sendingResult, "{\"C\":{\"Status\":%d,\"UC\":%u,\"ID\":%u,\"" \
				"Etebar\":%d,\"Hedie\":%d,\"PreHedie\":%d,\"PreEtebar\":%d,\"Price\"" \
				":%d, \"Gt\":%d, \"EEM\":%u}}\n", 
				Status, configUc, ID, Etebar, HNPayment, HEtebar, PreEtebar, Payment, 
				HPayment, requestID);
		send(sendingResult);		
		preRequestID = requestID;
		
		buf[idx++] = deviceType;
		
		buf[idx++] = ID & 0xFF;
		buf[idx++] = ( ID >> 8) & 0xFF;
		buf[idx++] = ( ID >> 16) & 0xFF;
		buf[idx++] = ( ID >> 24) & 0xFF;
		
		buf[idx++] =  UC & 0xFF;
		buf[idx++] = ( UC >> 8) & 0xFF;
		buf[idx++] = ( UC >> 16) & 0xFF;
		buf[idx++] = ( UC >> 24) & 0xFF;
		buf[idx++] = Status;
		
		buf[idx++] = CurECardInfo.OP; // type
		
		buf[idx++] =  Etebar & 0xFF;
		buf[idx++] = ( Etebar >> 8) & 0xFF;
		buf[idx++] = ( Etebar >> 16) & 0xFF;
		buf[idx++] = ( Etebar >> 24) & 0xFF;

		buf[idx++] =  PreEtebar & 0xFF;
		buf[idx++] = ( PreEtebar >> 8) & 0xFF;
		buf[idx++] = ( PreEtebar >> 16) & 0xFF;
		buf[idx++] = ( PreEtebar >> 24) & 0xFF;

		buf[idx++] =  HPayment & 0xFF;
		buf[idx++] = ( HPayment >> 8) & 0xFF;
		buf[idx++] = ( HPayment >> 16) & 0xFF;
		buf[idx++] = ( HPayment >> 24) & 0xFF;

		buf[idx++] =  Payment & 0xFF;
		buf[idx++] = ( Payment >> 8) & 0xFF;
		buf[idx++] = ( Payment >> 16) & 0xFF;
		buf[idx++] = ( Payment >> 24) & 0xFF;
		
		buf[idx++] =  HNPayment & 0xFF;
		buf[idx++] = ( HNPayment >> 8) & 0xFF;
		buf[idx++] = ( HNPayment >> 16) & 0xFF;
		buf[idx++] = ( HNPayment >> 24) & 0xFF;
		
		buf[idx++] = Year;
		buf[idx++] = Month;
		buf[idx++] = Day;
		buf[idx++] = Hour;
		buf[idx++] = Min;
		if (SPI_Flash_ReadID() == 0XEF16) {		
			SaveTransactionRecord(buf,idx);
			SaveRingDetail();
		}
		return 1;
	}
	#endif
	 break;   
  }
	return Status;
}

//==========================================================
char Write_GroupOfCard(char SabaGroup) {
	unsigned char snr[10],Status=0;
	unsigned int status=0;
	unsigned char TicketBuffer[32]={0},TicketBuffer2[32],BCC=0,RetValue=0;
	unsigned char Percent;
	unsigned int UC, Grouh;
	unsigned long int ID,CardID,Li;
	long int  Credit=0;
	signed long int Etebar=0, PreEtebar=0,HediyeCredit; 


#define INVALIDDATE send("{\"C\":{\"Status\":2}}\n");
#define TIMEOUTJSON send("{\"C\":{\"Status\":1}}\n");
#define CANSELLJSON send("{\"Receive\":{\"Status\":26},\"C\":{\"Type\":\"Cansel\"}}\n");
//	subgp == 0  { 68 subgp Y M D H M S crc} 
//	subgp == 22 { 68 subgp darsad Ye Me De Hs Ms He Me Y M D H M S crc } 
//	subgp == 23 { 68 subgp Y M D H M S crc}
//	subgp == 24 { 68 subgp Ye Me De Hs Ms He Me Y M D H M S crc } 
//	subgp == 25 { 68 subgp Ye Me De Hs Ms He Me Y M D H M S crc }


	
  CDCReceiveLen = 0;
  if (SabaGroup==0)   //addi 
	{
		TicketBuffer[2]=0;
	}		
  else if (SabaGroup==22)
  {
		Percent=TicketBuffer[2]=fromAndroid[2]; // darsad takhfif
    if (TicketBuffer[2]>=100)  return(0);
    if (TicketBuffer[2]<0)   return(0);		
			// takhfif ta tarikhe 
	   TicketBuffer[3]=fromAndroid[3];
	   TicketBuffer[4]=fromAndroid[4];
	   TicketBuffer[5]=fromAndroid[5];
	   WDTR
	   if ((TicketBuffer[3]<0) || (TicketBuffer[4]<1) || (TicketBuffer[4]>12) || (TicketBuffer[5]<1) || (TicketBuffer[5]>31))  // ==== edited ==== //
	   {
				 INVALIDDATE
				 return(1);
	   }
				// az saate 
	   TicketBuffer[6]=fromAndroid[6];
	   TicketBuffer[7]=fromAndroid[7];
		
	   if ((TicketBuffer[6]>23) || (TicketBuffer[7]>59))
	   {
				INVALIDDATE
		    return(1);
	   }	
		
			// ta saate 
	   TicketBuffer[8]=fromAndroid[8];
	   TicketBuffer[9]=fromAndroid[9];

			
	   if ((TicketBuffer[8]>23) || (TicketBuffer[9]>59))
	   {
				INVALIDDATE
		    return(1);
	   }
  }
  else if (SabaGroup==23)// tedadi 
	{
  
	}			
				
  else if (SabaGroup==24)// mehman poli 	
  {
			// mehman mablaqi ta tarikhe 
		TicketBuffer[3]=fromAndroid[2];
	  TicketBuffer[4]=fromAndroid[3];
	  TicketBuffer[5]=fromAndroid[4];
	
	   if ((TicketBuffer[3]<0) || (TicketBuffer[4]<1) || (TicketBuffer[4]>12) || (TicketBuffer[5]<1) || (TicketBuffer[5]>31))     // ==== edited ==== //
	   {
				INVALIDDATE
		    return(1);
	   }
			// az saate 
	   TicketBuffer[6]=fromAndroid[5];
	   TicketBuffer[7]=fromAndroid[6];
	   if ((TicketBuffer[6]>23) || (TicketBuffer[7]>59))
	   {
			 INVALIDDATE
		   return(1);
	   }	
			// ta saate 
	   TicketBuffer[8]= fromAndroid[7];
	   TicketBuffer[9]= fromAndroid[8];
	   Sec=0;
	   if ((TicketBuffer[8]>23) || (TicketBuffer[9]>59))
	   {
			 INVALIDDATE
		   return(1);
	   }	  
	}							
  else if (SabaGroup==25)// mehman tedadi 	
  {
			// ta tarikhe 
	   TicketBuffer[3]=fromAndroid[2];
	   TicketBuffer[4]=fromAndroid[3];
	   TicketBuffer[5]=fromAndroid[4];
	   if ((TicketBuffer[3]<0) || (TicketBuffer[4]<1) || (TicketBuffer[4]>12) || (TicketBuffer[5]<1) || (TicketBuffer[5]>31))    // ==== edited ==== //
	   {
				INVALIDDATE
		    return(1);
	   }
			// az saate 
	   TicketBuffer[6]=fromAndroid[5];
	   TicketBuffer[7]=fromAndroid[6];
	   if ((TicketBuffer[6]>23) || (TicketBuffer[7]>59))
	   {
				INVALIDDATE
		    return(1);
	   }	
			// ta saate 
	   TicketBuffer[8]=fromAndroid[7];
	   TicketBuffer[9]=fromAndroid[8];
		 if ((TicketBuffer[8]>23) || (TicketBuffer[9]>59))
		 {
				INVALIDDATE
				return(1);
		 }	  
	}					   
  
// subgp == 22 , dar sade takhfif 
// subgp == 23 , tedad bazi 
// subgp == 24 , mehman mablaqi 
// subgp == 25 , mehman tedad bazi 
// subgp == 0 , addi 						
   WDTR		  
   TicketBuffer[0]=SabaGroup%256;
   TicketBuffer[1]=SabaGroup/256;  
   BCC=0;
   for (unsigned char i=0; i<15; i++)              
     BCC^=TicketBuffer[i];
   TicketBuffer[15]=BCC; 
	 
   Li=HAL_GetTick();
   while (1)
	 {
		 WDTR 
     if (HAL_GetTick()-Li>10000) 
     {
		  	TIMEOUTJSON
			  return 1;
     }
		 if (CDCReceiveLen)
		 {
			 if ((fromAndroid[0] == 'c') || (fromAndroid[0] == 'C'))
			 {
					CANSELLJSON;	
					CDCReceiveLen = 0;
					return 1;
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
    if (ISO14443_ReadBlock(1,TicketBuffer2)) {  continue; }  
		//if (TicketBuffer2[6] != 0xAB) continue;
    UC=TicketBuffer2[0];
    UC+=(TicketBuffer2[1]<<8);
    CardID=TicketBuffer2[2];
    CardID+=(TicketBuffer2[3]<<8);
    CardID+=(TicketBuffer2[4]<<16);
    CardID+=(TicketBuffer2[5]<<24);
    if ((UC & 0x8000)==0x8000)
    {
       UC = UC & 0x7FFF;
       UC = UC ^ 0x6A29;
       CardID = CardID ^ 0xB2A6;
    }			
		WDTR
   //............................................................................................  
 	  if (ISO14443_LoginE2(4,0x39)) 
    {
      if (ISO14443_SingleTagSelect(snr))
      {
        if (ISO14443_SingleTagSelect(snr))
             continue;
      }
      if (ISO14443_LoginE2(4,0x39))  continue;
    }
    if (ISO14443_ReadBlock(17, TicketBuffer2)) 
    {
			if (ISO14443_LoginE2(4,0x39)) 
      {
        if (ISO14443_SingleTagSelect(snr))
        {
          if (ISO14443_SingleTagSelect(snr))
                 {  continue; }
        }
        if (ISO14443_LoginE2(4,0x39))  {  continue; }
      }
      if (ISO14443_ReadBlock(17,TicketBuffer2)) {  continue;}
    }
    BCC=0;
    for (unsigned char i=0; i<15; i++)              
        BCC^=TicketBuffer2[i];
     Grouh=0;
     if (BCC==TicketBuffer2[15])
     {
        Grouh=TicketBuffer2[0];
        Grouh+=(TicketBuffer2[1]<<8);
		 }
		 
		 WDTR
     HediyeBuf[0]=0;
     HediyeBuf[1]=0;
     HediyeBuf[2]=0;	
     HediyeBuf[3]=0;	
     HediyeBuf[4]=0xAB;	
     HediyeBuf[5]=0xCD;	 					
    if (ISO14443_WriteBlock(18,HediyeBuf))  continue;
    //............................................................................................  			 
    //............................................................................................  
		if (SabaGroup!=Grouh)
		{			
       RetValue=GetCardEtebar1(&Credit,&HediyeCredit);
       if (RetValue==S_TimeOut)  RetValue=GetCardEtebar1(&Credit,&HediyeCredit);
       PreEtebar = Credit;			 
       if ((RetValue!=0) && (RetValue!=90))  continue;          
       if (Credit)
       {
          DontChangeDebit=1;
          ResetCardFlag=1;
					ResetCardFlag2=1;
					if ((Grouh==23) || (Grouh==25)) ResetCardFlag2=2;
					changeGp = 1;
					// jahat kasr mojodi qarar dahid 
          IncrementCreditToCard(0,Credit,0,0); // HPayment
					changeGp = 0;
					ResetCardFlag2=0;
		      DontChangeDebit=0;
          ResetCardFlag=0;	
          RetValue=GetCardEtebar1(&Credit,&HediyeCredit);
          if (RetValue==S_TimeOut)  RetValue=GetCardEtebar1(&Credit,&HediyeCredit);
    
          if ((RetValue!=0) && (RetValue!=90))  continue;  
			    if (Credit)	                       continue;  
        }
		}  
		TicketBuffer[0]=SabaGroup%256;
		TicketBuffer[1]=SabaGroup/256;
		TicketBuffer[2]=Percent;
		BCC=0;
		WDTR
		for (unsigned char i=0; i<15; i++)              
			BCC^=TicketBuffer[i];
		TicketBuffer[15]=BCC;
		if (ISO14443_LoginE2(4,0x39))
		{
			if (ISO14443_SingleTagSelect(snr))
			{
				if (ISO14443_SingleTagSelect(snr))
					continue;
			}
			if (ISO14443_LoginE2(4,0x39))continue;
		}      
		if (ISO14443_WriteBlock(17, TicketBuffer)) 
		{
			if (ISO14443_LoginE2(4,0x39))
				{
					if (ISO14443_SingleTagSelect(snr))
						{
						if (ISO14443_SingleTagSelect(snr))
							continue;
						}
					if (ISO14443_LoginE2(4,0x39)) continue;
				}
			if (ISO14443_WriteBlock(17,TicketBuffer)) continue;
		}    
			sprintf(globalBuff,"{\"C\":{\"Status\":0,\"UC\":%u,\"ID\":%u,\"Etebar\":%u,\"PreEtebar\":%u}}\n",UC,CardID,Credit,PreEtebar);	
			send(globalBuff);					
			return(0);
	}
	return(0);
}
//*****************************************************************************
unsigned char readGroupOfCard(unsigned char *ser) {
	unsigned char buf[16], BCC, i, SubGroup, Discount;
	unsigned int tick = HAL_GetTick();
	unsigned long int etebar = 0;
	signed int Hetebar = 0; 
	while (1) {
		WDTR;
		if ((HAL_GetTick() - tick) > 1000) 
			return 255;
		if (ISO14443_SingleTagSelect(ser))
			if (ISO14443_SingleTagSelect(ser))
				continue;
		
		if (ISO14443_LoginE2(4, 0x39)) {
			if (ISO14443_SingleTagSelect(ser))
				if (ISO14443_SingleTagSelect(ser))
					continue;
			if (ISO14443_LoginE2(4, 0x39))
				continue;
		}
		
		if (!ISO14443_ReadBlock(17, buf)) {
			BCC = 0;
			for (i=0; i<15; i++)
				BCC ^= buf[i]; 

			if (BCC == buf[15]) {
				SubGroup =   buf[0];
				SubGroup += (buf[1]<<8);
				if (!((SubGroup == 0) || (SubGroup == 22) || (SubGroup == 23) || 
							(SubGroup == 24) || (SubGroup == 25))) {//Group card AmusementPark
					send("{\"C\":{\"Status\":2}}\n");
					return 255;
				}
				Discount = buf[2];
				if (!GetCardEtebar1(&etebar, &Hetebar)) {
					unsigned int len = sprintf(globalBuff, "{\"C\":{\"ID\":%d,\"UC\":%d,\"Etebar\":%u,\"Hedie\"" \
						":%d,\"Model\":%d}}\n", cardId, uc, etebar, Hetebar, SubGroup);
						//globalBuff[len] = getCRC(globalBuff) & 0xFF;
						//globalBuff[len + 1] = 0; //HNA CRC
					send(globalBuff);
					
					return 0;
				}
				else
					continue;	
			}	
			else { 
				send("{\"C\":{\"Status\":2}}\n"); 
				return 255;
			}
		}
	}
}
//=============================
#ifdef NFC
#include "pn532_stm32f1.h"

extern uint8_t NtagRead[4];  
extern uint8_t NtagWrite[4];
extern uint8_t NtagReadZiroBlock[4]; 
extern PN532 pn532;

uint8_t NtagWriteBlock(uint16_t block, uint8_t* data)
{
	unsigned char ser[4];
	unsigned int tickk = HAL_GetTick();
	while (1)
	{
		WDTR;
		if (HAL_GetTick() - tickk > 5000)
		{
			printf("writeTimeout\n");
			return 1;
		}
		if (ISO14443_SingleTagSelect(ser))
		{
			if (ISO14443_SingleTagSelect(ser))
				continue;
		}	
		if ( PN532_Ntag2xxWriteBlock(&pn532, data, block) == 0)
		{
			printf("Write\n");
			return 0;
		}
	}
}


uint8_t NtagReadBlock(uint16_t block, uint8_t* data)
{
	unsigned char ser[4];
	unsigned char read[4] = {0},res = 0xff;	
	unsigned int tickk = HAL_GetTick();
	while (1)
	{
		WDTR;
		if (HAL_GetTick() - tickk > 5000)
		{
			printf("raedTimeout\n");
			return 1;
		}
		if (ISO14443_SingleTagSelect(ser))
		{
			if (ISO14443_SingleTagSelect(ser))
				continue;
		}	
		if ( PN532_Ntag2xxReadBlock(&pn532, data, block) == 0)
		{
			printf("Read\n");
			return 0;
		}
	}
}

#endif

//#endif