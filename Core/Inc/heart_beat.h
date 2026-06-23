#ifndef __HEART_BEAT
#define __HEART_BEAT

//#include "main.h"
//#include "Config.h"
#include "stm32f1xx_hal.h"



void ADC1_Init(void);
uint32_t ADC_Read(void);
void heartBeat();

#endif