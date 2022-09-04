#include "main.h"
#include "eeprom.h"
#include <stdbool.h>

enum {	EEP_PAGE_ACTIVE = 0xBABEBABE,
		EEP_PAGE_ERASED = 0x600DF00D,
		EEP_PAGE_ERROR = 0xBAADF00D,
		EEP_PAGE_MARKER = 0x600D600D
};

enum {EEP_STATE_INIT, EEP_STATE_OK, EEP_STATE_LOAD_FAILED};
enum menuDataType {TYPE_BOOL, TYPE_INT, TYPE_FLOAT};

enum eepVariables {	EEVAR_TEST1, EEVAR_TEST2, EEVAR_TEST3,
					EEVAR_NR };

typedef struct {
	void *dataPtr;
	uint8_t dataType;
} eepVariable_t;

float 	testDataF = 0.12345f;
int 	testDataI = -555;
bool	testDataB = 1;

eepVariable_t eepVar[EEVAR_NR] = {
		{.dataPtr = &testDataB, .dataType = TYPE_BOOL},
		{.dataPtr = &testDataF, .dataType = TYPE_FLOAT},
		{.dataPtr = &testDataI, .dataType = TYPE_INT},
};


void eepInit(void){
	// find last written page:
	// try to read header of type EEP_PAGE_ACTIVE
	// if _ACTIVE, read CRC (first bytes)
	// calculate CRC and compare
	// if CRC ok:
	// set activePage to it
	// set eepState = EEP_STATE_OK
	// if CRC not ok, set eepState = EEP_STATE_LOAD_FAILED
}

void eepLoadAll(void){
	// check eepState, if EEP_STATE_LOAD_FAILED - set variables to default values
	// otherwise load all data
}

void eepWrite(void){
	// erase activePage
	// find next page with type EEP_PAGE_ERASED
	// write all vars
}

uint32_t pageHeader[3]; // marker, status, CRC
void eepTest(void){
	static FLASH_EraseInitTypeDef EraseInitStruct;

	EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
	EraseInitStruct.PageAddress = EEP_START_ADDR;
	EraseInitStruct.NbPages = EEP_PAGES;

	uint32_t page_error = 0;

	HAL_FLASH_Unlock();
		if(HAL_FLASHEx_Erase(&EraseInitStruct, &page_error) != HAL_OK){
			 uint32_t er = HAL_FLASH_GetError();
		}


		// calculate CRC
		uint32_t dataToSave[EEVAR_NR];
		for (int i=0; i<EEVAR_NR; i++)
			dataToSave[i] = *(uint32_t*)eepVar[i].dataPtr;

		pageHeader[0] = EEP_PAGE_MARKER;
		pageHeader[1] = HAL_CRC_Calculate(&hcrc, dataToSave, EEVAR_NR);
		pageHeader[2] = EEP_PAGE_ACTIVE;

		// write header
		uint32_t currAddr = EEP_START_ADDR;
		for(uint8_t i = 0; i < 3; i++){
			  if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, currAddr, pageHeader[i]) != HAL_OK){
				  uint32_t er = HAL_FLASH_GetError();
			  }
			  currAddr += 4;
		}

		// write data
		currAddr = (EEP_START_ADDR + 16);
		for(uint8_t i = 0; i < EEVAR_NR; i++){
			  if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, currAddr, dataToSave[i]) != HAL_OK){
				  uint32_t er = HAL_FLASH_GetError();
			  }
			  currAddr += 4;
		}
	HAL_FLASH_Lock();
}
