#include "main.h"
#include "string.h"
#ifdef LCD
#include "LCD.h"
extern SPI_HandleTypeDef hspi1;
extern IWDG_HandleTypeDef hiwdg;
#define WDTR  __HAL_IWDG_RELOAD_COUNTER(&hiwdg);
unsigned char	FontWidth;			// max width of font
unsigned char	FontHeight;		
unsigned char *FontPointer;	
unsigned char 	rot = 0;			//Rot 0=0¬∞, 1=90¬∞

#define CLR_CS HAL_GPIO_WritePin(GPIOA, SPI1_CS_Pin, 0);
#define SET_CS HAL_GPIO_WritePin(GPIOA, SPI1_CS_Pin, 1);

#define CLR_RS HAL_GPIO_WritePin(TFTRS_GPIO_Port, TFTRS_Pin, 0);
#define SET_RS HAL_GPIO_WritePin(TFTRS_GPIO_Port, TFTRS_Pin ,1);

#define CLR_RESET  HAL_GPIO_WritePin(TFTRST_GPIO_Port, TFTRST_Pin ,0);
#define SET_RESET  HAL_GPIO_WritePin(TFTRST_GPIO_Port, TFTRST_Pin ,1);

#define DELAY for(unsigned int cnt =0;cnt<4099;cnt++) __NOP();

extern unsigned int _color ,BkColor,FgColor;

unsigned char FontFixed = 0;
unsigned int cursorX = 0;		// x position
unsigned int cursorY = 0;		// y position
unsigned char FontXScale = 1;	// X size of font
unsigned char FontYScale = 1;	// Y size of font
unsigned char FontSpace = 1;	// space between char

unsigned char	NonTransparence = 0;
unsigned short ScreenShift=0,ScreenYShift=0 ;
unsigned char English_Str[300],Farsi_Str[300],Mapped_FarsiStr[350];
unsigned char Mapped_Str1[300],Mapped_Str2[350],Mapped_Str3[500];
char InLcdRect=0;
unsigned char DisTFT35Type = 2;
HAL_StatusTypeDef stateSPI = HAL_ERROR;



void write_command24(unsigned int command)
{
	unsigned char C[1];	
	CLR_RS
	C[0]=command;
	CLR_CS
	stateSPI = HAL_SPI_Transmit(&hspi1,C,1,100);
	SET_CS
	SET_RS
}

void write_data(unsigned int data)
{
  unsigned char D[1];	
	SET_RS
	D[0]=data;
	CLR_CS
	stateSPI = HAL_SPI_Transmit(&hspi1,D,1,100);
	SET_CS
	CLR_RS	
}
void Init_TFT24(void)
{ 
  int i,k,m;   
  SET_RESET
  HAL_Delay(1);
  CLR_RESET
  HAL_Delay(1);
  SET_RESET
  HAL_Delay(1);

  
  
//***************************************************************
   //LCD SETING
   write_command24(0xB0);// Manufacturer Command Access Protect   
   write_data(0x3F); 
   write_data(0x3F); 
   HAL_Delay(100);   
   write_command24(0xFE);// MAGIC - TODO
   write_data(0x00);
   write_data(0x00);
   write_data(0x00);
   write_data(0x21);
   write_data(0xB4);   
   write_command24(0xB3);// Frame Memory Access and Interface Setting
   write_data(0x00);
   write_data(0x10);   
   write_command24(0xE0);//NVM Access Control
   write_data(0x00);
   write_data(0x40);
   HAL_Delay(10);   
   write_command24(0xB3);// Frame Memory Access and Interface Setting
   write_data(0x00);
   write_data(0x00);
   write_command24(0xFE);// MAGIC - TODO
   write_data(0x00);
   write_data(0x00);
   write_data(0x00);
   write_data(0x21);
   write_data(0x30);  
   write_command24(0xB0);// Manufacturer Command Access Protect 
   write_data(0x3F);
   write_data(0x3F);   
   write_command24(0xB3);// Frame Memory Access and Interface Setting
   write_data(0x02);
   write_data(0x00);
   write_data(0x00);
   write_data(0x00);   
   write_command24(0xB4);//SET interface
   write_data(0x00);      
   write_command24(0xC0); //Panel Driving Setting
   write_data(0x03);
   write_data(0x4F);
   write_data(0x00);
   write_data(0x10);
   write_data(0xA2);
   write_data(0x00);
   write_data(0x01);
   write_data(0x00);  
   write_command24(0xC1);//Display Timing Setting for Normal/Partial Mode
   write_data(0x01);
   write_data(0x02);//2
   write_data(0x19);
   write_data(0x08);
   write_data(0x08);
   HAL_Delay(25);
   write_command24(0xC3);
   write_data(0x01);  
   write_data(0x00);
   write_data(0x28);//28
   write_data(0x08);
   write_data(0x08);
   HAL_Delay(25);   
   write_command24(0xC4);
   write_data(0x11);
   write_data(0x01);  
   write_data(0x43);//43
   write_data(0x04);//0V   
   write_command24(0xC8);//set gamma
   write_data(0x0C);
   write_data(0x0C);
   write_data(0x0D);
   write_data(0x14);
   write_data(0x18);
   write_data(0x0E);
   write_data(0x09);
   write_data(0x09);
   write_data(0x03);
   write_data(0x05);
   write_data(0x00);
   write_data(0x03);
   write_data(0x08);
   write_data(0x07);
   write_data(0x0E);
   write_data(0x15);
   write_data(0x12);
   write_data(0x0A);
   write_data(0x0E);
   write_data(0x0A);
   write_data(0x0A);
   write_data(0x00);   
   write_command24(0xC9);//set gamma
   write_data(0x0C);
   write_data(0x0C);
   write_data(0x0D);
   write_data(0x14);
   write_data(0x18);
   write_data(0x0E);
   write_data(0x09);
   write_data(0x09);
   write_data(0x03);
   write_data(0x05);
   write_data(0x00);
   write_data(0x03);
   write_data(0x08);
   write_data(0x07);
   write_data(0x0E);
   write_data(0x15);
   write_data(0x12);
   write_data(0x0A);
   write_data(0x0E);
   write_data(0x0A);
   write_data(0x0A);
   write_data(0x00);  
   write_command24(0xCA);//set gamma
   write_data(0x0C);
   write_data(0x0C);
   write_data(0x0D);
   write_data(0x14);
   write_data(0x18);
   write_data(0x0E);
   write_data(0x09);
   write_data(0x09);
   write_data(0x03);
   write_data(0x05);
   write_data(0x00);
   write_data(0x03);
   write_data(0x08);
   write_data(0x07);
   write_data(0x0E);
   write_data(0x15);
   write_data(0x12);
   write_data(0x0A);
   write_data(0x0E);
   write_data(0x0A);
   write_data(0x0A);
   write_data(0x00);
   write_command24(0xD0);//Power Setting 
   write_data(0x63);//BT[2:0]=110  VCI+VCI2ÿåÿ°2  :5   -(VCI2ÿåÿ°2):  
   write_data(0x53);
   write_data(0x82);//VC2[2:0]=010,VCI2=5V
   write_data(0x3F);//VREG=5.0V        
   write_command24(0xD1);//set vcom
   write_data(0x6A);//vcomH
   write_data(0x64);//VDV  
   write_command24(0xD2);//Power Setting (Note 1) for Normal/Partial Mode
   write_data(0x03);//03
   write_data(0x24);//24   
   write_command24(0xD4);//Power Setting (Note 1) for Idle Mode
   write_data(0x03);
   write_data(0x24);  
   write_command24(0xE2);//NVM Load Control
   write_data(0x3F);  
   write_command24(0x36);//set_address_mode
   
   write_data(0x61);

    
   
   write_command24(0x3A);//set_pixel_format
   write_data(0x66);//66=18BIT
   
   write_command24(0x2A);//set_column_address 
   write_data(0x00);
   write_data(0x00);

   write_data(0x00);
   write_data(0xEF);   
   write_command24(0x2B);//set_page_address
   write_data(0x00);
   write_data(0x00);


	 write_data(0x01);
	 write_data(0xDF);

   
   //delay_ms(1000);
   write_command24(0x11);//exit_sleep_mode
   HAL_Delay(100);
   write_command24(0x29);//set_display_on
   HAL_Delay(50);
   write_command24(0x2c);//send DDRAM set
   HAL_Delay(250);   

 }
 
