#include "wspr.h"
#include "main.h"
#include "si5351.h"
#include <stdint.h>

void wsprStartTx(void){
	HAL_TIM_Base_Start_IT(&htim3);
	si5351_EnableOutputs(1);
}

int wsprToneCounter = 0;



void wsprStopTx(void){
	wsprToneCounter = 0;
	HAL_TIM_Base_Stop_IT(&htim3);
	si5351_EnableOutputs(0);
}

int wsprTones[] = {3, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 0, 3, 3, 1, 0, 2, 2, 1, 2, 0, 3, 2, 1, 3, 3, 1, 2, 2, 0, 0, 2, 0, 0, 3, 0, 0, 1, 2, 1, 0, 2, 0, 0, 2, 2, 3, 0, 3, 3, 2, 2, 1, 3, 0, 3, 0, 2, 0, 3, 1, 2, 1, 2, 2, 0, 2, 1, 1, 0, 3, 0, 1, 2, 3, 2, 1, 0, 2, 3, 0, 2, 3, 0, 1, 3, 2, 0, 0, 3, 3, 0, 1, 2, 1, 0, 2, 0, 3, 2, 2, 0, 0, 0, 3, 0, 0, 3, 2, 2, 1, 3, 3, 2, 3, 1, 0, 0, 1, 3, 2, 3, 0, 2, 2, 3, 3, 3, 2, 2, 0, 2, 2, 1, 2, 3, 0, 0, 3, 1, 0, 2, 2, 2, 2, 0, 0, 3, 3, 0, 1, 0, 3, 3, 2, 0, 0, 1, 1, 0, 2, 2};
float wsprFreqs[4] = { };

si5351PLLConfig_t pll_conf[4];
si5351OutputConfig_t out_conf[4];

void wsprInit(uint32_t baseFreq){
	for(int i=0; i<4; i++){
		wsprFreqs[i] = baseFreq+i * 1000;
		si5351_Calc(wsprFreqs[i], &pll_conf[i], &out_conf[i]);
	}
}


inline void wsprIntCall(void){
	HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_8);
	//si5351_SetupCLK0(wsprFreqs[wsprTones[wsprToneCounter]], SI5351_DRIVE_STRENGTH_4MA);

	si5351_SetupPLL(SI5351_PLL_A, &pll_conf[wsprTones[wsprToneCounter]]);
	si5351_SetupOutput(0, SI5351_PLL_A, SI5351_DRIVE_STRENGTH_4MA, &out_conf[wsprTones[wsprToneCounter]], 0);

	if (wsprToneCounter < 162) wsprToneCounter++;
	else wsprStopTx();
};

