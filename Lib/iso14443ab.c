#include "stm32f1_rc522.h"
#include "iso14443ab.h"
#include "pn532_stm32f1.h"
#include "main.h"
//========================================================================
//
//                  Mifare & MFRC531 High Level Instructions
//                   
//========================================================================
extern volatile unsigned long int timestamp;
unsigned char current_card_snr[10];
extern int32_t uid_len;
extern PN532 pn532;
//========================================================================
unsigned char MFRC531_Reset(void) {
  MFRC522_Init();  
  return NO_ERROR;
}

//========================================================================
unsigned char MFRC531_WriteMasterKey(unsigned char Block, unsigned char* data) {}
//========================================================================
unsigned char ISO14443_SingleTagSelect(unsigned char *snr) {
	#if defined(PN532_Routine)
	unsigned char i;
	uid_len = PN532_ReadPassiveTarget(&pn532, snr, PN532_MIFARE_ISO14443A, 50);
  if(uid_len == PN532_STATUS_ERROR)	{
		snr[0] = snr [1] = snr[2] = snr[3] = 0;
		return HAS_ERROR;
	}
	for (i=0; i<4; i++)
		current_card_snr[i] = snr[i];
//							send(current_card_snr);
#endif

	
#if defined(RC522_Routine)
	unsigned char 
		i,
		tmp,
		status,
		data[32]; 
	unsigned int Len=0;

	for (i=0; i<10; i++) 
		current_card_snr[i]=0; 
	
	if(MFRC522_Check(current_card_snr)) {		
		snr[0] = snr [1] = snr[2] = snr[3] = 0;
		return('N');
	}
	//Log2("Check")
	if(MFRC522_SelectTag(current_card_snr) == 0) {
		snr[0] = snr [1] = snr[2] = snr[3] = 0;
		return('N');
	}
	//Log2("Select")
	for(i=0; i<4; i++)
		snr[i]=current_card_snr[i];
	//printf("\n\r%X%X%X%X",current_card_snr[0],current_card_snr[1],current_card_snr[2],current_card_snr[3]);
#endif

  return NO_ERROR;
}

//========================================================================
unsigned char ISO14443_LoginE2(unsigned char Sector, unsigned char Key) {
//unsigned char Keys[5][16] = {{0x89,0x51,0x71,0xf2,0x1d,0xcb},
//                            {0x00,0x00,0x00,0x00,0x00,0x00},
//                            {0x00,0x00,0x00,0x00,0x00,0x00},
//                            {0x00,0x00,0x00,0x00,0x00,0x00},
//														{0x00,0x00,0x00,0x00,0x00,0x00}};

//unsigned char Keys[5][16] = {{0xFF,0xFF,0xFF,0xFF,0xFF,0xFF},   //card new
//                            {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF},
//                            {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF},
//                            {0xe9,0xc1,0x25,0x4f,0xa0,0x7d},
//                            {0xff,0xff,0xff,0xff,0xff,0xff}};
 														
 unsigned char Keys[7][16] ={{0x89,0x51,0x71,0xf2,0x1d,0xcb},
                             {0x8D,0x5F,0x30,0x7C,0xF0,0x5E},
														 {0xB2,0x2C,0xE4,0x24,0x7D,0x90},
                             {0x57,0x9B,0x07,0xB3,0xA4,0xE9},
													   {0xe3,0x2f,0x50,0xf2,0xe3,0xd0},
													   {0xe9,0xc1,0x25,0x4f,0xa0,0x7d},
                             {0xff,0xff,0xff,0xff,0xff,0xff}};

  
#if defined (PN532_Routine)
													 
	if(uid_len == -1 )
		return 1;
	if((current_card_snr[0] == 0) && (current_card_snr[1] == 0) && (current_card_snr[2] == 0))
		return 1;
	
  switch(Key)	{
		case 0x10:
			 return(PN532_MifareClassicAuthenticateBlock(&pn532, current_card_snr, uid_len, Sector*4, MIFARE_CMD_AUTH_A,Keys[0]));
		
		case 0x37:
			 return(PN532_MifareClassicAuthenticateBlock(&pn532, current_card_snr, uid_len, Sector*4, MIFARE_CMD_AUTH_B,Keys[1]));
		   
		case 0x14:
			 return(PN532_MifareClassicAuthenticateBlock(&pn532, current_card_snr, uid_len, Sector*4, MIFARE_CMD_AUTH_A,Keys[2]));

		case 0x35:
			 return(PN532_MifareClassicAuthenticateBlock(&pn532, current_card_snr, uid_len, Sector*4, MIFARE_CMD_AUTH_B,Keys[3]));

		case 0x39:
			 return(PN532_MifareClassicAuthenticateBlock(&pn532, current_card_snr, uid_len, Sector*4, MIFARE_CMD_AUTH_B,Keys[4]));

		case 0x33:
			 return(PN532_MifareClassicAuthenticateBlock(&pn532, current_card_snr, uid_len, Sector*4, MIFARE_CMD_AUTH_B,Keys[5]));
	   		
		case 0x12:
			 return(PN532_MifareClassicAuthenticateBlock(&pn532, current_card_snr, uid_len, Sector*4, MIFARE_CMD_AUTH_A,Keys[6]));
		
		default: return(1);
	}
  return(1);													 
#endif
#if defined (RC522_Routine)
  switch(Key)
	{
		case 0x10:
       return(MFRC522_Auth(0x60, Sector*4, Keys[0], current_card_snr));
		case 0x37:
       return(MFRC522_Auth(0x61, Sector*4, Keys[1], current_card_snr));
		case 0x14:
       return(MFRC522_Auth(0x60, Sector*4, Keys[2], current_card_snr));
		case 0x35:
       return(MFRC522_Auth(0x61, Sector*4, Keys[3], current_card_snr));
		case 0x39:
       return(MFRC522_Auth(0x61, Sector*4, Keys[4], current_card_snr));
		case 0x33:
	     return(MFRC522_Auth(0x61, Sector*4, Keys[5], current_card_snr));		
		case 0x12:
      return(MFRC522_Auth(0x60, Sector*4, Keys[6], current_card_snr));

		
		default: return(1);
	}
  return(1);
	#endif
  
}

