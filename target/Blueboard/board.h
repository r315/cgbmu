/**
 * Blueboard HW definitions for GB emulator
 * 
 * */


#ifndef _board_h_
#define _board_h_

#include "blueboard.h"

#define DBG_PIN (1<<12)
#define DBG_PIN_INIT  LPC_GPIO2->FIODIR |= DBG_PIN
#define DBG_PIN_TOGGLE LPC_GPIO2->FIOPIN ^= DBG_PIN

#define USE_TIMER_SYSTICK	1

void BOARD_Init(void);
void BB_ConfigPLL(uint8_t fmhz);
void __debugbreak(void);

#endif
