#pragma once
#include <stdbool.h>
#include <stdint.h>

#define DBG_UART

extern long done;
extern bool uartTxDone;

enum {COLOR_RED, COLOR_GREEN, COLOR_BLUE};

//extern const UART_HandleTypeDef* dbgUartPtr;

inline uint16_t sadd16(uint16_t a, uint16_t b);
inline uint32_t sadd32(uint32_t a, uint32_t b);

void dbgTx(char* text, int len);

void debugPrintColor(int color, const char *fmt, ...);
void debugPrint(const char *fmt, ...);
void debugClearTerminal(void);
void debugPrintFast(const char *fmt, ...);

void testMethod(void);
float msPerOperation(int times);

void debugPinInit(void);
void debugPinSet(bool i);
void debugInit(void);


