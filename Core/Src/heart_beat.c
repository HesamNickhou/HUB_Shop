#include "heart_beat.h"

ADC_HandleTypeDef hadc1;
uint32_t adcTick = 0;
unsigned char buffer[50];
unsigned char debugBuf[60];

#define SAMPLE_MS    60
#define DETECT_CNT   3
#define STABLE_CNT   12
#define REMOVE_CNT   3
#define REMOVE_ZERO_CNT  10
#define TIMEOUT_MS   30000

void ADC1_Init(void) {
	__HAL_RCC_ADC1_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin   = GPIO_PIN_0;
	GPIO_InitStruct.Mode  = GPIO_MODE_ANALOG;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	hadc1.Instance              = ADC1;
	hadc1.Init.ScanConvMode     = ADC_SCAN_DISABLE;
	hadc1.Init.ContinuousConvMode = DISABLE;
	hadc1.Init.DiscontinuousConvMode = DISABLE;
	hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	hadc1.Init.DataAlign        = ADC_DATAALIGN_RIGHT;
	hadc1.Init.NbrOfConversion  = 1;
	HAL_ADC_Init(&hadc1);

	ADC_ChannelConfTypeDef sConfig = {0};
	sConfig.Channel        = ADC_CHANNEL_0;
	sConfig.Rank           = ADC_REGULAR_RANK_1;
	sConfig.SamplingTime   = ADC_SAMPLETIME_55CYCLES_5;
	HAL_ADC_ConfigChannel(&hadc1, &sConfig);
}

uint32_t ADC_Read(void) {
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
	return HAL_ADC_GetValue(&hadc1);
}

/*void heartBeat_old() {
	static enum { IDLE, PLACING, MEASURING } state = IDLE;
	static uint8_t cnt = 0;
	static uint32_t measStart = 0;

	if ((HAL_GetTick() - adcTick) < SAMPLE_MS)
		return;
	adcTick = HAL_GetTick();

	unsigned long heart = ADC_Read();

	switch (state) {

	case IDLE:
		if (heart > 3000) {
			if (++cnt >= DETECT_CNT) {
				sprintf(debugBuf, "[DBG] FINGER PLACED -> PLACING\n");
				send(debugBuf);
				state = PLACING;
				cnt = 0;
			}
		} else {
			cnt = 0;
		}
		break;

	case PLACING:
		if (heart > 3000) {
			cnt = 0;
		} else {
			if (++cnt >= STABLE_CNT) {
				sprintf(debugBuf, "[DBG] FINGER STABLE -> MEASURING\n");
				send(debugBuf);
				state = MEASURING;
				cnt = 0;
				measStart = HAL_GetTick();
			}
		}
		break;

	case MEASURING:
		if (heart > 3000) {
			if (++cnt >= REMOVE_CNT) {
				sprintf(debugBuf, "[DBG] FINGER REMOVED (%lu) -> IDLE\n", heart);
				send(debugBuf);
				state = IDLE;
				cnt = 0;
			}
		} else {
			cnt = 0;
			sprintf(buffer, "{\"M\":{\"Sample\":%lu}}\n", heart);
			send(buffer);
		}
		if (HAL_GetTick() - measStart > TIMEOUT_MS) {
			sprintf(debugBuf, "[DBG] MEAS TIMEOUT -> IDLE\n");
			send(debugBuf);
			state = IDLE;
			cnt = 0;
		}
		break;
	}
}
//======================================*/

/*#define ADC_HIGH_LEVEL      2800
#define ADC_LOW_VALID       1200
#define ADC_MAX_VALID       2600

#define HIGH_COUNT          4
#define NORMAL_COUNT        8
typedef enum {
    PULSE_NO_FINGER = 0,
    PULSE_WAIT_STABLE,
    PULSE_FINGER
}PulseState_t;
static PulseState_t pulseState = PULSE_NO_FINGER;
static uint8_t highCounter = 0;
static uint8_t normalCounter = 0;

char Pulse_Process(uint16_t adc) {
    switch(pulseState) {
        //-----------------------------------------
        case PULSE_NO_FINGER:
            if(adc > ADC_HIGH_LEVEL) {
                highCounter++;
                if(highCounter >= HIGH_COUNT) {
                    pulseState = PULSE_WAIT_STABLE;
                    highCounter = 0;
                    normalCounter = 0;
                }
            }
            else
                highCounter = 0;
            return 0;

        //-----------------------------------------
        case PULSE_WAIT_STABLE:
            if(adc > ADC_LOW_VALID && adc < ADC_MAX_VALID) {
                normalCounter++;
                if(normalCounter >= NORMAL_COUNT) {
                    pulseState = PULSE_FINGER;
                    normalCounter = 0;
                }
            }
            else
                normalCounter = 0;
            return 0;

        //-----------------------------------------
        case PULSE_FINGER:
            if(adc > ADC_HIGH_LEVEL){
                pulseState = PULSE_NO_FINGER;
                return 0;
            }

            if(adc < ADC_LOW_VALID)
                return 0;
            return 1;
    }

    return 0;
}


static unsigned char lowCounter = 0;
typedef enum {
	Idle = 0,
	Push,
	Release
} FingerState_t;
FingerState_t mystate;
unsigned short calculate(unsigned short value) {
	sprintf(buffer, "%d\n", value);
	send(buffer);
	
	if (mystate == Idle)
		sprintf(buffer, "%s\n", "Idle");
	else if (mystate == Push)
		sprintf(buffer, "%s\n", "Push");
	else if (mystate == Release)
		sprintf(buffer, "%s\n", "Idle");
	//send(buffer);
	
	switch (mystate) {
		case (Idle): {
			if (value >= 3000)
				highCounter++;
			else if (value <= 100)
				lowCounter++;
			else if ((highCounter >= 5) && (lowCounter >= 3)) {
				mystate = Push;
				//sprintf(buffer, "\nPushed\n");
				//send(buffer);
				highCounter = 0;
				lowCounter = 0;
			}
			else
				highCounter = lowCounter = 0;
			break;
		}
		
		case (Push): {
			if ((value <= 3200) && (value > 100)) {
				highCounter = 0;
				return value;
			}
			else if (value > 3000) {
				highCounter++;
				if (highCounter >= 5) {
					mystate = Release;
					sprintf(buffer, "Released\n");
					//send(buffer);
				}
			}
			break;
		}
		
		case (Release): {
			if (value < 3000) {
				mystate = Idle;
				sprintf(buffer, "Idle\n");
				//send(buffer);
			}
			break;
		}
		
		
	} //end of switch
	return 0;
}

unsigned short values[40];
void getDiff(unsigned short value) {
	for (int i=0; i<39; i++)
		values[i] = values[i+1];
	values[39] = value;
	
	int min = 3500, max = 0;
	for (int i=0; i<40; i++) {
		if (values[i] > max)
			max = values[i];
		else if (values[i] < min)
			min = values[i];
		
		if ((values[i] >= 3000) || (values[i] <= 100)) {
			max = 5000;
			min = 0;
		}
	}
	sprintf(buffer, "%d\n", value);
		send(buffer);
}*/

void heartBeat() {
	if (HAL_GetTick() - adcTick >= 60) {
		adcTick = HAL_GetTick();
		uint16_t adc = ADC_Read();
		sprintf(buffer, "{\"M\":{\"Smple\":%lu}}\n", adc);
		send(buffer);
	}
}