/** \file max30102.cpp ******************************************************
 *
 * Project: MAXREFDES117#
 * Filename: max30102.cpp
 * Description: This module is an embedded controller driver for the MAX30102
 *
 *
 * --------------------------------------------------------------------
 *
 * This code follows the following naming conventions:
 *
 * char              ch_pmod_value
 * char (array)      s_pmod_s_string[16]
 * float             f_pmod_value
 * int32_t           n_pmod_value
 * int32_t (array)   an_pmod_value[16]
 * int16_t           w_pmod_value
 * int16_t (array)   aw_pmod_value[16]
 * uint16_t          uw_pmod_value
 * uint16_t (array)  auw_pmod_value[16]
 * uint8_t           uch_pmod_value
 * uint8_t (array)   auch_pmod_buffer[16]
 * uint32_t          un_pmod_value
 * int32_t *         pn_pmod_value
 *
 * ------------------------------------------------------------------------- */
/*******************************************************************************
 * Copyright (C) 2016 Maxim Integrated Products, Inc., All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY CLAIM, DAMAGES
 * OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of Maxim Integrated
 * Products, Inc. shall not be used except as stated in the Maxim Integrated
 * Products, Inc. Branding Policy.
 *
 * The mere transfer of this software does not imply any licenses
 * of trade secrets, proprietary technology, copyrights, patents,
 * trademarks, maskwork rights, or any other form of intellectual
 * property whatsoever. Maxim Integrated Products, Inc. retains all
 * ownership rights.
 *******************************************************************************
 *  Modified original MAXIM source code on: 13.01.2019
 *		Author: Mateusz Salamon
 *		www.msalamon.pl
 *		mateusz@msalamon.pl
 *	Code is modified to work with STM32 HAL libraries.
 *
 *	Website: https://msalamon.pl/palec-mi-pulsuje-pulsometr-max30102-pod-kontrola-stm32/
 *	GitHub:  https://github.com/lamik/MAX30102_STM32_HAL
 *
 */
#include "main.h"
//#include "i2c.h"
#include "define.h"
#include "MAX30102.h"
#include "algorithm.h"
#define editII
extern void send(unsigned char *buf);
extern void Log(unsigned char *buf,unsigned int i);
#ifdef editI

#include "algorithm.h"

#define I2C_TIMEOUT	1


extern I2C_HandleTypeDef *i2c_max30102;



volatile uint32_t IrBuffer[MAX30102_BUFFER_LENGTH]={0}; //IR LED sensor data
volatile uint32_t RedBuffer[MAX30102_BUFFER_LENGTH]={0};    //Red LED sensor data
volatile uint32_t BufferHead=0;
volatile uint32_t BufferTail=0;
volatile uint32_t CollectedSamples=0;
volatile uint8_t IsFingerOnScreen=0;
int32_t Sp02Value=0;
int8_t Sp02IsValid=0;
int32_t HeartRate=0;
int8_t IsHrValid=0;


typedef enum
{
  MAX30102_STATE_BEGIN,
  MAX30102_STATE_CALIBRATE,
  MAX30102_STATE_CALCULATE_HR,
  MAX30102_STATE_COLLECT_NEXT_PORTION
} MAX30102_STATE;

MAX30102_STATE StateMachine;

MAX30102_STATUS Max30102_WriteReg(uint8_t uch_addr, uint8_t uch_data)
{
  if (HAL_I2C_Mem_Write(i2c_max30102, MAX30102_ADDRESS, uch_addr, 1, &uch_data,
                        1, I2C_TIMEOUT)
      == HAL_OK)
    return MAX30102_OK;
  return MAX30102_ERROR;
}

MAX30102_STATUS Max30102_ReadReg(uint8_t uch_addr, uint8_t *puch_data)
{
  if (HAL_I2C_Mem_Read(i2c_max30102, MAX30102_ADDRESS, uch_addr, 1, puch_data,
                       1, I2C_TIMEOUT)
      == HAL_OK)
    return MAX30102_OK;
  return MAX30102_ERROR;
}

MAX30102_STATUS Max30102_WriteRegisterBit(uint8_t Register, uint8_t Bit,
                                          uint8_t Value)
{
  uint8_t tmp;
  if (MAX30102_OK != Max30102_ReadReg(Register, &tmp))
    return MAX30102_ERROR;
  tmp &= ~(1 << Bit);
  tmp |= (Value & 0x01) << Bit;
  if (MAX30102_OK != Max30102_WriteReg(Register, tmp))
    return MAX30102_ERROR;

  return MAX30102_OK;
}

