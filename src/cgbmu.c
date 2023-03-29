#include "board.h"
#include "cartridge.h"
#include "cgbmu.h"
#include "video.h"
#include "dmgcpu.h"
#include "debug.h"
#include "decoder.h"

enum{ SINGLE_FRAME, SINGLE_STEP};
//----------------------------------------------------*/
//
//------------------------------------------------------
void runCpu(uint32_t nTicks) {
uint32_t elapsed_cycles = 0;
	while (nTicks > elapsed_cycles) {
		decode();
		timer();
		interrupts();		
		elapsed_cycles += instr_cycles;
	}
}
//-----------------------------------------
//
//-----------------------------------------
void runOneFrame(void) {

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
		runCpu(V_M1_CYCLE);
		IOLY++;
	}
}

void cgbmu(const uint8_t *rom) {
	uint32_t ticks;
	uint8_t mode = !SINGLE_FRAME;
	
	if(rom == NULL){
		bootCpu();
	}else{
		initCpu();
		cartridgeInit(rom);
	}

	if (mode == SINGLE_STEP) {			// instruction loop
		ticks = GetTick();
		while (readButtons() != 255) {
			decode();
			timer();
			if (video() == true) {
				ticks = GetTick() - ticks;
				if (ticks < FRAME_TIME)
					DelayMs(FRAME_TIME - ticks);
				ticks = GetTick();
				DBG_Fps();
			}
			interrupts();
		}
	}
	else {				// frame loop
		while (readButtons() != 255) {
#if defined(_WIN32) || defined(linux)
		int startTicks = GetTick();
#endif
			runOneFrame();
			//VIDEO_Update();
			DBG_Fps();

#if defined(_WIN32) || defined(linux)
			int delta = GetTick() - startTicks;
			if (delta < FRAME_TIME) {
				DelayMs(FRAME_TIME - delta);
			}
#endif
		}
	}	
}
