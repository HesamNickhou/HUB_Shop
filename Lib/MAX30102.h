/** \file max30102.h ******************************************************
 *
 * Project: MAXREFDES117#
 * Filename: max30102.h
 * Description: This module is an embedded controller driver header file for MAX30102
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
#ifndef MAX30102_H_
#define MAX30102_H_

#include "stdio.h"
#include "main.h"
#define editII

#ifdef editI
#define MAX30102_ADDRESS 0xAE	//(0x57<<1)
//#define MAX30102_USE_INTERNAL_TEMPERATURE

#define MAX30102_MEASUREMENT_SECONDS      4
#define MAX30102_SAMPLES_PER_SECOND	      100 // 50, 100, 200, 400, 800, 1000, 1600, 3200 sample rating
#define MAX30102_FIFO_ALMOST_FULL_SAMPLES 17

#define MAX30102_BUFFER_LENGTH	((MAX30102_MEASUREMENT_SECONDS+1) * MAX30102_SAMPLES_PER_SECOND)

//
//	Calibration
//
#define MAX30102_IR_LED_CURRENT_LOW		      0x01
#define MAX30102_RED_LED_CURRENT_LOW		      0x00
#define MAX30102_IR_LED_CURRENT_HIGH		      0x24
#define MAX30102_RED_LED_CURRENT_HIGH	      0x24
#define MAX30102_IR_VALUE_FINGER_ON_SENSOR  1600
#define MAX30102_IR_VALUE_FINGER_OUT_SENSOR 70000
//
//	Register addresses
//
#define REG_INTR_STATUS_1   0x00
#define REG_INTR_STATUS_2   0x01
#define REG_INTR_ENABLE_1   0x02
#define REG_INTR_ENABLE_2   0x03
#define REG_FIFO_WR_PTR     0x04
#define REG_OVF_COUNTER     0x05
#define REG_FIFO_RD_PTR     0x06
#define REG_FIFO_DATA       0x07
#define REG_FIFO_CONFIG     0x08
#define REG_MODE_CONFIG     0x09
#define REG_SPO2_CONFIG     0x0A
#define REG_LED1_PA         0x0C
#define REG_LED2_PA         0x0D
#define REG_PILOT_PA        0x10
#define REG_MULTI_LED_CTRL1 0x11
#define REG_MULTI_LED_CTRL2 0x12
#define REG_TEMP_INTR       0x1F
#define REG_TEMP_FRAC       0x20
#define REG_TEMP_CONFIG     0x21
#define REG_PROX_INT_THRESH 0x30
#define REG_REV_ID          0xFE
#define REG_PART_ID         0xFF

//
//	Interrupt Status 1 (0x00)
//	Interrupt Status 2 (0x01)
//	Interrupt Enable 1 (0x02)
//	Interrupt Enable 2 (0x03)
//
#define	INT_A_FULL_BIT			  7
#define	INT_PPG_RDY_BIT			  6
#define	INT_ALC_OVF_BIT			  5
#define	INT_DIE_TEMP_RDY_BIT	1
#define	INT_PWR_RDY_BIT			  0	// Only STATUS register

//
//	FIFO Configuration (0x08)
//
#define FIFO_CONF_SMP_AVE_BIT 			      7
#define FIFO_CONF_SMP_AVE_LENGHT 		    3
#define FIFO_CONF_FIFO_ROLLOVER_EN_BIT 	4
#define FIFO_CONF_FIFO_A_FULL_BIT 		    3
#define FIFO_CONF_FIFO_A_FULL_LENGHT 	  4

#define FIFO_SMP_AVE_1	0
#define FIFO_SMP_AVE_2	1
#define FIFO_SMP_AVE_4	2
#define FIFO_SMP_AVE_8	3
#define FIFO_SMP_AVE_16	  4
#define FIFO_SMP_AVE_32	  5

//
//	Mode Configuration (0x09)
//
#define MODE_SHDN_BIT	  	7
#define MODE_RESET_BIT		  6
#define MODE_MODE_BIT		  2
#define MODE_MODE_LENGTH  3

#define MODE_HEART_RATE_MODE  2
#define MODE_SPO2_MODE			    3
#define MODE_MULTI_LED_MODE   7

//
//	SpO2 Configuration (0x0A)
//
#define SPO2_CONF_ADC_RGE_BIT		  6
#define SPO2_CONF_ADC_RGE_LENGTH	  2
#define SPO2_CONF_SR_BIT			      4
#define SPO2_CONF_SR_LENGTH			  3
#define SPO2_CONF_LED_PW_BIT		    1
#define SPO2_CONF_LED_PW_LENGTH   2

#define	SPO2_ADC_RGE_2048   0
#define	SPO2_ADC_RGE_4096	  1
#define	SPO2_ADC_RGE_8192	  2
#define	SPO2_ADC_RGE_16384	3

#define	SPO2_SAMPLE_RATE_50		0
#define	SPO2_SAMPLE_RATE_100	1
#define	SPO2_SAMPLE_RATE_200	2
#define	SPO2_SAMPLE_RATE_400	3
#define	SPO2_SAMPLE_RATE_800	4
#define	SPO2_SAMPLE_RATE_1000	5
#define	SPO2_SAMPLE_RATE_1600	6
#define	SPO2_SAMPLE_RATE_3200	7

#if(MAX30102_SAMPLES_PER_SECOND == 50)
#define SPO2_SAMPLE_RATE SPO2_SAMPLE_RATE_50
#elif((MAX30102_SAMPLES_PER_SECOND == 100))
#define SPO2_SAMPLE_RATE SPO2_SAMPLE_RATE_100 
#elif((MAX30102_SAMPLES_PER_SECOND == 200))
#define SPO2_SAMPLE_RATE SPO2_SAMPLE_RATE_200
#elif((MAX30102_SAMPLES_PER_SECOND == 400))
#define SPO2_SAMPLE_RATE SPO2_SAMPLE_RATE_400
#elif((MAX30102_SAMPLES_PER_SECOND == 800))
#define SPO2_SAMPLE_RATE SPO2_SAMPLE_RATE_800
#elif((MAX30102_SAMPLES_PER_SECOND == 1000))
#define SPO2_SAMPLE_RATE SPO2_SAMPLE_RATE_1000
#elif((MAX30102_SAMPLES_PER_SECOND == 1600))
#define SPO2_SAMPLE_RATE SPO2_SAMPLE_RATE_1600
#elif((MAX30102_SAMPLES_PER_SECOND == 3200))
#define SPO2_SAMPLE_RATE SPO2_SAMPLE_RATE_3200
#else
#error "Wrong Sample Rate value. Use 50, 100, 200, 400, 800, 1000, 1600 or 3200."
#endif

#define	SPO2_PULSE_WIDTH_69			0
#define	SPO2_PULSE_WIDTH_118		1
#define	SPO2_PULSE_WIDTH_215		2
#define	SPO2_PULSE_WIDTH_411		3

//
//	Status enum
//
typedef enum
{
  MAX30102_ERROR = 0, MAX30102_OK = 1
} MAX30102_STATUS;

//
//	Functions
//
MAX30102_STATUS Max30102_Init(I2C_HandleTypeDef *i2c);
MAX30102_STATUS Max30102_ReadFifo(volatile uint32_t *pun_red_led,
    volatile uint32_t *pun_ir_led);
MAX30102_STATUS Max30102_WriteReg(uint8_t uch_addr, uint8_t uch_data);
MAX30102_STATUS Max30102_ReadReg(uint8_t uch_addr, uint8_t *puch_data);
//
//	Interrupts
//
MAX30102_STATUS Max30102_ReadInterruptStatus(uint8_t *Status);
MAX30102_STATUS Max30102_SetIntAlmostFullEnabled(uint8_t Enable);
MAX30102_STATUS Max30102_SetIntFifoDataReadyEnabled(uint8_t Enable);
MAX30102_STATUS Max30102_SetIntAmbientLightCancelationOvfEnabled(uint8_t Enable);
#ifdef MAX30102_USE_INTERNAL_TEMPERATURE
MAX30102_STATUS Max30102_SetIntInternalTemperatureReadyEnabled(uint8_t Enable);
#endif
void Max30102_InterruptCallback(void);
//
//	FIFO Configuration
//
MAX30102_STATUS Max30102_FifoWritePointer(uint8_t Address);
MAX30102_STATUS Max30102_FifoOverflowCounter(uint8_t Address);
MAX30102_STATUS Max30102_FifoReadPointer(uint8_t Address);
MAX30102_STATUS Max30102_FifoSampleAveraging(uint8_t Value);
MAX30102_STATUS Max30102_FifoRolloverEnable(uint8_t Enable);
MAX30102_STATUS Max30102_FifoAlmostFullValue(uint8_t Value); // 17-32 samples ready in FIFO
//
//	Mode Configuration
//
MAX30102_STATUS Max30102_ShutdownMode(uint8_t Enable);
MAX30102_STATUS Max30102_Reset(void);
MAX30102_STATUS Max30102_SetMode(uint8_t Mode);
//
//	SpO2 Configuration
//
MAX30102_STATUS Max30102_SpO2AdcRange(uint8_t Value);
MAX30102_STATUS Max30102_SpO2SampleRate(uint8_t Value);
MAX30102_STATUS Max30102_SpO2LedPulseWidth(uint8_t Value);
//
//	LEDs Pulse Amplitute Configuration
//
MAX30102_STATUS Max30102_Led1PulseAmplitude(uint8_t Value);
MAX30102_STATUS Max30102_Led2PulseAmplitude(uint8_t Value);
//
//	Usage functions
//
void Max30102_Task(void);
int32_t Max30102_GetHeartRate(void);
int32_t Max30102_GetSpO2Value(void);
uint8_t Max30102_IsFingerOnSensor(void);
#endif

#ifdef editII
#include "main.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>

#define MAX30102_BUFFER_LENGTH 200	//((MAX30102_MEASUREMENT_SECONDS+1) * MAX30102_SAMPLES_PER_SECOND)

#define MAX30102_I2C_ADDR 0x57
#define MAX30102_I2C_TIMEOUT 1000

#define MAX30102_BYTES_PER_SAMPLE 6
#define MAX30102_SAMPLE_LEN_MAX 32

#define MAX30102_INTERRUPT_STATUS_1 0x00
#define MAX30102_INTERRUPT_STATUS_2 0x01
#define MAX30102_INTERRUPT_ENABLE_1 0x02
#define MAX30102_INTERRUPT_ENABLE_2 0x03
#define MAX30102_INTERRUPT_A_FULL 7
#define MAX30102_INTERRUPT_PPG_RDY 6
#define MAX30102_INTERRUPT_ALC_OVF 5
#define MAX30102_INTERRUPT_DIE_TEMP_RDY 1

#define MAX30102_FIFO_WR_PTR 0x04
#define MAX30102_OVF_COUNTER 0x05
#define MAX30102_FIFO_RD_PTR 0x06

#define MAX30102_FIFO_DATA 0x07

#define MAX30102_FIFO_CONFIG 0x08
#define MAX30102_FIFO_CONFIG_SMP_AVE 5
#define MAX30102_FIFO_CONFIG_ROLL_OVER_EN 4
#define MAX30102_FIFO_CONFIG_FIFO_A_FULL 0

#define MAX30102_MODE_CONFIG 0x09
#define MAX30102_MODE_SHDN 7
#define MAX30102_MODE_RESET 6
#define MAX30102_MODE_MODE 0

#define MAX30102_SPO2_CONFIG 0x0a
#define MAX30102_SPO2_ADC_RGE 5
#define MAX30102_SPO2_SR 2
#define MAX30102_SPO2_LEW_PW 0

#define MAX30102_LED_IR_PA1 0x0c
#define MAX30102_LED_RED_PA2 0x0d

#define MAX30102_MULTI_LED_CTRL_1 0x11
#define MAX30102_MULTI_LED_CTRL_SLOT2 4
#define MAX30102_MULTI_LED_CTRL_SLOT1 0
#define MAX30102_MULTI_LED_CTRL_2 0x12
#define MAX30102_MULTI_LED_CTRL_SLOT4 4
#define MAX30102_MULTI_LED_CTRL_SLOT3 0

#define MAX30102_DIE_TINT 0x1f
#define MAX30102_DIE_TFRAC 0x20
#define MAX30102_DIE_TFRAC_INCREMENT 0.0625f
#define MAX30102_DIE_TEMP_CONFIG 0x21
#define MAX30102_DIE_TEMP_EN 1

typedef enum max30102_mode_t {
    max30102_heart_rate = 0x02,
    max30102_spo2 = 0x03,
    max30102_multi_led = 0x07
} max30102_mode_t;

typedef enum max30102_smp_ave_t {
    max30102_smp_ave_1,
    max30102_smp_ave_2,
    max30102_smp_ave_4,
    max30102_smp_ave_8,
    max30102_smp_ave_16,
    max30102_smp_ave_32,
} max30102_smp_ave_t;

typedef enum max30102_sr_t {
    max30102_sr_50,
    max30102_sr_100,
    max30102_sr_200,
    max30102_sr_400,
    max30102_sr_800,
    max30102_sr_1000,
    max30102_sr_1600,
    max30102_sr_3200
} max30102_sr_t;

typedef enum max30102_led_pw_t {
    max30102_pw_15_bit,
    max30102_pw_16_bit,
    max30102_pw_17_bit,
    max30102_pw_18_bit
} max30102_led_pw_t;

typedef enum max30102_adc_t {
    max30102_adc_2048,
    max30102_adc_4096,
    max30102_adc_8192,
    max30102_adc_16384
} max30102_adc_t;

typedef enum max30102_multi_led_ctrl_t {
    max30102_led_off,
    max30102_led_red,
    max30102_led_ir
} max30102_multi_led_ctrl_t;

typedef struct max30102_t {
    I2C_HandleTypeDef *_ui2c;
    uint32_t _ir_samples[32];
    uint32_t _red_samples[32];
    uint8_t _interrupt_flag;
} max30102_t;

__weak void max30102_plot(uint32_t ir_sample, uint32_t red_sample);

void max30102_init(max30102_t *obj, I2C_HandleTypeDef *hi2c);
void max30102_write(max30102_t *obj, uint8_t reg, uint8_t *buf, uint16_t buflen);
void max30102_read(max30102_t *obj, uint8_t reg, uint8_t *buf, uint16_t buflen);

void max30102_reset(max30102_t *obj);

void max30102_set_a_full(max30102_t *obj, uint8_t enable);
void max30102_set_ppg_rdy(max30102_t *obj, uint8_t enable);
void max30102_set_alc_ovf(max30102_t *obj, uint8_t enable);
void max30102_set_die_temp_rdy(max30102_t *obj, uint8_t enable);
void max30102_set_die_temp_en(max30102_t *obj, uint8_t enable);

void max30102_on_interrupt(max30102_t *obj);
uint8_t max30102_has_interrupt(max30102_t *obj);
void max30102_interrupt_handler(max30102_t *obj);

void max30102_shutdown(max30102_t *obj, uint8_t shdn);

void max30102_set_mode(max30102_t *obj, max30102_mode_t mode);
void max30102_set_sampling_rate(max30102_t *obj, max30102_sr_t sr);

void max30102_set_led_pulse_width(max30102_t *obj, max30102_led_pw_t pw);
void max30102_set_adc_resolution(max30102_t *obj, max30102_adc_t adc);

void max30102_set_led_current_1(max30102_t *obj, float ma);
void max30102_set_led_current_2(max30102_t *obj, float ma);
void max30102_set_multi_led_slot_1_2(max30102_t *obj, max30102_multi_led_ctrl_t slot1, max30102_multi_led_ctrl_t slot2);
void max30102_set_multi_led_slot_3_4(max30102_t *obj, max30102_multi_led_ctrl_t slot3, max30102_multi_led_ctrl_t slot4);

void max30102_set_fifo_config(max30102_t *obj, max30102_smp_ave_t smp_ave, uint8_t roll_over_en, uint8_t fifo_a_full);
void max30102_clear_fifo(max30102_t *obj);
void max30102_read_fifo(max30102_t *obj);

void max30102_read_temp(max30102_t *obj, int8_t *temp_int, uint8_t *temp_frac);

#endif
#endif /* MAX30102_H_ */
