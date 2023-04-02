#include "board.h"
#include "cartridge.h"
#include "cgbmu.h"
#include "video.h"
#include "dmgcpu.h"
#include "decoder.h"

enum{ SINGLE_FRAME, SINGLE_STEP};

uint8_t done;

void updateFps(void) {
	static uint32_t fpsupdatetick = 0;
	static uint16_t fps = 0;
	fps++;

	if (GetTick() > fpsupdatetick)
	{
		drawInt(SCREEN_W + 8, 0, fps, 10, 4);
		fps = 0;
		fpsupdatetick = GetTick() + 1000;
	}
}
/**
 * @brief 
 * 
 * @param nTicks 
 */
static void runCpu(uint32_t nTicks) {
uint32_t elapsed_cycles = 0;
	while (nTicks > elapsed_cycles) {
		decode();
		timer();
		interrupts();		
		elapsed_cycles += instr_cycles;
	}
}

/**
 * @brief 
 * 
 */
uint8_t runOneFrame(void) {

	for (; IOLY < SCREEN_H; IOLY++) {

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

	return 1;
}

/**
 * @brief 
 * 
 */
uint8_t runOneStep(void) {
	uint8_t frame;
	decode();
	frame = video();
	timer();
	serial();
	interrupts();
	return frame;
}

/**
 * @brief 
 * 
 * @param rom 
 */
void cgbmu(const uint8_t *rom) {
	uint8_t mode = SINGLE_STEP;
	uint32_t ticks = 0;
	done = 0;
	
	if(rom == NULL){
		bootCpu();
	}else{
		initCpu();
		cartridgeInit(rom);
	}
	
	if (mode == SINGLE_STEP) {			// instruction loop		
		while (!done) {
			if(runOneStep()){			
				ticks = GetTick() - ticks;
				updateFps();
				if (ticks < FRAME_TIME){
					DelayMs(FRAME_TIME - ticks);
				}
				ticks = GetTick();
			}
		}
	}
	else {				// frame loop
		while (!done) {
			runOneFrame();
		}
	}	
}