MAX30102_STATUS Max30102_ReadFifo(volatile uint32_t *pun_red_led,
                                  volatile uint32_t *pun_ir_led)
{
  uint32_t un_temp;
  *pun_red_led = 0;
  *pun_ir_led = 0;
  uint8_t ach_i2c_data[6];
	uint8_t buf[40];

  if (HAL_I2C_Mem_Read(i2c_max30102, MAX30102_ADDRESS, REG_FIFO_DATA, 1,
                       ach_i2c_data, 6, I2C_TIMEOUT)
      != HAL_OK)
  {
    return MAX30102_ERROR;
  }
  un_temp = ach_i2c_data[0];
  un_temp <<= 16;
  *pun_red_led += un_temp;
  un_temp = ach_i2c_data[1];
  un_temp <<= 8;
  *pun_red_led += un_temp;
  un_temp = ach_i2c_data[2];
  *pun_red_led += un_temp;

  un_temp = ach_i2c_data[3];
  un_temp <<= 16;
  *pun_ir_led += un_temp;
  un_temp = ach_i2c_data[4];
  un_temp <<= 8;
  *pun_ir_led += un_temp;
  un_temp = ach_i2c_data[5];
  *pun_ir_led += un_temp;
  *pun_red_led &= 0x03FFFF;  //Mask MSB [23:18]
  *pun_ir_led  &= 0x03FFFF;  //Mask MSB [23:18]
	
	
	if(*pun_ir_led>110000)
	{
//		if(cntt<sample)
//			avrIr += *pun_ir_led;
//		
//		avrIrBuf[cntt++] = *pun_ir_led;
//		
//		if(cntt>=sample) 
//		{
//			avrIr = avrIr / sample;
//			cntt = 0;
//			for(unsigned short cnt = 0; cnt < sample ; cnt++)
//			{
				//sprintf(buf,"{\"M\":{\"Smple\":%ld}}\n",*pun_ir_led);
	    	sprintf(buf,"%ld ",*pun_ir_led);
				send(buf);
//			}
//		}		
////		sprintf(buf,"%ld  ",*pun_ir_led);
////		send(buf);
//	}
//	
//	if(*pun_ir_led<=10000) 
//	{
//		cntt =  0;
//		avrIr = 0;
	}

  return MAX30102_OK;
}

//
//	Interrupts
//
MAX30102_STATUS Max30102_SetIntAlmostFullEnabled(uint8_t Enable)
{
  return Max30102_WriteRegisterBit(REG_INTR_ENABLE_1, INT_A_FULL_BIT, Enable);
}

MAX30102_STATUS Max30102_SetIntFifoDataReadyEnabled(uint8_t Enable)
{

  return Max30102_WriteRegisterBit(REG_INTR_ENABLE_1, INT_PPG_RDY_BIT, Enable);
}

MAX30102_STATUS Max30102_SetIntAmbientLightCancelationOvfEnabled(uint8_t Enable)
{

  return Max30102_WriteRegisterBit(REG_INTR_ENABLE_2, INT_ALC_OVF_BIT, Enable);
}
#ifdef MAX30102_USE_INTERNAL_TEMPERATURE
MAX30102_STATUS Max30102_SetIntInternalTemperatureReadyEnabled(uint8_t Enable)
{

  return Max30102_WriteRegisterBit(REG_INTR_ENABLE_2, INT_DIE_TEMP_RDY_BIT, Enable);
}
#endif
MAX30102_STATUS Max30102_ReadInterruptStatus(uint8_t *Status)
{
  uint8_t tmp;
  *Status = 0;

  if (MAX30102_OK != Max30102_ReadReg(REG_INTR_STATUS_1, &tmp))
    return MAX30102_ERROR;
  *Status |= tmp & 0xE1; // 3 highest bits
#ifdef MAX30102_USE_INTERNAL_TEMPERATURE
      if(MAX30102_OK != Max30102_ReadReg(REG_INTR_STATUS_2, &tmp))
      return MAX30102_ERROR;
      *Status |= tmp & 0x02;
#endif
  return MAX30102_OK;
}

void Max30102_InterruptCallback(void)
{
  uint8_t Status;
  while (MAX30102_OK != Max30102_ReadInterruptStatus(&Status))
    ;

  // Almost Full FIFO Interrupt handle
  if (Status & (1 << INT_A_FULL_BIT))
  {
    for (uint8_t i = 0; i < MAX30102_FIFO_ALMOST_FULL_SAMPLES; i++)
    {
      while (MAX30102_OK
          != Max30102_ReadFifo((RedBuffer + BufferHead),
                               (IrBuffer + BufferHead)))
        ;
      if (IsFingerOnScreen)
      {
        if (IrBuffer[BufferHead] < MAX30102_IR_VALUE_FINGER_OUT_SENSOR)
          IsFingerOnScreen = 0;
      }
      else
      {
        if (IrBuffer[BufferHead] > MAX30102_IR_VALUE_FINGER_ON_SENSOR)
          IsFingerOnScreen = 1;
      }
      BufferHead = (BufferHead + 1) % MAX30102_BUFFER_LENGTH;
//			if(BufferHead >= MAX30102_BUFFER_LENGTH)  BufferHead = 0;
      CollectedSamples++;
    }
  }

  // New FIFO Data Ready Interrupt handle
  if (Status & (1 << INT_PPG_RDY_BIT))
  {
    while (MAX30102_OK
        != Max30102_ReadFifo((RedBuffer + BufferHead), (IrBuffer + BufferHead)))
      ;
    if (IsFingerOnScreen)
    {
      if (IrBuffer[BufferHead] < MAX30102_IR_VALUE_FINGER_OUT_SENSOR)
        IsFingerOnScreen = 0;
    }
    else
    {
      if (IrBuffer[BufferHead] > MAX30102_IR_VALUE_FINGER_ON_SENSOR)
        IsFingerOnScreen = 1;
    }
    BufferHead = (BufferHead + 1) % MAX30102_BUFFER_LENGTH;
//		if(BufferHead >= MAX30102_BUFFER_LENGTH)  BufferHead = 0;
    CollectedSamples++;
  }

  //  Ambient Light Cancellation Overflow Interrupt handle
  if (Status & (1 << INT_ALC_OVF_BIT))
  {

  }

  // Power Ready Interrupt handle
  if (Status & (1 << INT_PWR_RDY_BIT))
  {
  }
#ifdef MAX30102_USE_INTERNAL_TEMPERATURE
  // Internal Temperature Ready Interrupt handle
  if(Status & (1<<INT_DIE_TEMP_RDY_BIT))
  {

  }
#endif
}

