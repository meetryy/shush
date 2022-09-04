#pragma once
#include <stdint.h>

extern char rtcTimeString[7];

void rtcSetTime(uint8_t h, uint8_t m);
uint8_t rtcGetHours(void);
uint8_t rtcGetMinutes(void);
void rtcUpdateTimeString(void);