void LCDSetCur24(unsigned int col,unsigned int row)
{
  write_command24(0x2a);
	unsigned char Col = col>>8 ;
  write_data(Col);	    //start Y
	Col = col & 0xff;
  write_data(Col);	    //start Y
	Col = col>>8 ;
  write_data(Col);	    //end Y
	Col = col & 0xff;
  write_data(Col);	    //end Y	
  write_command24(0x2b);	
	unsigned char Row = row>>8;
  write_data(Row);	    //start X
	Row = row & 0xff;
  write_data(Row);	    //start X
	Row = row>>8;
  write_data(Row);	    //end X
	Row = row & 0xff;
  write_data(Row);	    //end X    
  write_command24(0x2c); 	
}

void LCD24_SetArea(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2)
{
  unsigned char D[10];
  write_command24(0x2a);
  D[0]=(y1>>8);
  D[1]=(y1&0xff);
  D[2]=(y2>>8);
  D[3]=(y2&0xff);
   //PIO_Clear(&LCD_CS);
  SET_RS//PIO_Set(&LCD_RS);   
  CLR_CS		
	HAL_Delay(1);
	stateSPI = HAL_SPI_Transmit(&hspi1,D,4,100);
	HAL_Delay(1);
	SET_CS  
  //.......................................................................... 
  write_command24(0x2b);
  //..........................................................................
  D[0]=(x1>>8);
  D[1]=(x1&0xff);
  D[2]=(x2>>8);
  D[3]=(x2&0xff);
  //PIO_Clear(&LCD_CS);
  SET_RS//PIO_Set(&LCD_RS);   
	CLR_CS
	HAL_Delay(1);
  stateSPI = HAL_SPI_Transmit(&hspi1,D,4,100);
	HAL_Delay(1);
	SET_CS
 

  //..........................................................................
  write_command24(0x2c); 									 
  //..........................................................................
  
 	 
}
void LCDClear(unsigned int Color)
{
	 unsigned int i,j, b;
	 unsigned char DDD[1110];
	 //unsigned char DD[3];
	 LCDSetCur24(480, 320);
	 LCD24_SetArea(0, 0, 319, 479);
	 DDD[0]=(Color);
	 DDD[1]=(Color>>8);
	 DDD[2]=(Color>>16);  
	 CLR_CS;
	 SET_RS;
	 HAL_Delay(1);
	 HAL_SPI_Transmit(&hspi1,DDD,3,100);	 
	 HAL_Delay(1);
	 SET_CS
//	 DD[0]=Color;
//	 DD[1]=Color>>8;
//	 DD[2]=Color>>16; 
	 b=0;
	 for(i=0; i<3; i++)
	 {
			b=0;  
			for(j=0; j<370; j++)            
			{ 
         WDTR				
				 DDD[b++]=Color;
				 DDD[b++]=Color>>8;
				 DDD[b++]=Color>>16; 
				 CLR_CS
				 //for(unsigned int t=0;t<10000;t++);
				 DELAY
				 stateSPI = HAL_SPI_Transmit(&hspi1,DDD,b,100);	
				 //HAL_Delay(1);
				 SET_CS	
			}        
	 }
	 LCD24_SetArea(0, 0, 479, 319);  
}
void LcdFont(unsigned char *pointer)
{
	FontWidth 	= pointer[2];
	FontHeight	= pointer[3];
  FontPointer=pointer;
}
unsigned int GetMaxX2 (void)
{
 return(((unsigned int)SCREEN_HOR_SIZE-1));  
}
unsigned int  GetMaxY2 (void)
{
 return(((unsigned int)SCREEN_VER_SIZE-1));
}
void write_pixel(unsigned int y,unsigned int x,int color)
{ 
    unsigned char D[10];
    unsigned int x1=10,x2=20,y1=50,y2=100;

    write_command24(0x2a);

    D[0]=D[2]=(y>>8);
    D[1]=D[3]=(y&0xff);


    CLR_CS//PIO_Clear(&LCD_CS);
    SET_RS//PIO_Set(&LCD_RS);     


   



	  //HAL_Delay(1);
	  HAL_SPI_Transmit(&hspi1,D,4,100);
		//HAL_Delay(1);
	  SET_CS

    

    
    //.......................................................................... 
    write_command24(0x2b);
    //..........................................................................
    D[0]=D[2]=(x>>8);
    D[1]=D[3]=(x&0xff);


    CLR_CS //PIO_Clear(&LCD_CS);
    SET_RS //(&LCD_RS);      

    
 
      

    //HAL_Delay(1);
	  HAL_SPI_Transmit(&hspi1,D,4,100);
		//HAL_Delay(1);
	  SET_CS

    
  
    //..........................................................................
    write_command24(0x2c); 									 
    //..........................................................................
    D[0]=color;
    D[1]=color>>8;
    D[2]=color>>16;
    CLR_CS//PIO_Clear(&LCD_CS);
    SET_RS//(&LCD_RS);    
    


    
    //HAL_Delay(1);
	  HAL_SPI_Transmit(&hspi1,D,3,100);
		//HAL_Delay(1);
	  SET_CS
     
    
 }

void LCD_PutPixel(unsigned int x, unsigned int y)
{
	write_pixel(x,y,_color);
}
void PutChar(unsigned char c)
{
	unsigned char xc, yc, sx, sy;
	unsigned char Ccounter;
	unsigned int 	fontSize		=	(FontPointer[0]<<8)+FontPointer[1];
	unsigned char 	fontHeigh		=	FontPointer[3];
  unsigned char 	fontBitsPixel           =       FontPointer[4];
	unsigned char 	firstchar 		=	FontPointer[5];
	unsigned char 	lastchar 		=	FontPointer[6];
	unsigned char 	charwidth	 	= 	FontPointer[7+c-firstchar];
	unsigned char 	byte 			= 	0;
	unsigned char 	bitoffset 		= 	0;
	unsigned char 	maske 			= 	0;
	unsigned int 	bcounter 		= 	0;
	unsigned int 	bitsbischar 	= 	0;
	unsigned int 	bytesbischar	= 	0;
	unsigned int 	xPos,yPos;
  unsigned int offset=0;
  unsigned int i;
  unsigned char x=1,Table[5];
	unsigned int 	color = GetColor;	// backup color
        unsigned int _rX=0,_rY=0;

 
  if((c<firstchar)||(c>lastchar))
  {
    cursorX += 5;
    return;
  }  

  if(c==32)
  {
    cursorX += 5;
    return;
  }  
  
  if(fontBitsPixel & 0x80) //Compress
    {
    }
 
 
  offset=8+(lastchar-firstchar);
  for(i=0; i<c-firstchar; i++)
  offset+=(FontPointer[7+i]*(fontHeigh/8));
 // line adjust
     if(cursorX + charwidth * FontXScale > GetMaxX)
	{
		cursorY = cursorY + (unsigned int)FontHeight * FontYScale; 
		cursorX = 0;
	}

	for(Ccounter = 0; Ccounter < c-firstchar; Ccounter++)
            bitsbischar += (FontPointer[Ccounter+7]);


	for(xc = 0; xc < charwidth; xc++) // Eigentlcihe Ausgaberoutine
	{
		x=1;
                for(yc = 0; yc < fontHeigh; yc++)
		{
			if((yc)&&(yc%8==0))
                        {
                          offset++;
                          x=1;
                        }
                        byte=FontPointer[offset];
                        byte&=x;
                        x<<=1;

			xPos = (unsigned int)xc*FontXScale + cursorX;
#ifdef Horizontal
			yPos = (((unsigned int)yc)*FontYScale + cursorY);
#else
//			yPos = (((unsigned int)yc)*FontYScale + cursorY);
			yPos = (((unsigned int)FontHeight-yc)*FontYScale + cursorY);
#endif

			for(sx = 0; sx < FontXScale; sx++)
			{
				for(sy = 0; sy < FontYScale; sy++)
				{
					if(byte)
					{
						SetColor(GetFgColor());
						LCD_PutPixel(xPos+sx+_rX,yPos +sy+_rY);
					}
					else
					{
						if(NonTransparence)
						{
							SetColor(GetBkColor());
								LCD_PutPixel(xPos+sx+_rX,yPos +sy+_rY);
						}
					}
				}
			}
		}
                offset++;
	}
	SetColor(color);

	// adjust cursor to next position
	cursorX += charwidth * FontXScale + FontSpace;

}
void Puts(unsigned char *Text)
{	
unsigned int i=0;

	unsigned char c;
	while((c = Text[i++])!='\0')
	{
          PutChar(c);
	}
}
void PutText(unsigned int left, unsigned int top, unsigned int right, unsigned int bottom, char *str, unsigned char style)
{

// #ifdef TFT35WIDE
// left+=25;
// top+=36;
// 
// #if(DeviceType!=VMODULE)
//// top+=36;
//// bottom+=36;
// #endif
// 
// if((DeviceType==BCU)||(DeviceType==APARKINC))
// {
// left+=ScreenShift;

// } 
// #endif 
 

 if(DisTFT35Type==2)
  { 
     left+=20;
     top+=36;


        left+=ScreenShift;
         top+=ScreenYShift;

  }

 
 SetCursorX(left);
 SetCursorY(top);
 
 Puts((unsigned char *)str);
}
unsigned char Special_Charachters(unsigned char C)
{
 if((C == '.')||(C==',')||(C=='(')||(C==')')||(C=='=')||(C=='!')||(C=='%')||(C=='*')||(C=='-')||(C==' ')||(C=='\\')||(C=='\\')||(C==10)||(C==13)) return(1);
 return(0);
}
int isdigit(int _C)
{
      return _C >= '0' && _C <= '9';
}

