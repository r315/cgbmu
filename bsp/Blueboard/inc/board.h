/**
 * Blueboard HW definitions for GB emulator
 * 
 * */


#ifndef _board_h_
#define _board_h_

#include <libemb.h>
#include <blueboard.h>
#include "pff/pff.h"

#define PLL48   0
#define PLL72   1
#define PLL80   2
#define PLL100  3


#define XTAL        (12000000UL)        /* Oscillator frequency               */
#define OSC_CLK     (      XTAL)        /* Main oscillator frequency          */
#define RTC_CLK     (   32000UL)        /* RTC oscillator frequency           */
#define IRC_OSC     ( 4000000UL)        /* Internal RC oscillator frequency   */

#endif
