#include "board.h"
#include "cgbmu.h"
#include "video.h"
#include "dmgcpu.h"
#include "debug.h"
#include "decoder.h"

//----------------------------------------------------*/
//
//------------------------------------------------------
FAST_CODE
void runCpu(uint32_t nTicks) {
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

	prepareFrame();

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
void cgbmu(void) {
	uint32_t ticks;
	uint8_t mode = 0;
	
	initCpu();

	if (mode) {			// instruction loop
		ticks = GetTick();
		while (readJoyPad() != 255) {
			decode();
			interrupts();
			timer();
			if (video() == ON) {
				ticks = GetTick() - ticks;
				if (ticks < FRAME_TIME)
					DelayMs(FRAME_TIME - ticks);
				ticks = GetTick();
				DBG_Fps();
				//DBG_PIN_TOGGLE;
			}
		}
	}
	else {				// frame loop
		while (readJoyPad() != 255) {
#if defined(__EMU__)
		int startTicks = GetTick();
#endif
			runOneFrame();
			DBG_Fps();
			DBG_PIN_TOGGLE;

#if defined(__EMU__)			
			int delta = GetTick() - startTicks;
			if (delta < FRAME_TIME) {
				DelayMs(FRAME_TIME - delta);
			}
#endif
		}
	}	
}
