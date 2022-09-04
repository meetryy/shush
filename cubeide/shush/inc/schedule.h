#pragma once

#include <stdint.h>
#include <stdbool.h>

extern uint32_t elseSkippedCounter;
extern uint32_t elseLastBlock;

void schedInit(void);
void schedMainLoop(void);
extern inline void schedRun(void);
