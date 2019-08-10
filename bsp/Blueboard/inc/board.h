/**
 * Blueboard HW definitions for GB emulator
 * 
 * */


#ifndef _board_h_
#define _board_h_

#include <libemb.h>
#include <blueboard.h>
#include "pff/pff.h"


#define DBG_PIN (1<<12)
#define DBG_PIN_INIT  \
{                     \
	LPC_GPIO2->FIODIR |= DBG_PIN; \
}

#define DBG_PIN_TOGGLE \
{                      \
	LPC_GPIO2->FIOPIN ^= DBG_PIN; \
} 

#ifdef USE_FAST_CODE
#define FAST_CODE __attribute__ ((section(".fastcode")))
#else
#define FAST_CODE
#endif
void BOARD_Init(void);
#endif
