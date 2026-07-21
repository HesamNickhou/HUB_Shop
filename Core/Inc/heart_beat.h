#ifndef __HEART_BEAT
#define __HEART_BEAT

#include "stm32f1xx_hal.h"

//#define DEBUG_HEART

void ADC1_Init(void);
uint32_t ADC_Read(void);
void heartBeat();

#endif