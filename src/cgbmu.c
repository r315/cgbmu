#include "board.h"
#include <cgbmu.h>
#include <video.h>
#include <dmgcpu.h>
#include <debug.h>
#include <decoder.h>

//----------------------------------------------------*/
//
//------------------------------------------------------
FAST_CODE
inline void runCpu(uint32_t nTicks) {
uint32_t elapsed_cycles = 0;
	while (nTicks > elapsed_cycles) {
		timer();
		interrupts();
		decode();
		elapsed_cycles += CYCLES_COUNT;
	}
}
//-----------------------------------------
//
//-----------------------------------------
FAST_CODE
void runOneFrame(void) {

	LCD_Window(SCREEN_OFFSET_X, SCREEN_OFFSET_Y, SCREEN_W, SCREEN_H);  //3us

	IOSTAT &= ~(V_MODE_MASK);

	for (IOLY = 0; IOLY < SCREEN_H; IOLY++) {

		IOSTAT = (IOLY == IOLYC) ? (IOSTAT | LYC_LY_FLAG) : (IOSTAT & (~LYC_LY_FLAG));

		//if ((IOSTAT & LYC_LY_IE) && (IOSTAT & LYC_LY_FLAG))
		if (IOSTAT & LYC_LY_FLAG)
			IOIF |= LCDC_IF;

		IOSTAT |= V_M2;  			// Change to Mode2 scan OAM
		if (IOSTAT & OAM_IE)			// check OAM IE
			IOIF |= LCDC_IF;
		runCpu(V_M2_CYCLE);
		scanOAM();

		IOSTAT |= V_M3;  			// Change to Mode3 scan VRAM
		runCpu(V_M3_CYCLE);
		scanline();

		IOSTAT &= ~(V_MODE_MASK); 	// Change to Mode0 H-Blank
		if (IOSTAT & HB_IE)			// check H-Blank IE
			IOIF |= LCDC_IF;
		runCpu(V_M0_CYCLE);

	}

	IOSTAT |= V_M1;  		// Change to Mode 1
	IOIF |= V_BLANK_IF;		// V-Blank Flag is Always activated
	if (IOSTAT & VB_IE)		// LCD Flag is activated if IE is enabled
		IOIF |= LCDC_IF;

	while (IOLY < (SCREEN_H + VBLANK_LINES)) {
		IOSTAT = (IOLY == IOLYC) ? (IOSTAT | LYC_LY_FLAG) : (IOSTAT & ~LYC_LY_FLAG);
		runCpu(V_LINE_CYCLE);
		IOLY++;
	}
}

FAST_CODE
void cgbmu(uint8_t mode) {
	uint32_t ticks;

	initCpu();

	if (mode) {			// instruction loop
		ticks = GetTicks();
		while (readJoyPad() != 255) {
			decode();
			interrupts();
			timer();
			if (video() == ON) {
				ticks = GetTicks() - ticks;
				if (ticks < FRAME_TIME)
					DelayMs(FRAME_TIME - ticks);
				ticks = GetTicks();
				//DBG_Fps();
				//DBG_PIN_TOGGLE;
			}
		}
	}
	else {				// frame loop
		while (readJoyPad() != 255) {
			runOneFrame();
			DBG_Fps();
			DBG_PIN_TOGGLE;
		}
	}	
}
