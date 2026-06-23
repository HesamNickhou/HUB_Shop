#include "flash.h" 
//u16 SPI_FLASH_TYPE=W25Q64;
extern SPI_HandleTypeDef hspi2;
extern IWDG_HandleTypeDef hiwdg;
#define WDTR __HAL_IWDG_RELOAD_COUNTER(&hiwdg);
#define SPI_FLASH_CS(x) HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, x);

void FLASH_LOCK(void)
{
	SPI_FLASH_Write_Enable(); 
	SPI_FLASH_Write_SR(0x34); 
	HAL_Delay(100);
}
void FLASH_UNLOCK(void)
{
  SPI_FLASH_Write_Enable(); 
	SPI_FLASH_Write_SR(0);    
	HAL_Delay(100);
}
unsigned char SPI1_ReadWriteByte(unsigned char bytee)
{
	u8 send[1] = {bytee};
	u8 recv = 0;
  HAL_SPI_TransmitReceive(&hspi2,send,&recv,1,100);  
	return recv;
}
//=============================================================================
u8 SPI_Flash_ReadSR(void)   
{  
	u8 byte=0;  
  u8 send[1] = {W25X_ReadStatusReg};
	SPI_FLASH_CS(0)   
  HAL_SPI_TransmitReceive(&hspi2,send,&byte,1,100);
	send[0] = 0xff;
  HAL_SPI_TransmitReceive(&hspi2,send,&byte,1,100);	           
	SPI_FLASH_CS(1)                          
	return byte;   
} 

//=============================================================================
void SPI_FLASH_Write_SR(u8 sr)   
{   
  u8 send[1] = {W25X_WriteStatusReg};
	SPI_FLASH_CS(0)  
  HAL_SPI_Transmit(&hspi2,send,1,100);	
  send[0] = sr;	
	HAL_SPI_Transmit(&hspi2,send,1,100);                 
	SPI_FLASH_CS(1)                     
}   

//=============================================================================
void SPI_FLASH_Write_Enable(void)   
{
  u8 send[1] = {W25X_WriteEnable}, recv = 0;
	SPI_FLASH_CS(0)                     
	HAL_SPI_Transmit(&hspi2,send,1,100);
	SPI_FLASH_CS(1)                           
} 

//=============================================================================
void SPI_FLASH_Write_Disable(void)   
{  
  u8 send[1] = {W25X_WriteDisable} , recv = 0;
	SPI_FLASH_CS(0)
  HAL_SPI_Transmit(&hspi2,send,1,100);		
	SPI_FLASH_CS(1)                          
} 			    

//=============================================================================
u16 SPI_Flash_ReadID(void)
{
  u8 send[1] = {W25X_ManufactDeviceID}, recv = 0;
	u16 Temp = 0;	  
	SPI_FLASH_CS(0)	
	HAL_SPI_TransmitReceive(&hspi2,send,&recv,1,100);
	send[0] = 0x00;
	HAL_SPI_TransmitReceive(&hspi2,send,&recv,1,100); // dummy 
	HAL_SPI_TransmitReceive(&hspi2,send,&recv,1,100); // dummy
	HAL_SPI_TransmitReceive(&hspi2,send,&recv,1,100); // 
	HAL_SPI_TransmitReceive(&hspi2,send,&recv,1,100);  
	Temp = recv;
	HAL_SPI_TransmitReceive(&hspi2,send,&recv,1,100); 	 			   
	Temp = Temp << 8; 
  Temp = recv | Temp;	
	SPI_FLASH_CS(1)				    
	return Temp;
	
}   		    

//=============================================================================
void SPI_Flash_Read(u8* pBuffer,u32 ReadAddr,u16 NumByteToRead)   
{ 
  u8 send[1] = {W25X_ReadData} , recv = 0;
 	u16 i;    												    
	SPI_FLASH_CS(0)                         
  HAL_SPI_TransmitReceive(&hspi2,send,&recv,1,100);	
	
	send[0] = ReadAddr >>16 ;
	HAL_SPI_TransmitReceive(&hspi2,send,&recv,1,100);
	
  send[0] = ReadAddr >> 8 ;   
	HAL_SPI_TransmitReceive(&hspi2,send,&recv,1,100);
	
  send[0] = ReadAddr ;   
	HAL_SPI_TransmitReceive(&hspi2,send,&recv,1,100);
	
	send[0] = 0xff ;
  for(i=0;i<NumByteToRead;i++)
	{ 
		HAL_SPI_TransmitReceive(&hspi2,send,&pBuffer[i],1,100); 
  }
	SPI_FLASH_CS(1)                           
	
}  