unsigned char Dos2Farsi(unsigned char *str1,unsigned char *OutStr,unsigned char Direction)
{
        unsigned char i,j,Pos_Num,Pos_First,Pos_Last,Pos_Mid,Pos_Alone;
        unsigned char InStr[200],tempstr[200];
        unsigned char * str;
        unsigned char Len;
        
        Len = strlen((char *)str1);
        for(i=0;i<Len;i++)     InStr[i] = str1[Len-i-1];     
         

        
        i = 0;
        
        
        while(i<Len)
             {
              if(isdigit(InStr[i]))
                {
                 Pos_Num = i;
                 i++;  
                 while((i < Len) && (isdigit(InStr[i])) )       i++;
                 for(j=0;j<(i-Pos_Num);j++)                     tempstr[j] = InStr[Pos_Num+j];
                 for(j=0;j<(i-Pos_Num);j++)                     InStr[Pos_Num+j] = tempstr[i-Pos_Num-1-j];
                }
              else
                 i++;                                
             }
       
        j = 0;
        
    //  printf("\n\rIn > ",Len);        
     // for(i=0;i<Len;i++)   printf("%d,",InStr[i]);   
        
        for(i=0;i<Len;i++)  
           {                
             if(InStr[i]==32)  OutStr[j] =0x20;
             else if(InStr[i]=='.')   OutStr[j]=47;
             else if(InStr[i]=='-')   OutStr[j]=45;
             else if(InStr[i]=='(')   OutStr[j]=40;
             else if(InStr[i]==')')   OutStr[j]=41;
             else if(InStr[i]=='*')   OutStr[j]=42;
             else if(InStr[i]=='ø')   OutStr[j]=63;
             else if(InStr[i]=='=')   OutStr[j]=61;             
             else if(InStr[i]=='!')   OutStr[j]=33;             
             else if(InStr[i]=='°')   OutStr[j]=44;
             else if(InStr[i]=='\\')  OutStr[j]=44;
             else if(InStr[i]=='//')  OutStr[j]=0x2E;             
             else if(InStr[i] > 127)
              {
                OutStr[j] =0x20;
              
                if(i == Len-1)     Pos_First = 1;
                else if(InStr[i+1] == '«' || InStr[i+1] == '¬' ||InStr[i+1] == 'œ' || InStr[i+1] == '–' || InStr[i+1] == '—' || InStr[i+1] == '“' || InStr[i+1] == 'é' || InStr[i+1] == 'Ê' || InStr[i+1] == ' ')
                                   Pos_First = 1;
                else 
                                   Pos_First = 0;
                        
                if(i == 0)         Pos_Last = 1;
                else if(InStr[i-1] == ' ')
                                   Pos_Last = 1;
                else
                                   Pos_Last = 0;
                        
                if(Pos_Last == 1 && Pos_First == 1)
                  {
                   Pos_Alone = 1;
                   Pos_Last = 0;
                   Pos_First = 0;
                  }
                else
                  {
                   Pos_Alone = 0;
                   if(Pos_Last == 0 && Pos_First == 0)
                     {
                      Pos_Mid = 1;
                      Pos_Last = 0;
                      Pos_First = 0;
                     }
                   else
                      Pos_Mid = 0;
                  }
                        
                
                 switch(InStr[i]){
                        case '¬':    
                                if(Pos_Alone)
                                        OutStr[j] = 67;//176;
                                else if(Pos_First)
                                        OutStr[j] = 67;//176;
                                else if(Pos_Last)
                                        OutStr[j] = 67;//176;
                                else
                                        OutStr[j] = 67;//176;                                
                                break;
                        case '«':    
                                if(Pos_Alone)
                                        OutStr[j] = 65;//177; ÿß
                                else if(Pos_First)
                                        OutStr[j] = 65;//177; ÿß
                                else if(Pos_Last)
                                        OutStr[j] = 66;//178;   ÿ®ÿß
                                else
                                        OutStr[j] = 66;//178;   ÿ®ÿß                             
                                break;
                        case '»':    
                                if(Pos_Alone)
                                        OutStr[j] = 74;//181;    ÿ® 
                                else if(Pos_First)
                                        OutStr[j] = 77;//179;    ÿ®ÿ≥
                                else if(Pos_Last)
                                        OutStr[j] = 75;//181;    ÿ≥ÿ®
                                else
                                        OutStr[j] = 76;//180;    ÿ≥ÿ®ÿ≥                            
                                break;
                        case 'Å':    
                                if(Pos_Alone)
                                        OutStr[j] = 78;//181;    ÿ® 
                                else if(Pos_First)
                                        OutStr[j] = 81;//179;    ÿ®ÿ≥
                                else if(Pos_Last)
                                        OutStr[j] = 79;//181;    ÿ≥ÿ®
                                else
                                        OutStr[j] = 80;//180;    ÿ≥ÿ®ÿ≥                            
                                break;
                        case ' ':    
                                if(Pos_Alone)
                                        OutStr[j] = 82;//186;
                                else if(Pos_First)
                                        OutStr[j] = 85;//184;
                                else if(Pos_Last)
                                        OutStr[j] = 83;//186;
                                else
                                        OutStr[j] = 84;//185;                                
                                break;
                        case 'À':    
                                if(Pos_Alone)
                                        OutStr[j] = 86;
                                else if(Pos_First)
                                        OutStr[j] = 89;
                                else if(Pos_Last)
                                        OutStr[j] = 87;
                                else
                                        OutStr[j] = 88;                                
                                break;
                        case 'Ã':    
                                if(Pos_Alone)
                                        OutStr[j] = 90;
                                else if(Pos_First)
                                        OutStr[j] = 93;
                                else if(Pos_Last)
                                        OutStr[j] = 91;
                                else
                                        OutStr[j] = 92;                                
                                break;
                        case 'ç':    
                                if(Pos_Alone)
                                        OutStr[j] = 94;
                                else if(Pos_First)
                                        OutStr[j] = 97;
                                else if(Pos_Last)
                                        OutStr[j] = 95;
                                else
                                        OutStr[j] = 96;                                
                                break;
                        case 'Õ':    
                                if(Pos_Alone)
                                        OutStr[j] = 98;
                                else if(Pos_First)
                                        OutStr[j] = 101;
                                else if(Pos_Last)
                                        OutStr[j] = 99;
                                else
                                        OutStr[j] = 100;                                
                                break;
                        case 'Œ':    
                                if(Pos_Alone)
                                        OutStr[j] = 102;
                                else if(Pos_First)
                                        OutStr[j] = 105;
                                else if(Pos_Last)
                                        OutStr[j] = 103;
                                else
                                        OutStr[j] = 104;                                
                                break;
                        case 'œ':    
                              //  if(Pos_Alone)
                                      OutStr[j] = 106;    
                              //  else 
                              //        OutStr[j] = 107;  
                                break;
                        case '–':    
                              //  if(Pos_Alone)
                                      OutStr[j] = 108;    
                              //  else 
                              //        OutStr[j] = 109;                                
                                break;                  
                        case '—':    
                              //  if(Pos_Alone)
                                      OutStr[j] = 110;    
                              //  else 
                              //        OutStr[j] = 111;                                 
                                break;
                        case '“':    
                              //  if(Pos_Alone)
                                      OutStr[j] = 112;    
                              //  else 
                              //        OutStr[j] = 113;                                 
                                break;
                        case 'é':    
                              //  if(Pos_Alone)
                                      OutStr[j] = 114;    
                              //  else 
                              //        OutStr[j] = 115;                                 
                                break;
                        case '”':    
                                if(Pos_Alone)
                                        OutStr[j] = 116;
                                else if(Pos_First)
                                        OutStr[j] = 119;
                                else if(Pos_Last)
                                        OutStr[j] = 117;
                                else
                                        OutStr[j] = 118;                                
                                break;
                        case '‘':    
                                if(Pos_Alone)
                                        OutStr[j] = 120;//1;
                                else if(Pos_First)
                                        OutStr[j] = 123;//204;
                                else if(Pos_Last)
                                        OutStr[j] = 121;//1;
                                else
                                        OutStr[j] = 122;//204;                                
                                break;
                        case '’':    
                                if(Pos_Alone)
                                        OutStr[j] = 124;
                                else if(Pos_First)
                                        OutStr[j] = 126;
                                else if(Pos_Last)
                                        OutStr[j] = 125;
                                else
                                        OutStr[j] = 126;                                
                                break;
                        case '÷':    
                                if(Pos_Alone)
                                        OutStr[j] = 231;
                                else if(Pos_First)
                                        OutStr[j] = 131;
                                else if(Pos_Last)
                                        OutStr[j] = 232;
                                else
                                        OutStr[j] = 130;                                
                                break;
                        case 'ÿ':    
                                if(Pos_Alone)
                                        OutStr[j] = 132;
                                else if(Pos_First)
                                        OutStr[j] = 135;
                                else if(Pos_Last)
                                        OutStr[j] = 133;
                                else
                                        OutStr[j] = 134;                                
                                break;
                        case 'Ÿ':    
                                if(Pos_Alone)
                                        OutStr[j] = 136;
                                else if(Pos_First)
                                        OutStr[j] = 139;
                                else if(Pos_Last)
                                        OutStr[j] = 137;
                                else
                                        OutStr[j] = 138;
                                break;
                        case '⁄':
                                if(Pos_Alone)
                                        OutStr[j] = 140;
                                else if(Pos_First)
                                        OutStr[j] = 238;
                                else if(Pos_Last)
                                        OutStr[j] = 233;
                                else
                                        OutStr[j] = 237;
                                break;
                        case '€':
                                if(Pos_Alone)
                                        OutStr[j] = 239;
                                else if(Pos_First)
                                        OutStr[j] = 147;
                                else if(Pos_Last)
                                        OutStr[j] = 145;
                                else
                                        OutStr[j] = 146;
                                break;
                        case '›':
                                if(Pos_Alone)
                                        OutStr[j] = 148;//
                                else if(Pos_First)
                                        OutStr[j] = 151;
                                else if(Pos_Last)
                                        OutStr[j] = 149;//
                                else
                                        OutStr[j] = 150;
                                break;
                        case 'ﬁ':    
                                if(Pos_Alone)
                                        OutStr[j] = 152;
                                else if(Pos_First)
                                        OutStr[j] = 155;
                                else if(Pos_Last)
                                        OutStr[j] = 153;
                                else
                                        OutStr[j] = 154;                                
                                break;
                        case 'ò':
                                if(Pos_Alone)
                                        OutStr[j] = 156;
                                else if(Pos_First)
                                        OutStr[j] = 159;
                                else if(Pos_Last)
                                        OutStr[j] = 240;
                                else
                                        OutStr[j] = 241;                                
                                break;
                        case 'ê':
                                if(Pos_Alone)
                                        OutStr[j] = 242;
                                else if(Pos_First)
                                        OutStr[j] = 163;
                                else if(Pos_Last)
                                        OutStr[j] = 161;
                                else
                                        OutStr[j] = 162;                                
                                break;
                        case '·':    
                                if(Pos_Alone)
                                        OutStr[j] = 164;
                                else if(Pos_First)
                                        OutStr[j] = 167;
                                else if(Pos_Last)
                                        OutStr[j] = 165;
                                else
                                        OutStr[j] = 166;     //ÿ≥ŸÑÿ≥                           
                                break;
                        case '„':    
                                if(Pos_Alone)
                                        OutStr[j] = 168;
                                else if(Pos_First)
                                        OutStr[j] = 171;
                                else if(Pos_Last)
                                        OutStr[j] = 169;
                                else
                                        OutStr[j] = 170;                                
                                break;
                        case '‰':
                                if(Pos_Alone)
                                        OutStr[j] = 172;
                                else if(Pos_First)
                                        OutStr[j] = 175;
                                else if(Pos_Last)
                                        OutStr[j] = 173;
                                else
                                        OutStr[j] = 174;                                
                                break;
                        case 'Ê':    
                            //    if(Pos_Alone)
                                      OutStr[j] = 176;    
                            //    else 
                            //          OutStr[j] = 177;                                
                                break;
                        case 'Â':    
                                if(Pos_Alone)
                                        OutStr[j] = 178;
                                else if(Pos_First)
                                        OutStr[j] = 181;
                                else if(Pos_Last)
                                        OutStr[j] = 179;
                                else
                                        OutStr[j] = 180;                                
                                break;
                        case 'Ì':    
                                if(Pos_Alone)
                                        OutStr[j] = 186;
                                else if(Pos_First)
                                        OutStr[j] = 189;
                                else if(Pos_Last)
                                        OutStr[j] = 187;
                                else
                                        OutStr[j] = 188;                                
                                break;
                        }
                        //j++; 
                }
                else
                {       
                    
                  if(isdigit(InStr[i]))
                                OutStr[j] = InStr[i];
                        else
                                OutStr[j] = InStr[i];
                      //  j++;
                  
                }
              j++;  
                            
        }                  
         
       OutStr[j] = 0; 
       
       if(Direction==R2L)
         {
          for(i=0;i<Len;i++)     InStr[i] = OutStr[Len-i-1];   
          for(i=0;i<Len;i++)     OutStr[i] = InStr[i];   
          OutStr[j] = 0; 
         } 
        
        return 0;
} 


