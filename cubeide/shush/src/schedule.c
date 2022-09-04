#include "schedule.h"
#include "si5351.h"
#include "tools.h"
#include "main.h"
#include "rtc.h"
#include "eeprom.h"

void everythingElse(void);

void schedInit(void){
	HAL_ADCEx_Calibration_Start(&hadc1);
	HAL_Delay(100);
	debugInit();

	HAL_Delay(10);
}

void schedMainLoop(void){
	 everythingElse();
}

inline void schedRun(void){
	schedInit();
	while (1){
		schedMainLoop();
	}
}

bool elseDone = 0;


void everythingElse(void){
	if (!elseDone){
			elseDone = 1;

	}

}