//
//	FIFO Configuration
//
MAX30102_STATUS Max30102_FifoWritePointer(uint8_t Address)
{
  if (MAX30102_OK != Max30102_WriteReg(REG_FIFO_WR_PTR, (Address & 0x1F))) //FIFO_WR_PTR[4:0]
    return MAX30102_ERROR;
  return MAX30102_OK;
}

MAX30102_STATUS Max30102_FifoOverflowCounter(uint8_t Address)
{
  if (MAX30102_OK != Max30102_WriteReg(REG_OVF_COUNTER, (Address & 0x1F))) //OVF_COUNTER[4:0]
    return MAX30102_ERROR;
  return MAX30102_OK;
}

MAX30102_STATUS Max30102_FifoReadPointer(uint8_t Address)
{
  if (MAX30102_OK != Max30102_WriteReg(REG_FIFO_RD_PTR, (Address & 0x1F))) //FIFO_RD_PTR[4:0]
    return MAX30102_ERROR;
  return MAX30102_OK;
}

MAX30102_STATUS Max30102_FifoSampleAveraging(uint8_t Value)
{
  uint8_t tmp;
  if (MAX30102_OK != Max30102_ReadReg(REG_FIFO_CONFIG, &tmp))
    return MAX30102_ERROR;
  tmp &= ~(0x07);
  tmp |= (Value & 0x07) << 5;
  if (MAX30102_OK != Max30102_WriteReg(REG_FIFO_CONFIG, tmp))
    return MAX30102_ERROR;

  return MAX30102_OK;
}

MAX30102_STATUS Max30102_FifoRolloverEnable(uint8_t Enable)
{
  return Max30102_WriteRegisterBit(REG_FIFO_CONFIG,
  FIFO_CONF_FIFO_ROLLOVER_EN_BIT,
                                   (Enable & 0x01));
}

MAX30102_STATUS Max30102_FifoAlmostFullValue(uint8_t Value)
{
  if (Value < 17)
    Value = 17;
  if (Value > 32)
    Value = 32;
  Value = 32 - Value;
  uint8_t tmp;
  if (MAX30102_OK != Max30102_ReadReg(REG_FIFO_CONFIG, &tmp))
    return MAX30102_ERROR;
  tmp &= ~(0x0F);
  tmp |= (Value & 0x0F);
  if (MAX30102_OK != Max30102_WriteReg(REG_FIFO_CONFIG, tmp))
    return MAX30102_ERROR;

  return MAX30102_OK;
}
//
//	Mode Configuration
//
MAX30102_STATUS Max30102_ShutdownMode(uint8_t Enable)
{
  return Max30102_WriteRegisterBit(REG_MODE_CONFIG, MODE_SHDN_BIT,
                                   (Enable & 0x01));
}

MAX30102_STATUS Max30102_Reset(void)
{
  uint8_t tmp = 0xFF;
  if (MAX30102_OK != Max30102_WriteReg(REG_MODE_CONFIG, 0x40))
    return MAX30102_ERROR;
  do
  {
    if (MAX30102_OK != Max30102_ReadReg(REG_MODE_CONFIG, &tmp))
      return MAX30102_ERROR;
  } while (tmp & (1 << 6));

  return MAX30102_OK;
}

MAX30102_STATUS Max30102_SetMode(uint8_t Mode)
{
  uint8_t tmp;
  if (MAX30102_OK != Max30102_ReadReg(REG_MODE_CONFIG, &tmp))
    return MAX30102_ERROR;
  tmp &= ~(0x07);
  tmp |= (Mode & 0x07);
  if (MAX30102_OK != Max30102_WriteReg(REG_MODE_CONFIG, tmp))
    return MAX30102_ERROR;

  return MAX30102_OK;
}
//
//	SpO2 Configuration
//
MAX30102_STATUS Max30102_SpO2AdcRange(uint8_t Value)
{
  uint8_t tmp;
  if (MAX30102_OK != Max30102_ReadReg(REG_SPO2_CONFIG, &tmp))
    return MAX30102_ERROR;
  tmp &= ~(0x03);
  tmp |= ((Value & 0x03) << 5);
  if (MAX30102_OK != Max30102_WriteReg(REG_SPO2_CONFIG, tmp))
    return MAX30102_ERROR;

  return MAX30102_OK;
}

MAX30102_STATUS Max30102_SpO2SampleRate(uint8_t Value)
{
  uint8_t tmp;
  if (MAX30102_OK != Max30102_ReadReg(REG_SPO2_CONFIG, &tmp))
    return MAX30102_ERROR;
  tmp &= ~(0x07);
  tmp |= ((Value & 0x07) << 2);
  if (MAX30102_OK != Max30102_WriteReg(REG_SPO2_CONFIG, tmp))
    return MAX30102_ERROR;

  return MAX30102_OK;
}

