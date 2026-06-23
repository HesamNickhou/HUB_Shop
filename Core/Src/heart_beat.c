#include "heart_beat.h"

ADC_HandleTypeDef hadc1;
uint32_t adcTick = 0;
uint8_t fingerDetected = 0;
unsigned char buffer[50];

typedef enum {
    SENSOR_IDLE = 0,
    SENSOR_WAIT_STABLE,
    SENSOR_MEASURING
} SensorState;

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
	static SensorState state   = SENSOR_IDLE;
	static uint8_t lowCounter  = 0;
	static uint8_t highCounter = 0;
	
	if ((HAL_GetTick() - adcTick) >= 60) {
		unsigned long heart = ADC_Read();
		unsigned char i = 0;
		
		switch (state) {
        //------------------------------------------------------
        // Waiting for a finger
        //------------------------------------------------------
        case SENSOR_IDLE:
					if (heart < 100) {
						if (lowCounter < 50)
							lowCounter++;
					}
					else
						lowCounter = 0; // Restart if the low sequence is interrupted.
					if (lowCounter >= 40) { // Finger detected.
						state 		 = SENSOR_WAIT_STABLE;
						lowCounter = 0;
					}
					break;

        //------------------------------------------------------
        // Ignore the initialization period (<100)
        //------------------------------------------------------
        case SENSOR_WAIT_STABLE:
					// Wait until the sensor starts producing real values.
					if ((heart >= 100) && (heart < 3000)) {
						state = SENSOR_MEASURING;
						highCounter = 0;
						sprintf(buffer, "{\"M\":{\"Smple\":%lu}}\n", heart);
						send(buffer);
					}
					break;

        //------------------------------------------------------
        // Send pulse samples
        //------------------------------------------------------
        case SENSOR_MEASURING:
					if (heart > 3000) {
						if (highCounter < 50)
							highCounter++;
					}
					else {
						// Any normal sample means we're still measuring.
						highCounter = 0;
						sprintf(buffer, "{\"M\":{\"Sample\":%lu}}\n", heart);
						send(buffer);
					}

					if (highCounter >= 40) {
						state = SENSOR_IDLE; // Finger removed.
						highCounter = 0;
					}
					break;

        default:
					state = SENSOR_IDLE;
					lowCounter = 0;
					highCounter = 0;
					break;
    }
		/*values[0] = values[1];
		values[1] = values[2];
		values[2] = values[3];
		values[3] = values[4];
		values[4] = values[5];
		values[5] = values[6];
		values[6] = values[7];
		values[7] = values[8];
		values[8] = values[9];
		values[9] = heart;*/
		
		/*memmove(values, values + 1, sizeof(values) - sizeof(values[0]));
		values[9] = heart;
		
		uint8_t lowCount = 0;
		uint8_t highCount = 0;

		for (i = 0; i < 10; i++) {
			if (values[i] < 100)
					lowCount++;
			if (values[i] > 3000)
					highCount++;
		}
		if (lowCount >= 10)
			fingerDetected = 1;
		else if (highCount == 10)
			fingerDetected = 0;*/
		
		/*if ((values[0] < 100) &&
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
			isPressed = 0;*/
		
		
		
		/*if (fingerDetected && (heart >= 100) && (heart <= 3000)) {
			sprintf(buffer, "{\"M\":{\"Smple\":%ld}}\n", heart);
			send(buffer);
		}*/
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
