#include "heart_beat.h"

ADC_HandleTypeDef hadc1;
uint16_t values[10];
uint32_t adcTick = 0;
uint8_t isPressed = 0;
unsigned char buffer[50];

void ADC1_Init(void) {
	__HAL_RCC_ADC1_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	
	// Configure PA0 as analog input
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin  						 = GPIO_PIN_0;
	GPIO_InitStruct.Mode 						 = GPIO_MODE_ANALOG;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	// ADC configuration
	hadc1.Instance 									 = ADC1;
	hadc1.Init.ScanConvMode 				 = ADC_SCAN_DISABLE;
	hadc1.Init.ContinuousConvMode 	 = DISABLE;
	hadc1.Init.DiscontinuousConvMode = DISABLE;
	hadc1.Init.ExternalTrigConv 		 = ADC_SOFTWARE_START;
	hadc1.Init.DataAlign 						 = ADC_DATAALIGN_RIGHT;
	hadc1.Init.NbrOfConversion 			 = 1;
	HAL_ADC_Init(&hadc1);
	
	// Configure ADC channel
	ADC_ChannelConfTypeDef sConfig = {0};
	sConfig.Channel 							 = ADC_CHANNEL_0;
	sConfig.Rank 									 = ADC_REGULAR_RANK_1;
	sConfig.SamplingTime 					 = ADC_SAMPLETIME_55CYCLES_5;
	HAL_ADC_ConfigChannel(&hadc1, &sConfig);
}

uint32_t ADC_Read(void) {
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
	return HAL_ADC_GetValue(&hadc1);
}

void heartBeat() {
	if ((HAL_GetTick() - adcTick) >= 60) {
		unsigned long heart = ADC_Read();
		//sprintf(buffer, "{\"M\":{\"Smple\":%ld}}\n", heart);
		//send(buffer);
		unsigned char i = 0, notNow = 1;
		
		values[0] = values[1];
		values[1] = values[2];
		values[2] = values[3];
		values[3] = values[4];
		values[4] = values[5];
		values[5] = values[6];
		values[6] = values[7];
		values[7] = values[8];
		values[8] = values[9];
		values[9] = heart;
		
		if ((values[0] < 100) &&
				(values[1] < 100) &&
				(values[2] < 100) &&
				(values[3] < 100) &&
				(values[4] < 100) &&
				(values[5] < 100) &&
				(values[6] < 100) &&
				(values[7] < 100) &&
				(values[8] < 100) &&
				(values[9] < 100)) {
			isPressed = 1;
			notNow = 0;
		}
		
		if ((values[0] > 3000) &&
				(values[1] > 3000) &&
				(values[2] > 3000) &&
				(values[3] > 3000) &&
				(values[4] > 3000) &&
				(values[5] > 3000) &&
				(values[6] > 3000) &&
				(values[7] > 3000) &&
				(values[8] > 3000) &&
				(values[9] > 3000))
			isPressed = 0;
		
		if (isPressed && notNow) {
			sprintf(buffer, "{\"M\":{\"Smple\":%ld}}\n", heart);
			send(buffer);
		}
			/*int32_t diff = heart - values[8];
			diff /= 10;
			
			heart *= 40;
			heart += 170000;
			
			//sprintf(buffer, "{\"M\":{\"Smple\":%ld}}\n", heart);
			//send(buffer);
			
			/*for (unsigned char k=0; k<10; k++) {
				heart = values[8] + (k * diff);
				heart *= 15;
				heart += 190000;
				sprintf(buffer, "{\"M\":{\"Smple\":%ld}}\n", heart);
				send(buffer);
				//HAL_Delay(1);
			}
			heart = values[9] * 15 + 190000;*/
			
			/*int32_t A = values[8] * 40 + 170000;
			int32_t B = heart ; 
			if (B > 260000)
				B = 260000;
			if (A > 260000)
				A = 260000;
			
			//> 260000 ? 260000 : heart;
			
			int32_t result = ((B - A) / 8) + A;			
			//sprintf(buffer, "{\"M\":{\"Smple\":%ld}}\n", result > 260000 ? 260000 : (result < 190000 ? 190000 : result));
			sprintf(buffer, "{\"M\":{\"Smple\":%ld}}\n", result);
			send(buffer);
			
			result = 2 * ((B - A) / 8) + A;
			sprintf(buffer, "{\"M\":{\"Smple\":%ld}}\n", result);
			send(buffer);
			
			result = 3 * ((B - A) / 8) + A;			
			sprintf(buffer, "{\"M\":{\"Smple\":%ld}}\n", result);
			send(buffer);
				result = 4 * ((B - A) / 8) + A;			
			sprintf(buffer, "{\"M\":{\"Smple\":%ld}}\n", result);
			send(buffer);
			result = 5 * ((B - A) / 8) + A;			
			sprintf(buffer, "{\"M\":{\"Smple\":%ld}}\n", result);
			send(buffer);
			result = 6 * ((B - A) / 8) + A;			
			sprintf(buffer, "{\"M\":{\"Smple\":%ld}}\n", result);
			send(buffer);
			result = 7 * ((B - A) / 8) + A;			
			sprintf(buffer, "{\"M\":{\"Smple\":%ld}}\n", result);
			send(buffer);
		
			sprintf(buffer, "{\"M\":{\"Smple\":%ld}}\n", B);
			send(buffer);
			
		}*/
		
		adcTick = HAL_GetTick();
	}
}