void PutChar2(unsigned char c,unsigned char Direction)
{
//    0x12, 0x10, 0x18, 0x20, 0x81, 0x23, 0xF2,
  //0x24, 0x9A, 0x18, 0x20, 0x01, 0x21, 0xF2, old
	unsigned char xc, yc, sx, sy;
	unsigned char Ccounter;
	unsigned int 	fontSize		=	(FontPointer[0]<<8)+FontPointer[1];
	unsigned char 	fontHeigh		=	FontPointer[3];
	unsigned char 	firstchar 		=	FontPointer[5];
	unsigned char 	lastchar 		=	FontPointer[6];
	unsigned char 	charwidth	 	= 	FontPointer[7+c-firstchar];
	unsigned char 	byte 			= 	0;
	unsigned char 	bitoffset 		= 	0;
	unsigned char 	maske 			= 	0;
	unsigned int 	bcounter 		= 	0;
	unsigned int 	bitsbischar 	= 	0;
	unsigned int 	bytesbischar	= 	0;
	unsigned int 	xPos,yPos;
        unsigned int offset=0;
        unsigned int i;
        unsigned char x=1;
	unsigned int 	color = GetColor;	// backup color

  if((c<firstchar)||(c>lastchar)||(c==0x20))
  {
    if(Direction==L2R)      cursorX += 5;
    else if(Direction==R2L) cursorX -= 5;    
    return;
  }  
  offset=8+(lastchar-firstchar);
  for(i=0; i<c-firstchar; i++)      offset+=(FontPointer[7+i]*(fontHeigh/8));
  
  if(FontFixed) charwidth = FontWidth;
  else
    {
     if (((c >= 0xd4) && (c <= 0xda)) || 
	  ((c >= 0xe7) && (c <= 0xec)) ||
	    (c == 0xd1))
   	      {
		// cursorX = cursorX - charwidth * FontXScale;
	      }
     }
  
   //printf(" *%d-%d * ",cursorX,charwidth);
   //FontSpace=0;
   //if((c>=0x20)&&(c<=0x7D)) FontSpace=1;
   if(Direction==R2L)  cursorX -= charwidth * FontXScale + FontSpace; 

  //line adjust
   if (cursorX + charwidth * FontXScale > GetMaxX)
      {
	cursorY = cursorY + (unsigned int)FontHeight * FontYScale; 
	cursorX = 0;
      }

   for(Ccounter = 0; Ccounter < c-firstchar; Ccounter++)   bitsbischar += (FontPointer[Ccounter+7]);


   SetColor(GetFgColor());
   for(xc = 0; xc < charwidth; xc++) // Eigentlcihe Ausgaberoutine
      {
	x=1;
        for(yc = 0; yc < fontHeigh; yc++)
	   {
	    if((yc)&&(yc%8==0))
              {
               offset++;
               x=1;
              }
            byte=FontPointer[offset];
            byte&=x;
            x<<=1;

       	    xPos = (unsigned int)xc*FontXScale + cursorX;

#ifdef Horizontal
			yPos = (((unsigned int)yc)*FontYScale + cursorY);
#else
//			yPos = (((unsigned int)yc)*FontYScale + cursorY);
			yPos = (((unsigned int)FontHeight-yc)*FontYScale + cursorY);
#endif

			for(sx = 0; sx < FontXScale; sx++)
			{
				for(sy = 0; sy < FontYScale; sy++)
				{
					if(byte)
					{
		          		//SetColor(GetFgColor());
                                        // if(DisplayType==TFT3)
                                          //  LCD_PutPixel(xPos+sx,yPos +sy);
                                         //else 
                                         //{
					    LCD_PutPixel(xPos+sx,yPos+sy);
        
                                        // }
					}
					else
					{
	   				if(NonTransparence)
					  {
				           SetColor(GetBkColor());
                                           //if(DisplayType==TFT3)
					      //LCD_PutPixel(xPos+sx,yPos +sy);
                                           //else 
                                           //{
					      LCD_PutPixel(xPos+sx,yPos +sy);
                                             
                                           //}
                                                        
						}
					}
				}
			}
		}
                offset++;
	}
	SetColor(color);

	// adjust cursor to next position
   if(Direction==L2R)  cursorX += charwidth * FontXScale + FontSpace;


}
unsigned char Get_CharWidth(unsigned char c)
{
	unsigned int 	fontSize		=	(FontPointer[0]<<8)+FontPointer[1];
	unsigned char 	fontHeigh		=	FontPointer[3];
	unsigned char 	firstchar 		=	FontPointer[5];
	unsigned char 	lastchar 		=	FontPointer[6];
	unsigned char 	charwidth	 	= 	FontPointer[7+c-firstchar];

  if((c<firstchar)||(c>lastchar)||(c==0x20))
  {
    return(5);
  }  

  
   //FontSpace=0;
   //if((c>=0x20)&&(c<=0x7D)) FontSpace=1;   //English

   return(charwidth * FontXScale + FontSpace); 

}