MAX30102_STATUS Max30102_SpO2LedPulseWidth(uint8_t Value)
{
  uint8_t tmp;
  if (MAX30102_OK != Max30102_ReadReg(REG_SPO2_CONFIG, &tmp))
    return MAX30102_ERROR;
  tmp &= ~(0x03);
  tmp |= (Value & 0x03);
  if (MAX30102_OK != Max30102_WriteReg(REG_SPO2_CONFIG, tmp))
    return MAX30102_ERROR;

  return MAX30102_OK;
}

//
//	LEDs Pulse Amplitute Configuration
//	LED Current = Value * 0.2 mA
//
MAX30102_STATUS Max30102_Led1PulseAmplitude(uint8_t Value)
{
  if (MAX30102_OK != Max30102_WriteReg(REG_LED1_PA, Value))
    return MAX30102_ERROR;
  return MAX30102_OK;
}

MAX30102_STATUS Max30102_Led2PulseAmplitude(uint8_t Value)
{
  if (MAX30102_OK != Max30102_WriteReg(REG_LED2_PA, Value))
    return MAX30102_ERROR;
  return MAX30102_OK;
}

//
//	Usage functions
//
uint8_t Max30102_IsFingerOnSensor(void)
{
  return IsFingerOnScreen;
}

int32_t Max30102_GetHeartRate(void)
{
  if (IsHrValid)
    return HeartRate;
  return 0;
}

int32_t Max30102_GetSpO2Value(void)
{
  if (Sp02IsValid)
    return Sp02Value;
  return 0;
}

void Max30102_Task(void)
{
	unsigned char buff[40];
	unsigned int avr = 0;
	unsigned short sample = 0;
  switch (StateMachine)
  {
  case MAX30102_STATE_BEGIN:
    HeartRate = 0;
    Sp02Value = 0;
    if (IsFingerOnScreen)
    {
      CollectedSamples = 0;
      BufferTail = BufferHead;
      Max30102_Led1PulseAmplitude(MAX30102_RED_LED_CURRENT_HIGH);
      Max30102_Led2PulseAmplitude(MAX30102_IR_LED_CURRENT_HIGH);
      StateMachine = MAX30102_STATE_CALIBRATE;
    }
    break;

  case MAX30102_STATE_CALIBRATE:
    if (IsFingerOnScreen)
    {
      if (CollectedSamples > (MAX30102_BUFFER_LENGTH
          - MAX30102_SAMPLES_PER_SECOND))
      {
        StateMachine = MAX30102_STATE_CALCULATE_HR;
      }
    }
    else
    {
      Max30102_Led1PulseAmplitude(MAX30102_RED_LED_CURRENT_LOW);
      Max30102_Led2PulseAmplitude(MAX30102_IR_LED_CURRENT_LOW);
      StateMachine = MAX30102_STATE_BEGIN;
    }
    break;

  case MAX30102_STATE_CALCULATE_HR:
    if (IsFingerOnScreen)
    {
//			send("\n========IR======\n");
////			for(unsigned short cnt = 0 ;cnt<MAX30102_BUFFER_LENGTH;cnt++) avr +=IrBuffer[cnt]; 
////			avr = avr / MAX30102_BUFFER_LENGTH ;
//			for(unsigned short cnt = MAX30102_SAMPLES_PER_SECOND ;cnt<MAX30102_BUFFER_LENGTH ;cnt++)			
//				if(IrBuffer[cnt] >= 100000)
//				{
//					sample++;
//					avr +=IrBuffer[cnt]; 
//				}				
//			for(unsigned short cnnt = MAX30102_SAMPLES_PER_SECOND ;cnnt<MAX30102_BUFFER_LENGTH  ;cnnt++)
//				if(IrBuffer[cnnt] >= 100000)
//				{	
//					avr = avr / sample ;
//					if(sample>=MAX30102_BUFFER_LENGTH-MAX30102_SAMPLES_PER_SECOND-1)
//					{
//						sprintf(buff,"%ld  ",IrBuffer[cnnt] - avr);
//						send(buff);
//					}
//				}
//			send("========REd======\n");
//			for(unsigned short cnt =0;cnt<MAX30102_BUFFER_LENGTH;cnt++)
//			{
//				sprintf(buff,"%ld  ",RedBuffer[cnt]);
//				send(buff);				
//			}
      maxim_heart_rate_and_oxygen_saturation(IrBuffer,RedBuffer,
      MAX30102_BUFFER_LENGTH - MAX30102_SAMPLES_PER_SECOND,
                                             BufferTail, &Sp02Value,
                                             &Sp02IsValid, &HeartRate,
                                             &IsHrValid);
      BufferTail = (BufferTail + MAX30102_SAMPLES_PER_SECOND)
          % MAX30102_BUFFER_LENGTH;
      CollectedSamples = 0;
      StateMachine = MAX30102_STATE_COLLECT_NEXT_PORTION;
    }
    else
    {
      Max30102_Led1PulseAmplitude(MAX30102_RED_LED_CURRENT_LOW);
      Max30102_Led2PulseAmplitude(MAX30102_IR_LED_CURRENT_LOW);
      StateMachine = MAX30102_STATE_BEGIN;
    }
    break;

  case MAX30102_STATE_COLLECT_NEXT_PORTION:
    if (IsFingerOnScreen)
    {
      if (CollectedSamples > MAX30102_SAMPLES_PER_SECOND)
      {
        StateMachine = MAX30102_STATE_CALCULATE_HR;
      }
    }
    else
    {
      Max30102_Led1PulseAmplitude(MAX30102_RED_LED_CURRENT_LOW);
      Max30102_Led2PulseAmplitude(MAX30102_IR_LED_CURRENT_LOW);
      StateMachine = MAX30102_STATE_BEGIN;
    }
    break;
  }
}