//=============================================================================
void SPI_Flash_Write_Page(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)
{
  u8 send[1] = {W25X_PageProgram} , recv = 0;
 	u16 i;  
  SPI_FLASH_Write_Enable();               
	SPI_FLASH_CS(0)                           
  HAL_SPI_Transmit(&hspi2,send,1,100); //1
	
  send[0] = WriteAddr >> 16;
  HAL_SPI_Transmit(&hspi2,send,1,100); //2
  send[0] = WriteAddr >> 8;
  HAL_SPI_Transmit(&hspi2,send,1,100);  //3
  send[0] = WriteAddr ;
  HAL_SPI_Transmit(&hspi2,send,1,100); //4
    
  for(i=0;i<NumByteToWrite;i++) 
	{
		send[0] = pBuffer[i];
		HAL_SPI_Transmit(&hspi2,&pBuffer[i],1,100); 

	}
	SPI_FLASH_CS(1)                   
	SPI_Flash_Wait_Busy();					  

 
} 

//=============================================================================
void SPI_Flash_PowerDown(void)   
{ 
 
 	SPI_FLASH_CS(0)	
  SPI1_ReadWriteByte(W25X_PowerDown);    
	SPI_FLASH_CS(1)                          
  HAL_Delay(3);                           

  
}   

//=============================================================================
void SPI_Flash_WAKEUP(void)   
{  

	SPI_FLASH_CS(0)                        
  SPI1_ReadWriteByte(W25X_ReleasePowerDown); 
	SPI_FLASH_CS(1)                           
  HAL_Delay(3);                            
	
}   

//=============================================================================
void SPI_Flash_Erase_Chip(void)   
{                                             
  u8 send[1] = {W25X_ChipErase};
	u8 recv = 0;
  SPI_FLASH_Write_Enable(); 
  SPI_Flash_Wait_Busy();   
 	SPI_FLASH_CS(0)

  HAL_SPI_Transmit(&hspi2,send,1,100);	

	SPI_FLASH_CS(1)                      
	SPI_Flash_Wait_Busy();   				  

}   

//=============================================================================
void SPI_Flash_Erase_Sector(u32 Dst_Addr)   
{   
  u8 send[1] = {W25X_SectorErase};
	u8 recv = 0;
	Dst_Addr*=4096;
  SPI_FLASH_Write_Enable();       
  SPI_Flash_Wait_Busy();   
	SPI_FLASH_CS(0)  
  	

  HAL_SPI_Transmit(&hspi2,send,1,100); 
	
	send[0] = Dst_Addr >> 16;
	HAL_SPI_Transmit(&hspi2,send,1,100);
	
	send[0] = Dst_Addr >> 8;
	HAL_SPI_Transmit(&hspi2,send,1,100);
	
	send[0] = Dst_Addr;
	HAL_SPI_Transmit(&hspi2,send,1,100);
	
	
	SPI_FLASH_CS(1)                          
  SPI_Flash_Wait_Busy();   				

  
}  

//=============================================================================
void SPI_Flash_Write_NoCheck(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)   
{ 			 		 
	u16 pageremain;	   
	pageremain=256-WriteAddr%256; 
	if(NumByteToWrite<=pageremain)pageremain=NumByteToWrite;
	while(1)
	{	 
    WDTR		
		SPI_Flash_Write_Page(pBuffer,WriteAddr,pageremain);
		if(NumByteToWrite==pageremain)break;
	 	else 
		{
			pBuffer+=pageremain;
			WriteAddr+=pageremain;	

			NumByteToWrite-=pageremain;			 
			if(NumByteToWrite>256)pageremain=256;
			else pageremain=NumByteToWrite; 	  
		}
	}	    
} 