void Show_TextInTFT(unsigned int X_Start,unsigned int Y_Start,unsigned int X_End,unsigned int Y_End,const unsigned char *str1,unsigned char Direction,unsigned char Center_Justify)
{
        unsigned int i,j,l,Farsi_Start,English_Start;
        unsigned char Mapped_BufferLastIndex,Input_BufferLastIndex,Num_OfShiftByte;
        signed char k;
        unsigned char * str;
        unsigned int Len;
        unsigned char c;
        unsigned int Text_BoarderWidth=0,Char_WidthSum,Found_SpaceFlag,Have_Enter;
       
// printf(" (%d-%d > ",X_Start,X_End);
 
 #ifdef TFT35WIDE

 
  #if(DeviceType==VMODULE)

  #else
  X_Start+=25;
  Y_Start+=36;        
  X_End+=25;
  Y_End+=36;
  #endif
 
 if((DeviceType==BCU)||(DeviceType==APARKINC))
 {
 X_Start+=ScreenShift;
 X_End+=ScreenShift;
 }
 #endif  
 
// printf(" %d-%d ) ",X_Start,X_End);
 

 
 #ifdef NEWHOMA
 if(DisTFT35Type==2)
    { 
     X_Start+=20;
     X_End+=20;

      if((DeviceType==BCU)||(DeviceType==APARKINC))
       {
        X_Start+=ScreenShift;
        X_End+=ScreenShift;
        Y_Start+=ScreenYShift;
        Y_End+=ScreenYShift;
       }
    }
 #endif
        
 English_Str[0]=Farsi_Str[0]=Mapped_FarsiStr[0]=Mapped_Str1[0]=Mapped_Str2[0]=Mapped_Str3[0]=0;
        
 if(Direction==R2L) if(X_Start<=X_End) return;
 if(Direction==L2R) if(X_Start>=X_End) return;        

 if((Direction!=L2R)&&(Direction!=R2L))  return;
        
  Len =strlen((char *)str1);

        if(Len==0)  return;
        
        if(Len>=300) Len=300;
        for(i=0;i<Len;i++)   Mapped_Str1[i]=str1[i];
       
        
        i = 0;
        
        if(Direction==R2L) Text_BoarderWidth=X_Start-X_End;
        else               Text_BoarderWidth=X_End-X_Start;

  //      printf(" Text_BoarderWidth=%d  ",Text_BoarderWidth);
          
        while(i<Len)
             {
              if(Mapped_Str1[i] > 127)
                {
                 k=0;
                 Farsi_Start=i;
                 while((i<Len)&&((Mapped_Str1[i] > 127)||(Special_Charachters(Mapped_Str1[i]))))
                      {
                       Farsi_Str[k]=Mapped_Str1[i];
                       k++;
                       i++;  
                      }
                 Farsi_Str[k]=0;
                 Dos2Farsi(Farsi_Str,Mapped_FarsiStr,Direction);
                 for(j=0;j<k;j++) Mapped_Str1[Farsi_Start+j]=Mapped_FarsiStr[j];
                }
              else if(Mapped_Str1[i] <= 127)
                {
                 k=0;
                 English_Start=i;
                 while((i<Len)&&(Mapped_Str1[i] <= 127))
                      {
                       English_Str[k]=Mapped_Str1[i];
                       k++;
                       i++;  
                      }
                 English_Str[k]=0;
                 if(Direction==R2L)    for(j=0;j<k;j++) Mapped_Str1[English_Start+j]=English_Str[k-j-1];
                }
             
             }

        Mapped_Str1[i]=0;
       
        //..............................................................................................................................................................
        i=0;
        Char_WidthSum=0;
        Input_BufferLastIndex=0;
        Mapped_BufferLastIndex=0;
        while(i<Len)
             {
              if((Mapped_Str1[i]==13)||Mapped_Str1[i]==10)
                {
                 Char_WidthSum=0;
                 for(;Input_BufferLastIndex<i;Input_BufferLastIndex++)          Mapped_Str2[Mapped_BufferLastIndex++]=Mapped_Str1[Input_BufferLastIndex];                      
                 Mapped_Str2[Mapped_BufferLastIndex++]=255;
                 Input_BufferLastIndex++;
                 i++;
                 while((i<Len)&&(Mapped_Str1[i]==32)) i++;
                 
                 continue;
                }
              
               Char_WidthSum+=Get_CharWidth(Mapped_Str1[i]);

              if(Char_WidthSum>=Text_BoarderWidth) 
                {
//                    printf(" Char_WidthSum>=Text_BoarderWidth  ");
                    
                  Found_SpaceFlag=0;
                  for(k=i;k>Input_BufferLastIndex;k--) 
                    {
                      if(Mapped_Str1[k]==32) //Space
                       {

                        for(;Input_BufferLastIndex<(k);Input_BufferLastIndex++)  Mapped_Str2[Mapped_BufferLastIndex++]=Mapped_Str1[Input_BufferLastIndex];
                        Mapped_Str2[Mapped_BufferLastIndex++]=255;
                        Input_BufferLastIndex++;
                        Found_SpaceFlag=1;
                        i=k+1;
                        break;
                       }
                    }
                  if(!Found_SpaceFlag) 
                    {
                     for(;Input_BufferLastIndex<i;Input_BufferLastIndex++)       Mapped_Str2[Mapped_BufferLastIndex++]=Mapped_Str1[Input_BufferLastIndex];                      
                     Mapped_Str2[Mapped_BufferLastIndex++]=255;
                    }
                  Char_WidthSum=0;
                 while((i<Len)&&(Mapped_Str1[i]==32)) i++;
                }
               else i++;
             }
        for(;Input_BufferLastIndex<Len;Input_BufferLastIndex++)     Mapped_Str2[Mapped_BufferLastIndex++]=Mapped_Str1[Input_BufferLastIndex]; 
        Mapped_Str2[Mapped_BufferLastIndex]=0;
        Len = strlen((char *)Mapped_Str2);
      
        //..............................................................................................................................................................        

        if(Center_Justify)
          {
           i=0;
           Char_WidthSum=0;
           Have_Enter=0;
           Input_BufferLastIndex=0;
           Mapped_BufferLastIndex=0;
           while(i<Len)
             {
              while((i<Len)&&(Mapped_Str2[i]!=255))
                   {
                    Char_WidthSum+=Get_CharWidth(Mapped_Str2[i]);
                    i++;
                    Have_Enter=1;
                   }
              if(Have_Enter==1)
                {
                 Num_OfShiftByte=(Text_BoarderWidth-Char_WidthSum)/2;
                 
                 Mapped_Str3[Mapped_BufferLastIndex++]=254;  
                 Mapped_Str3[Mapped_BufferLastIndex++]=Num_OfShiftByte;  
                 
                 for(;Input_BufferLastIndex<i;Input_BufferLastIndex++)          Mapped_Str3[Mapped_BufferLastIndex++]=Mapped_Str2[Input_BufferLastIndex]; 
                 Have_Enter=0;
                 Input_BufferLastIndex++;
                 Mapped_Str3[Mapped_BufferLastIndex++]=255;
                 Char_WidthSum=0;
                }
              i++;
             }
           Mapped_Str3[Mapped_BufferLastIndex]=0;
         }
        else
         {
           for(i=0;i<Len;i++)   Mapped_Str3[i]=Mapped_Str2[i];  
           Mapped_Str3[i]=0;
         }
        //..............................................................................................................................................................                
        Len = strlen((char *)Mapped_Str3);

	SetCursorX(X_Start);
	SetCursorY(Y_Start);
        
//   printf(" ! %d-%d ) ",X_Start,Y_Start);
        i=0;
        while((c = Mapped_Str3[i++])!='\0')
    	     {
              if(c==255) //Enter
                {
                 SetCursorX(X_Start);
                 if(cursorY>=(Y_End+3))     cursorY = cursorY - (unsigned int)FontHeight * FontYScale; 
                                    
                else break;
               }
              else if(c==254) //Enter
                {
                 
                 if(Direction==L2R)      cursorX=X_Start+Mapped_Str3[i++];
                 else if(Direction==R2L) cursorX=X_Start-Mapped_Str3[i++];                 
               }
            
              else PutChar2(c,Direction);          
	}
  
}
#define DrawPixel(x,y)			LCD_PutPixel(x,y)
void Line(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2)
{
 signed int  x, y, addx, addy, dx, dy;
 signed long P;
 int i;
        
 #ifdef TFT35WIDE

 if(InLcdRect==0)
   {
    #if((DeviceType==BD90)||(DeviceType==VMODULE))
 
    #else
    x1+=25;
    y1+=36;
 
    x2+=25;
    y2+=36;
    #endif
 
    
    
    if((DeviceType==BCU)||(DeviceType==APARKINC))
      {
       x1+=ScreenShift;
       x2+=ScreenShift;
      }
   }
 #endif        
 
  #ifdef NEWHOMA
 if(DisTFT35Type==2)
   { 
 if(InLcdRect==0)
   {
    x1+=20;
    //y1+=36;
    x2+=20;
    //y2+=36;
    
    if((DeviceType==BCU)||(DeviceType==APARKINC))
      {
       x1+=ScreenShift;
       x2+=ScreenShift;
       y1+=ScreenYShift;
       y2+=ScreenYShift; 
      }
     }
   }
#endif
/*
	if (x1>x2)
	{
		dx = x1; x1 = x2; x2 = dx;
		dy = y1; y1 = y2; y2 = dy;
	}

	// vertical line
	if ((x1 == x2) || (y1 == y2))
	{
		FillRectangle(x1, y1, x2, y2);
		return;
	}
*/
	if(x2>x1)
          dx=x2-x1;
        else
          dx=x1-x2;
	if(y2>y1)
          dy=y2-y1;
        else
          dy=y1-y2;
	x = x1;
	y = y1;

	addx = addy = 1;
	if(x1 > x2)
		addx = -1;
	if(y1 > y2)
		addy = -1;

	if (dx >= dy)
	{
		P = 2*dy - dx;
		for (i=0; i<=dx; ++i)
		{
	  		DrawPixel(x, y);
			if(P < 0)
	  		{
	    		P += 2*dy;
	    		x += addx;
	  		}
			else
			{
	    		P += 2*dy - 2*dx;
	    		x += addx;
	    		y += addy;
	  		}
		}
	}
	else
	{
		P = 2*dx - dy;
		for(i=0; i<=dy; ++i)
		{
	  		DrawPixel(x, y);
	  		if(P < 0)
	  		{
	    		P += 2*dx;
	    		y += addy;
	  		}
	  		else
	  		{
	    		P += 2*dx - 2*dy;
	    		x += addx;
	    		y += addy;
	  		}
		}
	}
}
void LCDSetCur(unsigned int row,unsigned int col)
{               
	LCDSetCur24(row,col);
}
#define DisplayType TFT24
void LCD_Rect(unsigned int left, unsigned int top, unsigned int right, unsigned int bottom, unsigned int color)
{	
	 unsigned int x,y,b=0;
	 unsigned char D[480*3],Lineflag=0;//,DD[3];   
	 Lineflag=0;   
	 if(left>=480)    left=479;
	 if(right>=480)   right=479;
	 if(top>=800)     top=779;
	 if(bottom>=800)  bottom=830;       
	 if(left==right) Lineflag=1;
	 if(top==bottom) Lineflag=1;      
	 if(Lineflag)
	 {			
			Line(left,top, right , bottom);	
			return;
	 }
	 _color=color; 
	 LCDSetCur(0,0); 
	 LCD24_SetArea( top,left,bottom,right);
	 SET_RS
	 Lineflag = 0;
	 for(x=top; x<=bottom; x++)
	 {		   
			 b=0; 
			 for(y=left; y<=right; y++)            
			 {  
					 D[b++]=_color&0xff;
					 D[b++]=_color>>8;
					 D[b++]=_color>>16;	 							
			 }			 
			 CLR_CS
			 //if(Lineflag>5)
			 //{
				//  Lineflag = 0;
					DELAY
			 //}
			 //Lineflag++;
			 HAL_SPI_Transmit(&hspi1,D,b,100);	
			// DELAY
			 SET_CS	
       //return;			 
	 }
	 LCD24_SetArea(0,0,479,779); 
}

