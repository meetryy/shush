#include "rtc.h"
//#include "global.h"
#include "main.h"
#include <stdint.h>

char rtcTimeString[7] = "XX:XX";

void rtcSetTime(uint8_t h, uint8_t m){
	if ((h < 24) && (m < 60)){
		RTC_TimeTypeDef sTime = {0};
		sTime.Hours = h;
		sTime.Minutes = m;
		sTime.Seconds = 0;

		if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
			Error_Handler();
	}
	  /*
	  DateToUpdate.WeekDay = RTC_WEEKDAY_WEDNESDAY;
	  DateToUpdate.Month = RTC_MONTH_FEBRUARY;
	  DateToUpdate.Date = 20;
	  DateToUpdate.Year = 19;

	  if (HAL_RTC_SetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BIN) != HAL_OK)
	    Error_Handler();
	  */
}

uint8_t rtcGetHours(void){
	RTC_TimeTypeDef sTime = {0};
	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN); // RTC_FORMAT_BIN , RTC_FORMAT_BCD
	return (uint8_t)(sTime.Hours);
}

uint8_t rtcGetMinutes(void){
	RTC_TimeTypeDef sTime = {0};
	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN); // RTC_FORMAT_BIN , RTC_FORMAT_BCD
	return (uint8_t)(sTime.Minutes);
}

void rtcUpdateTimeString(void){
	RTC_TimeTypeDef sTime = {0};
	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN); // RTC_FORMAT_BIN , RTC_FORMAT_BCD
	snprintf(rtcTimeString, 7, "%02u:%02u\0", (uint8_t)sTime.Hours, (uint8_t)sTime.Minutes);
}