//
//	Initialization
//
MAX30102_STATUS Max30102_Init(I2C_HandleTypeDef *i2c)
{
  uint8_t uch_dummy;
  i2c_max30102 = i2c;
  if (MAX30102_OK != Max30102_Reset()) //resets the MAX30102
    return MAX30102_ERROR;
  if (MAX30102_OK != Max30102_ReadReg(0, &uch_dummy))
    return MAX30102_ERROR;
  if (MAX30102_OK != Max30102_FifoWritePointer(0x00))
    return MAX30102_ERROR;
  if (MAX30102_OK != Max30102_FifoOverflowCounter(0x00))
    return MAX30102_ERROR;
  if (MAX30102_OK != Max30102_FifoReadPointer(0x00))
    return MAX30102_ERROR;
  if (MAX30102_OK != Max30102_FifoSampleAveraging(FIFO_SMP_AVE_1)) 
    return MAX30102_ERROR;
  if (MAX30102_OK != Max30102_FifoRolloverEnable(0))
    return MAX30102_ERROR;
  if (MAX30102_OK != Max30102_FifoAlmostFullValue(
      MAX30102_FIFO_ALMOST_FULL_SAMPLES))
    return MAX30102_ERROR;
  if (MAX30102_OK != Max30102_SetMode(MODE_SPO2_MODE))
    return MAX30102_ERROR;
  if (MAX30102_OK != Max30102_SpO2AdcRange(SPO2_ADC_RGE_4096)) 
    return MAX30102_ERROR;
  if (MAX30102_OK != Max30102_SpO2SampleRate(SPO2_SAMPLE_RATE))
    return MAX30102_ERROR;
  if (MAX30102_OK != Max30102_SpO2LedPulseWidth(SPO2_PULSE_WIDTH_411)) 
    return MAX30102_ERROR;
  if (MAX30102_OK != Max30102_Led1PulseAmplitude(MAX30102_RED_LED_CURRENT_LOW))
    return MAX30102_ERROR;
  if (MAX30102_OK != Max30102_Led2PulseAmplitude(MAX30102_IR_LED_CURRENT_LOW))
    return MAX30102_ERROR;
  if (MAX30102_OK != Max30102_SetIntAlmostFullEnabled(1))
    return MAX30102_ERROR;
  if (MAX30102_OK != Max30102_SetIntFifoDataReadyEnabled(1))
    return MAX30102_ERROR;
//	if(MAX30102_OK != Max30102_WriteReg(REG_PILOT_PA,0x7f))   // Choose value for ~ 25mA for Pilot LED
//		return MAX30102_ERROR;
  StateMachine = MAX30102_STATE_BEGIN;
  return MAX30102_OK;
}
#endif
#ifdef editII
unsigned char count = 0;
unsigned int min = 300000, max = 0;

int8_t spo2valid=0,hrvalid=0;
int32_t spo2=0,hr=0;
int32_t hrs[5],spo2s[5];
uint32_t Ir[MAX30102_BUFFER_LENGTH],Red[MAX30102_BUFFER_LENGTH];
unsigned char cnt = 0,counter=0;
void max30102_init(max30102_t *obj, I2C_HandleTypeDef *hi2c) {
    obj->_ui2c = hi2c;
    obj->_interrupt_flag = 0;
    memset(obj->_ir_samples, 0, MAX30102_SAMPLE_LEN_MAX * sizeof(uint32_t));
    memset(obj->_red_samples, 0, MAX30102_SAMPLE_LEN_MAX * sizeof(uint32_t));
}

/**
 * @brief Write buffer of buflen bytes to a register of the MAX30102.
 *
 * @param obj Pointer to max30102_t object instance.
 * @param reg Register address to write to.
 * @param buf Pointer containing the bytes to write.
 * @param buflen Number of bytes to write.
 */
void max30102_write(max30102_t *obj, uint8_t reg, uint8_t *buf, uint16_t buflen) {
	uint8_t *payload = (uint8_t *)malloc((buflen + 1) * sizeof(uint8_t));
	*payload = reg;
	if (buf != NULL && buflen != 0)
			memcpy(payload + 1, buf, buflen);
	HAL_I2C_Master_Transmit(obj->_ui2c, MAX30102_I2C_ADDR << 1, payload, buflen + 1, MAX30102_I2C_TIMEOUT);
	free(payload);
}