#define DrawRect(x1,y1,x2,y2)		LCD_Rect(x1, y1, x2, y2, GetColor)

void RectangleFill(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned int radius)
{
 signed int a, b, P,tt;
 signed int bx, ay;
        
        
	if (radius == 0)
	{
	    DrawRect(x1, y1, x2, y2);
      InLcdRect=0;
		return;
	}
	// adjust size
	x1 = x1 + radius;
	x2 = x2 - radius;
	y1 = y1 + radius;
	y2 = y2 - radius;

	a = 0;       // increment by 1
	b = radius;  // decrement by 1 using P
	P = 1 - radius;
	bx = b;
	ay = a;
	
	DrawRect(x1, y1-radius, x2, y2+radius);
	return;
        
  

	do
	{
		if (bx != b)
		{
			DrawRect(x1-bx, y1-ay, x1-bx, y2+ay); // left in
			DrawRect(x2+bx, y1-ay, x2+bx, y2+ay); // right in
			bx = b;
		}
  		ay = a;
		if (a != b)
		{ 
			DrawRect(x1-a, y1-b, x1, y2+b); // left out
			DrawRect(x2  , y1-b, x2+a, y2+b); // rigth out
  		}
		if(P < 0)
			P+= 3 + 2*a++;
		else
			P+= 5 + 2*(a++ - b--);
	} while(a <= b);

	if (bx != b)
	{
		DrawRect(x1-bx, y1-ay, x1-bx, y2+ay); // left in
		DrawRect(x2+bx, y1-ay, x2+bx, y2+ay); // right in
	}
         InLcdRect=0;
}