//========================================================================
unsigned char ISO14443_Login(unsigned char Sector, 
														 unsigned char KeyType, 
														 unsigned char *Key) {}
//========================================================================
unsigned char ISO14443_ReadBlock(unsigned char Block, unsigned char *buf) {
	#if defined (PN532_Routine)
	return(PN532_MifareClassicReadBlock(&pn532, buf, Block));
	                              
	#endif
	#if defined (RC522_Routine)
  return(MFRC522_Read(Block, buf));
	#endif
}

//========================================================================
unsigned char ISO14443_WriteBlock(unsigned char Block, unsigned char* data) {
	#if defined (PN532_Routine)
	return(PN532_MifareClassicWriteBlock(&pn532, data, Block));
	#endif
	#if defined (RC522_Routine)
  return(MFRC522_Write(Block, data));
	#endif  
}

//========================================================================
unsigned char ISO14443_WriteValue(unsigned char Block,  signed long int Val) {
	unsigned char i;
	unsigned char data[4] = {1,0,0,0};
	unsigned char FormattedData[16];
	unsigned char d[16]; 

  data[0] =  Val 				& 0xFF;
  data[1] = (Val >> 8)  & 0xFF;
  data[2] = (Val >> 16) & 0xFF;
  data[3] = (Val >> 24) & 0xFF;
  
  FormattedData[0] = data[0];
  FormattedData[1] = data[1];
  FormattedData[2] = data[2];
  FormattedData[3] = data[3];

  FormattedData[4] = data[0] ^ 0xFF;
  FormattedData[5] = data[1] ^ 0xFF;
  FormattedData[6] = data[2] ^ 0xFF;
  FormattedData[7] = data[3] ^ 0xFF; 
  
  FormattedData[8]  = data[0];
  FormattedData[9]  = data[1];
  FormattedData[10] = data[2];
  FormattedData[11] = data[3];
  
  FormattedData[12] = 0;
  FormattedData[13] = 0xFF;
  FormattedData[14] = 0;
  FormattedData[15] = 0xFF;
  
	#if defined (PN532_Routine)
		return(PN532_MifareClassicWriteBlock(&pn532,FormattedData, Block));
	#endif
	#if defined (RC522_Routine)
  return(MFRC522_Write(Block, FormattedData));
	#endif
}

//========================================================================
unsigned char ISO14443_ReadValueBlock(unsigned char Block, signed long int *Val) {
	unsigned char i, data[16]; 
	#if defined (RC522_Routine)	
  if(MFRC522_Read(Block, data)!=0) return(1);
	#endif
	
	#if defined (PN532_Routine)
	if(PN532_MifareClassicReadBlock(&pn532, data, Block) !=0 ) return(1);
	#endif	
  for(i=0; i<4; i++) if(data[i]!=data[i+8]) { return(1); }  
  for(i=0; i<4; i++) if(data[i+4]!=(data[i]^0xFF)) { return(1); }  
  if((data[12]!=0)||(data[13]!=0xFF)||(data[14]!=0)||(data[15]!=0xFF)) { return(1); }  
  
  *Val=data[3];  *Val<<=8;
	*Val+=data[2]; *Val<<=8;
	*Val+=data[1]; *Val<<=8;
	*Val+=data[0];
	
  return NO_ERROR;
}