//=============================================================================
volatile unsigned char SPI_FLASH_BUF[4096];
u8 SPI_Flash_Write(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)   
{ 
u32 secpos;
u16 secoff;
u16 secremain;	   
u16 i;    
	
	secpos=WriteAddr/4096; // 15.05
	secoff=WriteAddr%4096; //240
	secremain=4096-secoff; // 3856
  FLASH_UNLOCK();
	if((secoff==0)&&(secpos!=0))
	{
		SPI_Flash_Erase_Sector(secpos);
	}
	
	if(NumByteToWrite<=secremain)secremain=NumByteToWrite;

	while(1) 
	{
		WDTR
		SPI_Flash_Read(SPI_FLASH_BUF,secpos*4096,4096);
		for(i=0;i<secremain;i++)
		{
			if(SPI_FLASH_BUF[secoff+i]!=0XFF)break;
		}
		if(i<secremain)
		{
			FLASH_UNLOCK();
			SPI_Flash_Erase_Sector(secpos);
			for(i=0;i<secremain;i++)	  
			{
				SPI_FLASH_BUF[i+secoff]=pBuffer[i];	  
			}
			SPI_Flash_Write_NoCheck(SPI_FLASH_BUF,secpos*4096,4096);

		}else SPI_Flash_Write_NoCheck(pBuffer,WriteAddr,secremain);


		for(i=0; i<secremain; i++)	  
			SPI_FLASH_BUF[i]=0;	  
		SPI_Flash_Read(SPI_FLASH_BUF,WriteAddr,secremain);
		for(i=0; i<secremain; i++)	  
		  if(SPI_FLASH_BUF[i]!=pBuffer[i])
				break;
    if(i<secremain)
      return(1);			

		if(NumByteToWrite==secremain)break;

		secpos++;
		secoff=0;

   	pBuffer+=secremain;
		WriteAddr+=secremain;
   	NumByteToWrite-=secremain;
		if(NumByteToWrite>4096)secremain=4096;
		else secremain=NumByteToWrite;			

	};	 	 

	return(0);
}

u8 SPI_Flash_Write2(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)   
{ 
u32 secpos;
u16 secoff;
u16 secremain;	   
u16 i;    
	
	secpos=WriteAddr/4096;
	secoff=WriteAddr%4096;
	secremain=4096-secoff;

	//if((secoff==0)&&(secpos!=0))
	//	SPI_Flash_Erase_Sector(secpos);
	
	if(NumByteToWrite<=secremain)secremain=NumByteToWrite;

	while(1) 
	{	
		SPI_Flash_Read(SPI_FLASH_BUF,secpos*4096,4096);
		for(i=0;i<secremain;i++)
		{
			if(SPI_FLASH_BUF[secoff+i]!=0XFF)break;
		}
		if(i<secremain)
		{
			SPI_Flash_Erase_Sector(secpos);
			for(i=0;i<secremain;i++)	  
			{
				SPI_FLASH_BUF[i+secoff]=pBuffer[i];	  
			}
			SPI_Flash_Write_NoCheck(SPI_FLASH_BUF,secpos*4096,4096);

		}else SPI_Flash_Write_NoCheck(pBuffer,WriteAddr,secremain);


		for(i=0; i<secremain; i++)	  
			SPI_FLASH_BUF[i]=0;	  
		SPI_Flash_Read(SPI_FLASH_BUF,WriteAddr,secremain);
		for(i=0; i<secremain; i++)	  
		  if(SPI_FLASH_BUF[i]!=pBuffer[i])
				break;
    if(i<secremain)
      return(1);			

		if(NumByteToWrite==secremain)break;

		secpos++;
		secoff=0;

   	pBuffer+=secremain;
		WriteAddr+=secremain;
   	NumByteToWrite-=secremain;
		if(NumByteToWrite>4096)secremain=4096;
		else secremain=NumByteToWrite;			

	};	 	 

	return(0);
}

//=============================================================================
void SPI_Flash_Wait_Busy(void)   
{   
unsigned long int i=0;
	
	while ((SPI_Flash_ReadSR()&0x01)==0x01)
	{
		WDTR;
	}
}  
//=======================
void SPI_Flash_Enable_Reset(void)
{
  u8 send[1] = {W25X_ENABLERESET}, recv = 0;
	SPI_FLASH_CS(0)                     
	HAL_SPI_Transmit(&hspi2,send,1,100);
	SPI_FLASH_CS(1)                           
} 
//==============================================
void SPI_Flash_Reset_Device(void)
{
  u8 send[1] = {W25X_RESETDEVICE}, recv = 0;
	SPI_Flash_Enable_Reset();
	SPI_FLASH_CS(0)                     
	HAL_SPI_Transmit(&hspi2,send,1,100);
	SPI_FLASH_CS(1)                           
} 


void SPI_Flash_GlobalUnlock(void)
{
  u8 send[1] = {W25X_GlobalUnlock};
	SPI_FLASH_CS(0)                     
	HAL_SPI_Transmit(&hspi2,send,1,100);
	SPI_FLASH_CS(1)                           
}