void Line2(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2)
{
 signed int  x, y, addx, addy, dx, dy;
 signed long P;
 int i;
 
 #ifdef COMPRESS 
 Line(x1,y1,x2,y2);
 
 #else
        
 #ifdef TFT35WIDE
 if(InLcdRect==0)
   {
    x1+=25;
    y1+=36;
    x2+=25;
    y2+=36;
 
    if((DeviceType==BCU)||(DeviceType==APARKINC))
      {
       x1+=ScreenShift;
       x2+=ScreenShift;
      }
   }
 #endif

 #ifdef NEWHOMA
 if(DisTFT35Type==2)
   { 
    if(InLcdRect==0)
      {
       x1+=20;
       // y1+=36;
       x2+=20;
       // y2+=36;
 
 if((DeviceType==BCU)||(DeviceType==APARKINC))
   {
    x1+=ScreenShift;
    x2+=ScreenShift;
    y1+=ScreenYShift;
    y2+=ScreenYShift; 
   }
  }
 }
#endif
 
 
 if(x2>x1)
   dx=x2-x1;
 else
   dx=x1-x2;

 if(y2>y1)
    dy=y2-y1;
 else
    dy=y1-y2;
 
 x = x1;
 y = y1;

	addx = addy = 1;
	if(x1 > x2)
		addx = -1;
	if(y1 > y2)
		addy = -1;

	if (dx >= dy)
	{
		P = 2*dy - dx;
		for (i=0; i<=dx; ++i)
		{
	  		DrawPixel(x+1, y);
                        DrawPixel(x-1, y);
                        DrawPixel(x, y+1);
                        DrawPixel(x, y-1);
                        DrawPixel(x+1, y+1);
                        DrawPixel(x+1, y-1);
                        DrawPixel(x-1, y+1);
                        DrawPixel(x-1, y-1);
                        DrawPixel(x, y);
			if(P < 0)
	  		{
	    		P += 2*dy;
	    		x += addx;
	  		}
			else
			{
	    		P += 2*dy - 2*dx;
	    		x += addx;
	    		y += addy;
	  		}
		}
	}
	else
	{
		P = 2*dx - dy;
		for(i=0; i<=dy; ++i)
		{
	  		DrawPixel(x+1, y);
                        DrawPixel(x-1, y);
                        DrawPixel(x, y+1);
                        DrawPixel(x, y-1);
                        DrawPixel(x+1, y+1);
                        DrawPixel(x+1, y-1);
                        DrawPixel(x-1, y+1);
                        DrawPixel(x-1, y-1);
                        DrawPixel(x, y);

	  		if(P < 0)
	  		{
	    		P += 2*dx;
	    		y += addy;
	  		}
	  		else
	  		{
	    		P += 2*dx - 2*dy;
	    		x += addx;
	    		y += addy;
	  		}
		}
	}
#endif
}


