#include "tools.h"
#include "main.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

inline uint16_t sadd16(uint16_t a, uint16_t b) {
  return (a > 0xFFFF - b) ? 0xFFFF : a + b;
}

inline uint32_t sadd32(uint32_t a, uint32_t b) {
  return (a > 0xFFFFFFFF - b) ? 0xFFFFFFFF : a + b;
}

char dbgOutBuf[256] = {0};
volatile uint32_t lastTime = 0;
bool uartTxDone = 1;
long done = 0;

UART_HandleTypeDef* dbgUartPtr = &huart1;

float audioLoad = 0;
float everythingElseLoad = 0;

void debugClearTerminal(void){
	char buffer[] = "\x1b[2J";//"\033[H";
	char buffer2[] = "\x1b[H"; //"\x1b[H";

	int result;

#ifdef DBG_UART
	HAL_UART_Transmit(dbgUartPtr, buffer, strlen(buffer), 2);
	HAL_Delay(1);
	HAL_UART_Transmit(dbgUartPtr, buffer2, strlen(buffer2), 2);
	HAL_Delay(1);
#endif

#ifdef DBG_USB_UART
	do result = CDC_Transmit_FS((unsigned char*)buffer, strlen(buffer));
		while (result != USBD_OK);
		HAL_Delay(10);
	do result = CDC_Transmit_FS((unsigned char*)buffer2, strlen(buffer2));
		while (result != USBD_OK);
#endif


}

/*
void debugPrintFast(const char *fmt, ...){
	va_list args;
	va_start(args, fmt);
	int rc = vsnprintf(	gfxItems[G_DEBUG_STRING].text,
						sizeof(gfxItems[G_DEBUG_STRING].text),
						fmt,
						args);
	va_end(args);
	char nlBuf[] = {"\r\n"};
	strcat(gfxItems[G_DEBUG_STRING].text, nlBuf);
	gfxItems[G_DEBUG_STRING].pendUpd = 1;
}
*/

void debugPrint(const char *fmt, ...){

	//static char timBuf[32];
	//sprintf(timBuf, "+%u ms: ", (HAL_GetTick() - lastTime));
	
	static char txtBuf[64];
    va_list args;
    va_start(args, fmt);
    int rc = vsnprintf(txtBuf, sizeof(txtBuf), fmt, args);
    va_end(args);
	
    const char nlBuf[] = {"\r\n"};

    static char output[70];
    *output = "";
	//output = malloc(strlen(timBuf) + strlen(txtBuf) + strlen(nlBuf) + 1); /* make space for the new string (should check the return value ...) */
	//strcpy(output, timBuf);
	strcpy(output, txtBuf);
	strcat(output, nlBuf); 

	lastTime = HAL_GetTick();
	
#ifdef DBG_UART
	HAL_UART_Transmit(dbgUartPtr, output, strlen(output), 2);
#endif


#ifdef DBG_USB_UART
	int result;
	do result = CDC_Transmit_FS((unsigned char*)output, strlen(output));
	while (result != USBD_OK);
#endif

#ifdef DBG_LCD
	gfxLabelSet(G_DEBUG_STRING, output);
#endif

	free(output);
}



void debugPrintColor(int color, const char *fmt, ...){
	
	uint32_t timeNow = HAL_GetTick();
	
	//char timBuf[32];
	//sprintf(timBuf, "+%u ms: ", (timeNow - lastTime));
	
	char txtBuf[32];
    va_list args;
    va_start(args, fmt);
    int rc = vsnprintf(txtBuf, sizeof(txtBuf), fmt, args);
    va_end(args);
	
	//CDC_Transmit_FS((unsigned char*)txtBuf, strlen(txtBuf));
	////HAL_Delay(100);
	char nlBuf[] = {"\r\n"};

	char* output;
	output = malloc(10 + /*strlen(timBuf)*/ + strlen(txtBuf) + strlen(nlBuf) + 1); /* make space for the new string (should check the return value ...) */
		
	switch (color){
		case COLOR_RED: {strcpy(output, "\033[0;31m"); break;}
		case COLOR_GREEN: {strcpy(output, "\033[0;32m"); break;}
		case COLOR_BLUE: {strcpy(output, "\033[1;34m"); break;}
	}
	
	//strcat(output, timBuf);
	strcat(output, txtBuf); 
	strcat(output, nlBuf); 
	strcat(output, "\033[0m");
	
	
	//((USBD_CDC_HandleTypeDef*)(USBD_Device.pClassData))->TxState
	
	lastTime = timeNow;
	
	#ifdef DBG_UART
		HAL_UART_Transmit(dbgUartPtr, output, strlen(output), 2);
	#endif

	/*
	#ifdef DBG_USB_UART
		int result;
		do result = CDC_Transmit_FS((unsigned char*)output, strlen(output));
		while (result != USBD_OK);
	#endif
	*/

	//free(output);
}

void debugPrintSameLine(const char *fmt, ...){
	char buffer[512];
    va_list args;
    va_start(args, fmt);
    int rc = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
	CDC_Transmit_FS((unsigned char*)buffer, strlen(buffer));
	HAL_Delay(1);
}


float msPerOperation(int times){
	int time1 = HAL_GetTick();
	for (int i =0; i<times; i++)
		testMethod();

	int timeMs = HAL_GetTick() - time1;
	float timePerOp = (float)timeMs / times;
	return timePerOp;
}



//#include "gpio.h"

#define DEBUG_PIN				GPIO_PIN_7
#define DEBUG_PIN_PORT 	GPIOC

GPIO_InitTypeDef GPIO_InitStruct_ = {0};
	
void debugPinInit(void){
	__HAL_RCC_GPIOC_CLK_ENABLE();
	
	GPIO_InitStruct_.Pin = DEBUG_PIN;
  GPIO_InitStruct_.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct_.Pull = GPIO_NOPULL;
  GPIO_InitStruct_.Speed = GPIO_SPEED_FREQ_HIGH;

  HAL_GPIO_Init(DEBUG_PIN_PORT, &GPIO_InitStruct_);
}


void debugPinSet(bool i){
	HAL_GPIO_WritePin(DEBUG_PIN_PORT, DEBUG_PIN, i); 
}

void debugInit(void){
	//HAL_Delay(500);
	//debugClearTerminal();
	HAL_Delay(500);
	debugPrintColor(COLOR_GREEN, "hello!");
	debugPrintColor(COLOR_BLUE, "%u", HAL_RCC_GetSysClockFreq());
	//debugPinInit();
}