/**
 * @brief Read buflen bytes from a register of the MAX30102 and store to buffer.
 *
 * @param obj Pointer to max30102_t object instance.
 * @param reg Register address to read from.
 * @param buf Pointer to the array to write to.
 * @param buflen Number of bytes to read.
 */
void max30102_read(max30102_t *obj, uint8_t reg, uint8_t *buf, uint16_t buflen) {
    uint8_t reg_addr = reg;
    HAL_I2C_Master_Transmit(obj->_ui2c, MAX30102_I2C_ADDR << 1, &reg_addr, 1, MAX30102_I2C_TIMEOUT);
    HAL_I2C_Master_Receive(obj->_ui2c, MAX30102_I2C_ADDR << 1, buf, buflen, MAX30102_I2C_TIMEOUT);
}

/**
 * @brief Reset the sensor.
 *
 * @param obj Pointer to max30102_t object instance.
 */
void max30102_reset(max30102_t *obj) {
    uint8_t val = 0x40;
    max30102_write(obj, MAX30102_MODE_CONFIG, &val, 1);
}

/**
 * @brief Enable A_FULL interrupt.
 *
 * @param obj Pointer to max30102_t object instance.
 * @param enable Enable (1) or disable (0).
 */
void max30102_set_a_full(max30102_t *obj, uint8_t enable) {
    uint8_t reg = 0;
    max30102_read(obj, MAX30102_INTERRUPT_ENABLE_1, &reg, 1);
    reg &= ~(0x01 << MAX30102_INTERRUPT_A_FULL);
    reg |= ((enable & 0x01) << MAX30102_INTERRUPT_A_FULL);
    max30102_write(obj, MAX30102_INTERRUPT_ENABLE_1, &reg, 1);
}

/**
 * @brief Enable PPG_RDY interrupt.
 *
 * @param obj Pointer to max30102_t object instance.
 * @param enable Enable (1) or disable (0).
 */
void max30102_set_ppg_rdy(max30102_t *obj, uint8_t enable) {
    uint8_t reg = 0;
    max30102_read(obj, MAX30102_INTERRUPT_ENABLE_1, &reg, 1);
    reg &= ~(0x01 << MAX30102_INTERRUPT_PPG_RDY);
    reg |= ((enable & 0x01) << MAX30102_INTERRUPT_PPG_RDY);
    max30102_write(obj, MAX30102_INTERRUPT_ENABLE_1, &reg, 1);
}

/**
 * @brief Enable ALC_OVF interrupt.
 *
 * @param obj Pointer to max30102_t object instance.
 * @param enable Enable (1) or disable (0).
 */
void max30102_set_alc_ovf(max30102_t *obj, uint8_t enable) {
    uint8_t reg = 0;
    max30102_read(obj, MAX30102_INTERRUPT_ENABLE_1, &reg, 1);
    reg &= ~(0x01 << MAX30102_INTERRUPT_ALC_OVF);
    reg |= ((enable & 0x01) << MAX30102_INTERRUPT_ALC_OVF);
    max30102_write(obj, MAX30102_INTERRUPT_ENABLE_1, &reg, 1);
}

/**
 * @brief Enable DIE_TEMP_RDY interrupt.
 *
 * @param obj Pointer to max30102_t object instance.
 * @param enable Enable (1) or disable (0).
 */
void max30102_set_die_temp_rdy(max30102_t *obj, uint8_t enable) {
    uint8_t reg = (enable & 0x01) << MAX30102_INTERRUPT_DIE_TEMP_RDY;
    max30102_write(obj, MAX30102_INTERRUPT_ENABLE_2, &reg, 1);
}

/**
 * @brief Enable temperature measurement.
 *
 * @param obj Pointer to max30102_t object instance.
 * @param enable Enable (1) or disable (0).
 */
void max30102_set_die_temp_en(max30102_t *obj, uint8_t enable) {
    uint8_t reg = (enable & 0x01) << MAX30102_DIE_TEMP_EN;
    max30102_write(obj, MAX30102_DIE_TEMP_CONFIG, &reg, 1);
}

/**
 * @brief Set interrupt flag on interrupt. To be called in the corresponding external interrupt handler.
 *
 * @param obj Pointer to max30102_t object instance.
 */
void max30102_on_interrupt(max30102_t *obj) {
    obj->_interrupt_flag = 1;
}

/**
 * @brief Check whether the interrupt flag is active.
 *
 * @param obj Pointer to max30102_t object instance.
 * @return uint8_t Active (1) or inactive (0).
 */
uint8_t max30102_has_interrupt(max30102_t *obj) {
    return obj->_interrupt_flag;
}

/**
 * @brief Read interrupt status registers (0x00 and 0x01) and perform corresponding tasks.
 *
 * @param obj Pointer to max30102_t object instance.
 */
void max30102_interrupt_handler(max30102_t *obj) {
    uint8_t reg[2] = {0x00};
    // Interrupt flag in registers 0x00 and 0x01 are cleared on read
    max30102_read(obj, MAX30102_INTERRUPT_STATUS_1, reg, 2);

    if ((reg[0] >> MAX30102_INTERRUPT_A_FULL) & 0x01)
    {
        // FIFO almost full
        max30102_read_fifo(obj);
    }

    if ((reg[0] >> MAX30102_INTERRUPT_PPG_RDY) & 0x01)
    {
        // New FIFO data ready
    }

    if ((reg[0] >> MAX30102_INTERRUPT_ALC_OVF) & 0x01)
    {
        // Ambient light overflow
    }

    if ((reg[1] >> MAX30102_INTERRUPT_DIE_TEMP_RDY) & 0x01)
    {
        // Temperature data ready
        int8_t temp_int;
        uint8_t temp_frac;
        max30102_read_temp(obj, &temp_int, &temp_frac);
        // float temp = temp_int + 0.0625f * temp_frac;
    }

    // Reset interrupt flag
    obj->_interrupt_flag = 0;
}

