#ifndef FINGER_H
#define FINGER_H

typedef struct {
                unsigned short Len;
	              unsigned short ID;
                unsigned char Buffer[571];
               }TemplateStruct;

extern TemplateStruct FTemplate;	
							 

extern char FPB_DeleteTemplate(unsigned int ID);
extern char FPB_DeleteAllTemplate(void);
extern char FPB_AddNewUser(unsigned int ID);
extern char FPB_Identify(void);
extern void EmptyFPRXBuffer(void);
extern unsigned char FPR_SetFingerBaudRate9600(void);
extern unsigned char FPR_FingerInit(void);
void FP_SendSerialData(unsigned char *buff);
void FPRN_Send_Command(unsigned short Command,unsigned short Len,unsigned char *Data);
void FPRN_Send_TemplateDataPacket(unsigned short Command,unsigned int Len,unsigned int ID,unsigned char *Data);
char FPRN_CancelCmd(void);
unsigned char FPRN_DeleteAllTemplate(void);
unsigned int FPRN_Identify(void) ;
char FPRN_DeleteTemplate(unsigned int ID);
void FPRN_GetTemplateByID(unsigned int FPID, unsigned char inHEX);
unsigned int FPRN_WriteTemplateByID(unsigned int Ftamplate_ID,unsigned int FTemplate_Len,unsigned char *_FTemplate);
unsigned int FPRN_GetEmptyID(void);
char FPRN_AddNewUser(unsigned int ID);
#endif //#ifndef FINGER_H

