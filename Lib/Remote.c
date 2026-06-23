#include "main.h"

#ifdef remote
extern TIM_HandleTypeDef htim2;
extern unsigned int Tm,Time,HTime,LTime;
#define RDATA   HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_8)
extern unsigned char isZero(void);
extern unsigned char isSync(void);
extern unsigned char isOne(void);
extern void FinalAnalyse(void);
extern unsigned char DetectTruePacket;
extern volatile unsigned char Tmp[24],bufer[12];
extern unsigned char RemotePress,press;
extern volatile unsigned int learncode;

//extern unsigned int shiftDate;

//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
//{

//}
unsigned char isSync(void)
{
  if(HTime<LTime)
    if(HTime>100 && HTime<2000)
      if(LTime>5000 && LTime<15000)
		  {
        return(1);            
		  }
  return(0);     
}
//----------------------
unsigned char isZero(void)
{
  if(HTime<LTime)
	{
    return(1); 
	}			           
  return(0);     
}
//----------------------------
unsigned char isOne(void)
{
  if(HTime>LTime)
	{
    return(1);            
	}
  return(0);     
}
//---------------------------
void FinalAnalyse(void)
{

	static volatile unsigned char i = 0;	
//	for(i=0;i<12;i++)
//	{
//    if(Tmp[i*2]==0 && Tmp[(i*2)+1]==0)
//		{
//      bufer[i]='0';
//    }
//    else if(Tmp[i*2]==1 && Tmp[(i*2)+1]==1)
//		{             
//      bufer[i]='1';    
//    }
//    else
//		{
//      bufer[i]='F';        
//    }
//  }; 
	
//  bufer[12]=0; // terminate string 
//	valid = 1;
//  printf("\n\rDetect");
//  for(i=0; i<12; i++)
 //   printf("%c", bufer[i]);
  learncode = 0;
//	shiftDate = 0;
	if( (Tmp[23] == 0) && (Tmp[22] == 0) && (Tmp[21] == 0) && (Tmp[20] == 0))
			return ;
	else DetectTruePacket = 1; 
	for(i=0;i<20;i++)
	{
		learncode += Tmp[i] << (19-i);
		//learncode = shiftDate | learncode ;
	}

	press = 0;
	
	for(i=20;i<24;i++)
	{
		press |= Tmp[i] << (23-i);
		//press = shiftDate | press ;
	}
//  for(i=0; i<8; i++)
//    LearnCode[i]=bufer[i];
//	
//  for(i=0; i<12; i++)
//    if(OPEN[i]!=bufer[i])
//		  break;
//  if(i>=12)
//  {
//    RemotePress=RK_OPEN;	 
//	  //printf("\n\rRELAY_OPEN");
//  }
//  for(i=0; i<12; i++)
//    if(CLOSE[i]!=bufer[i])
//		  break;
//  if(i>=12)
//  {
//    //RELAY_CLOSE;	
//    RemotePress=RK_CLOSE;
//	  //printf("\n\rRELAY_CLOSE");
//  }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == GPIO_PIN_8)
  {
		static volatile unsigned char bitCnt=0,byte=0;    
		Tm=TIM2->CNT;
		TIM2->CNT=0;
		Tm+=Time;
		Time=0;			
		if(GPIOB->IDR & (1<<8)) //READ GPIO B8 
		{
			LTime=Tm;
			if(isSync())
			{
				if(bitCnt==24)
				{								
					FinalAnalyse();	        				
				}
				bitCnt=0;
			} 
			else if(isZero())
			{
					Tmp[bitCnt++]=0;
			} 
			else if(isOne())
			{
					Tmp[bitCnt++]=1;         
			}else{
					bitCnt=0;
			}
		}else{       
			HTime=Tm;
		}
		
	__HAL_GPIO_EXTI_CLEAR_IT(EXTI_LINE_8);
//	EXTI->PR = 0x00100 ;		
     // ask_pinchange_callback(&ask433);
  }
}
#endif