/**
 * @brief Shutdown the sensor.
 *
 * @param obj Pointer to max30102_t object instance.
 * @param shdn Shutdown bit.
 */
void max30102_shutdown(max30102_t *obj, uint8_t shdn) {
    uint8_t config;
    max30102_read(obj, MAX30102_MODE_CONFIG, &config, 1);
    config = (config & 0x7f) | (shdn << MAX30102_MODE_SHDN);
    max30102_write(obj, MAX30102_MODE_CONFIG, &config, 1);
}

/**
 * @brief Set measurement mode.
 *
 * @param obj Pointer to max30102_t object instance.
 * @param mode Measurement mode enum (max30102_mode_t).
 */
void max30102_set_mode(max30102_t *obj, max30102_mode_t mode) {
    uint8_t config;
    max30102_read(obj, MAX30102_MODE_CONFIG, &config, 1);
    config = (config & 0xf8) | mode;
    max30102_write(obj, MAX30102_MODE_CONFIG, &config, 1);
    max30102_clear_fifo(obj);
}

/**
 * @brief Set sampling rate.
 *
 * @param obj Pointer to max30102_t object instance.
 * @param sr Sampling rate enum (max30102_spo2_st_t).
 */
void max30102_set_sampling_rate(max30102_t *obj, max30102_sr_t sr) {
    uint8_t config;
    max30102_read(obj, MAX30102_SPO2_CONFIG, &config, 1);
    config = (config & 0x63) << MAX30102_SPO2_SR;
    max30102_write(obj, MAX30102_SPO2_CONFIG, &config, 1);
}

/**
 * @brief Set led pulse width.
 *
 * @param obj Pointer to max30102_t object instance.
 * @param pw Pulse width enum (max30102_led_pw_t).
 */
void max30102_set_led_pulse_width(max30102_t *obj, max30102_led_pw_t pw) {
    uint8_t config;
    max30102_read(obj, MAX30102_SPO2_CONFIG, &config, 1);
    config = (config & 0x7c) | (pw << MAX30102_SPO2_LEW_PW);
    max30102_write(obj, MAX30102_SPO2_CONFIG, &config, 1);
}

/**
 * @brief Set ADC resolution.
 *
 * @param obj Pointer to max30102_t object instance.
 * @param adc ADC resolution enum (max30102_adc_t).
 */
void max30102_set_adc_resolution(max30102_t *obj, max30102_adc_t adc) {
    uint8_t config;
    max30102_read(obj, MAX30102_SPO2_CONFIG, &config, 1);
    config = (config & 0x1f) | (adc << MAX30102_SPO2_ADC_RGE);
    max30102_write(obj, MAX30102_SPO2_CONFIG, &config, 1);
}

/**
 * @brief Set LED current.
 *
 * @param obj Pointer to max30102_t object instance.
 * @param ma LED current float (0 < ma < 51.0).
 */
void max30102_set_led_current_1(max30102_t *obj, float ma) {
    uint8_t pa = ma / 0.2;
    max30102_write(obj, MAX30102_LED_IR_PA1, &pa, 1);
}

/**
 * @brief Set LED current.
 *
 * @param obj Pointer to max30102_t object instance.
 * @param ma LED current float (0 < ma < 51.0).
 */
void max30102_set_led_current_2(max30102_t *obj, float ma) {
    uint8_t pa = ma / 0.2;
    max30102_write(obj, MAX30102_LED_RED_PA2, &pa, 1);
}

/**
 * @brief Set slot mode when in multi-LED mode.
 *
 * @param obj Pointer to max30102_t object instance.
 * @param slot1 Slot 1 mode enum (max30102_multi_led_ctrl_t).
 * @param slot2 Slot 2 mode enum (max30102_multi_led_ctrl_t).
 */
void max30102_set_multi_led_slot_1_2(max30102_t *obj, max30102_multi_led_ctrl_t slot1, max30102_multi_led_ctrl_t slot2) {
    uint8_t val = 0;
    val |= ((slot1 << MAX30102_MULTI_LED_CTRL_SLOT1) | (slot2 << MAX30102_MULTI_LED_CTRL_SLOT2));
    max30102_write(obj, MAX30102_MULTI_LED_CTRL_1, &val, 1);
}

/**
 * @brief Set slot mode when in multi-LED mode.
 *
 * @param obj Pointer to max30102_t object instance.
 * @param slot1 Slot 1 mode enum (max30102_multi_led_ctrl_t).
 * @param slot2 Slot 2 mode enum (max30102_multi_led_ctrl_t).
 */
