#pragma once
#include <stdbool.h>
#include <stdint.h>

void wsprStartTx(void);
void wsprIntCall(void);
void wsprInit(uint64_t baseFreq);
uint64_t hzToFreq(float f);