void RectangleFill2(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2,char Type)
{
 signed int a, b, P,tt;
 signed int bx, ay;

 #ifdef TFT35WIDE

 #if((DeviceType==BD90)||(DeviceType==VMODULE))
 
 #else
 x1+=25;
 y1+=36;
 
 x2+=25;
 y2+=36;
 #endif
 
 InLcdRect=1;
 
 #if((DeviceType==BCU)||(DeviceType==APARKINC))
 x1+=ScreenShift;
 x2+=ScreenShift;
 #endif
 
 #endif
 
 
 #ifdef NEWHOMA
 if(DisTFT35Type==2)
   { 
 x1+=20;
// y1+=36;
 

 x2+=20;
// y2+=36;
 
 InLcdRect=1;
 
 #if((DeviceType==BCU)||(DeviceType==APARKINC))
 x1+=ScreenShift;
 x2+=ScreenShift;
 y1+=ScreenYShift;
 y2+=ScreenYShift; 
 
 #endif
   }
#endif
 
        if(Type<2) 
          {
           DrawRect(x1+(x2-x1)/4, y1+(y2-y1)/4, x2-(x2-x1)/4, y2-(y2-y1)/4);
           


           for(a=1;a<8;a++)
              {
               DrawRect(x1+a*(x2-x1)/32, y1+((32/2-a)*(y2-y1))/32, x2-a*(x2-x1)/32, y2-((32/2-a)*(y2-y1))/32);
               DrawRect(x1+(16-a)*(x2-x1)/32, y1+((32/2-(16-a))*(y2-y1))/32, x2-(16-a)*(x2-x1)/32, y2-((32/2-(16-a))*(y2-y1))/32);
              }           
           
          // DrawRect(x1+2*(x2-x1)/16, y1+((16/2-2)*(y2-y1))/16, x2-2*(x2-x1)/16, y2-((16/2-2)*(y2-y1))/16);
           

          }

        b=0;
        for(a=0;a<5/*=((x2-x1)/4)*/;a+=4)
           {
            if(Type==0)
              {
               /*
               Line2(x1+a,y1+(y2-y1)/2,x1+(x2-x1)/4,y2-(y2-y1)/4-b);
               Line2(x1+(x2-x1)/4+a,y2-(y2-y1)/4,x1+(x2-x1)/2,y2-b);
               
               Line2(x1+(x2-x1)/2,y1+b,x2-(x2-x1)/4-a,y1+(y2-y1)/4);            
               Line2(x2-(x2-x1)/4,y1+(y2-y1)/4+b,x2-a,y1+(y2-y1)/2);
               
               Line2(x1+(x2-x1)/2,y2-b,x2-(x2-x1)/4-a,y2-(y2-y1)/4);
               Line2(x2-(x2-x1)/4,y2-(y2-y1)/4-b,x2-a,y1+(y2-y1)/2);
               
               Line2(x1+a,y1+(y2-y1)/2,x1+(x2-x1)/4,y1+(y2-y1)/4+b);
               Line2(x1+(x2-x1)/4+a,y1+(y2-y1)/4,x1+(x2-x1)/2,y1+b);
               */
              }
            else if(Type==1)
              {
               Line2(x1+a,y1+(y2-y1)/2,x1+(x2-x1)/2,y2-b);
               Line2(x1+(x2-x1)/2,y1+b,x2-a,y1+(y2-y1)/2);            
               Line2(x1+(x2-x1)/2,y2-b,x2-a,y1+(y2-y1)/2);
               Line2(x1+a,y1+(y2-y1)/2,x1+(x2-x1)/2,y1+b);
              }
            else if(Type==2)
              {
               Line(x1+a,y1+(y2-y1)/2,x1+(x2-x1)/2,y2-b);
               Line(x1+(x2-x1)/2,y1+b,x2-a,y1+(y2-y1)/2);            
               Line(x1+(x2-x1)/2,y2-b,x2-a,y1+(y2-y1)/2);
               Line(x1+a,y1+(y2-y1)/2,x1+(x2-x1)/2,y1+b);
              }
            
            b+=4;
            if(b>((y2-y1)/4)) break;
           }
       InLcdRect=0; 
        return;
        
        /*

        b=0;
        for(a=0;a<((x2-x1)/2);a+=2)
           {
            Line2(x1+(x2-x1)/2,y2-b,x2-a,y1+(y2-y1)/2);
            b+=2;
            if(b>((y2-y1)/2)) break;
           }
        

        b=0;
        for(a=0;a<((x2-x1)/2);a+=2)
           {
            Line2(x1+a,y1+(y2-y1)/2,x1+(x2-x1)/2,y1+b);
            b+=2;
            if(b>((y2-y1)/2)) break;
           }
        
       
        b=0;
        for(a=0;a<((x2-x1)/2);a+=2)
           {
            Line2(x1+(x2-x1)/2,y1+b,x2-a,y1+(y2-y1)/2);
            b+=2;
            if(b>((y2-y1)/2)) break;
           }
        */
        Line2(x1,y1+(y2-y1)/2,x1+(x2-x1)/2,y2);        
        Line2(x1+(x2-x1)/2,y2,x2,y1+(y2-y1)/2);
        Line2(x1,y1+(y2-y1)/2,x1+(x2-x1)/2,y1);
        Line2(x1+(x2-x1)/2,y1,x2,y1+(y2-y1)/2); 
        
        //HAL_Delay(5000);
				return ;
        
	do
	{
		if (bx != b)
		{
			DrawRect(x1-bx, y1-ay, x1-bx, y2+ay); // left in
			DrawRect(x2+bx, y1-ay, x2+bx, y2+ay); // right in
			bx = b;
		}
  		ay = a;
		if (a != b)
		{ 
			DrawRect(x1-a, y1-b, x1, y2+b); // left out
			DrawRect(x2  , y1-b, x2+a, y2+b); // rigth out
  		}
		if(P < 0)
			P+= 3 + 2*a++;
		else
			P+= 5 + 2*(a++ - b--);
	} while(a <= b);

	if (bx != b)
	{
		DrawRect(x1-bx, y1-ay, x1-bx, y2+ay); // left in
		DrawRect(x2+bx, y1-ay, x2+bx, y2+ay); // right in
	}
        
        InLcdRect=0;
}

void Rectangle(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned int radius)
{
	signed int a, b, P,tt;
	signed int bx, ay;
        
 #ifdef TFT35WIDE

 #if((DeviceType==BD90)||(DeviceType==VMODULE))
 
 #else
 x1+=25;
 y1+=36;
 
 x2+=25;
 y2+=36;
 #endif
 
    if((DeviceType==BCU)||(DeviceType==APARKINC))
 {
 x1+=ScreenShift;
 x2+=ScreenShift;
 }

 #endif     
 
 #ifdef NEWHOMA
 if(DisTFT35Type==2)
   { 
 x1+=20;
 //y1+=36;
 x2+=20;
 //y2+=36;
 
    if((DeviceType==BCU)||(DeviceType==APARKINC))
 {
 x1+=ScreenShift;
 x2+=ScreenShift;
 y1+=ScreenYShift;
 y2+=ScreenYShift; 
 
 }
   }
 #endif
        
 InLcdRect=1;
 /*
        if(DisplayType==TFT3)
          {
           tt=y1;
         
           y1=240-y2;
           y2=240-tt; 
           }
  */      
	// adjust size
	x1 = x1 + radius;
	x2 = x2 - radius;
	y1 = y1 + radius;
	y2 = y2 - radius;


	DrawRect(x1       , y1-radius, x2       , y1-radius);	// top
  DrawRect(x1       , y2+radius, x2       , y2+radius);	// bottom
	DrawRect(x1-radius, y1       , x1-radius, y2       );	// left
	DrawRect(x2+radius, y1       , x2+radius, y2       );	// right        

        

	if (radius == 0)	{ InLcdRect=0;return;}

	// draw broder
	a = 0;       // increment by 1
	b = radius;  // decrement by 1 using P
	P = 1 - radius;
	bx = b;
	ay = a;

	do
	{
		if (a && (a != b))
		{
			DrawPixel(x1-b, y1-a);
			DrawPixel(x1-b, y2+a);
			DrawPixel(x2+b, y1-a);
			DrawPixel(x2+b, y2+a);
		}
		DrawPixel(x1-a, y1-b);
		DrawPixel(x2+a, y1-b);
		DrawPixel(x1-a, y2+b);
		DrawPixel(x2+a, y2+b);

		if(P < 0)
			P+= 3 + 2*a++;
		else
			P+= 5 + 2*(a++ - b--);
	} while(a <= b);
        
         InLcdRect=0;
}
void Show_Image(unsigned char *Image,unsigned int Width,unsigned int Height,unsigned int X,unsigned int Y, unsigned int Color,char Mode)
{
 unsigned int xc, yc, sx, sy;
	unsigned int 	Image_Width	 	= 	Width;
	unsigned int 	Image_Height		=	Height;
	unsigned char 	byte 			= 	0;
	unsigned char 	maske 			= 	0;
	unsigned int 	xPos,yPos;
        unsigned int offset=0;        
        unsigned int i;
        unsigned char x=1;
     
        
#ifdef TFT35WIDE

 //InLcdRect=1;
 X+=25;
 Y+=36;

#endif

   cursorY=Y;
   cursorX=X;
   if(Mode==1)   cursorX+=Width;
        
   SetColor(Color);

   for(yc = 0; yc < Image_Height; yc++) // Eigentlcihe Ausgaberoutine
      {
	x=128;
        for(xc = 0; xc < Image_Width; xc++)
	   {
	    if((xc)&&(xc%8==0))
              {
               offset++;
               x=128;
              }
            byte=Image[offset];
            byte&=x;
            x>>=1;

       	    yPos =  cursorY-(unsigned int)yc;
            if(Mode==0) xPos = (((unsigned int)xc) + cursorX);
            else xPos = (cursorX-(unsigned int)xc);
  
            if(byte) LCD_PutPixel(xPos,yPos);
	   }
         offset++;
     }
}
void ProgressBar2(unsigned int X_Start,unsigned int Y_Start,unsigned int X_End,unsigned int Y_End, unsigned long int position, unsigned long int max,unsigned int Color)
{
unsigned char i, j, data;
unsigned long int loc;
float f;

  loc=position;
  loc*=(X_End-X_Start);
  loc/=max;   

  SetColor(WHITE);
  Rectangle(X_Start, Y_Start, X_End, Y_End, 5);
  SetColor(Color);
  if(loc>7)
    RectangleFill(X_Start+1, Y_Start+1, loc+X_Start+1, Y_End-1, 5);
}
#endif