void max30102_set_multi_led_slot_3_4(max30102_t *obj, max30102_multi_led_ctrl_t slot3, max30102_multi_led_ctrl_t slot4) {
    uint8_t val = 0;
    val |= ((slot3 << MAX30102_MULTI_LED_CTRL_SLOT3) | (slot4 << MAX30102_MULTI_LED_CTRL_SLOT4));
    max30102_write(obj, MAX30102_MULTI_LED_CTRL_2, &val, 1);
}

/**
 * @brief
 *
 * @param obj Pointer to max30102_t object instance.
 * @param smp_ave
 * @param roll_over_en Roll over enabled(1) or disabled(0).
 * @param fifo_a_full Number of empty samples when A_FULL interrupt issued (0 < fifo_a_full < 15).
 */
void max30102_set_fifo_config(max30102_t *obj, max30102_smp_ave_t smp_ave, uint8_t roll_over_en, uint8_t fifo_a_full) {
    uint8_t config = 0x00;
    config |= smp_ave << MAX30102_FIFO_CONFIG_SMP_AVE;
    config |= ((roll_over_en & 0x01) << MAX30102_FIFO_CONFIG_ROLL_OVER_EN);
    config |= ((fifo_a_full & 0x0f) << MAX30102_FIFO_CONFIG_FIFO_A_FULL);
    max30102_write(obj, MAX30102_FIFO_CONFIG, &config, 1);
}

/**
 * @brief Clear all FIFO pointers in the sensor.
 *
 * @param obj Pointer to max30102_t object instance.
 */
void max30102_clear_fifo(max30102_t *obj) {
    uint8_t val = 0x00;
    max30102_write(obj, MAX30102_FIFO_WR_PTR, &val, 3);
    max30102_write(obj, MAX30102_FIFO_RD_PTR, &val, 3);
    max30102_write(obj, MAX30102_OVF_COUNTER, &val, 3);
}


void addSample(uint32_t sample) {
	min = 300000;
	max = 0;
	for (unsigned char i=0; i<199; i++) {
		Ir[i] = Ir[i + 1];
		if (Ir[i] < min)
			min = Ir[i];
		if (Ir[i] > max)
			max = Ir[i];
	}
	if (sample < min)
		min = sample;
	if (sample > max)
		max = sample;
	Ir[199] = sample;
}

uint32_t getRefactored(uint32_t sample) {
	unsigned int distance = max - min;
	unsigned int newSample = sample - min;
	float percent = (newSample * 100) / distance;
	return (uint32_t)((percent * 700) + 190000);
}

/**
 * @brief Read FIFO content and store to buffer in max30102_t object instance.
 *
 * @param obj Pointer to max30102_t object instance.
 */
void max30102_read_fifo(max30102_t *obj) {
	// First transaction: Get the FIFO_WR_PTR
	uint8_t wr_ptr = 0, rd_ptr = 0;
	max30102_read(obj, MAX30102_FIFO_WR_PTR, &wr_ptr, 1);
	max30102_read(obj, MAX30102_FIFO_RD_PTR, &rd_ptr, 1);
	unsigned char buf[30];

	int8_t i = 0;	  
	int8_t num_samples;

	num_samples = (int8_t)wr_ptr - (int8_t)rd_ptr;
	if (num_samples < 1)
		num_samples += 32;

	// Second transaction: Read NUM_SAMPLES_TO_READ samples from the FIFO
	for (i=0; i<num_samples; i++) {
		uint8_t sample[6];
		max30102_read(obj, MAX30102_FIFO_DATA, sample, 6);
		uint32_t ir_sample   = ((uint32_t)(sample[0] << 16) | (uint32_t)(sample[1] << 8) | (uint32_t)(sample[2])) & 0x3ffff;
		uint32_t red_sample  = ((uint32_t)(sample[3] << 16) | (uint32_t)(sample[4] << 8) | (uint32_t)(sample[5])) & 0x3ffff;
		obj->_ir_samples[i]  = ir_sample;
		obj->_red_samples[i] = red_sample;

		//int32_t ave = addSample(ir_sample, red_sample);
		//ave = (int32_t)(ave - ir_sample);

		//sprintf(buf, "%d , %d\n", red_sample, ave);
		//ir_sample = addSample1(ir_sample);
		
		//addSample(ir_sample);
		//ir_sample += 30000;
		if (ir_sample > 190000) {
			sprintf(buf, "{\"M\":{\"Smple\":%ld}}\n", ir_sample); //getRefactored(ir_sample));
			send(buf);
		}
		
		/*if (ir_sample > 190000) {
			sprintf(buf, "{\"M\":{\"Smple\":%ld}}\n", ir_sample);
			send(buf);
		}*/
	}
}

/**
 * @brief Read die temperature.
 *
 * @param obj Pointer to max30102_t object instance.
 * @param temp_int Pointer to store the integer part of temperature. Stored in 2's complement format.
 * @param temp_frac Pointer to store the fractional part of temperature. Increments of 0.0625 deg C.
 */

void max30102_read_temp(max30102_t *obj, int8_t *temp_int, uint8_t *temp_frac) {
	max30102_read(obj, MAX30102_DIE_TINT, (uint8_t *)temp_int, 1);
	max30102_read(obj, MAX30102_DIE_TFRAC, temp_frac, 1);
}

__weak void max30102_plot(uint32_t ir_sample, uint32_t red_sample) {}
#endif