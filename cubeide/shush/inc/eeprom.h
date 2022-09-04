#pragma once
#include "stm32f1xx_hal.h"

#define FLASH_START_ADDR	(0x8000000)
#define FLASH_LENGTH		(0x1F400)
#define FLASH_PG_SIZE		(PAGESIZE)

#define EEP_PAGES			(3)
#define EEP_START_ADDR		(FLASH_START_ADDR + FLASH_LENGTH - FLASH_PAGE_SIZE * EEP_PAGES)
#define MYADDR 				(EEP_START_ADDR)

//	0x0801E800
void eepTest(void);
