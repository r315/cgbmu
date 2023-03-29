#include "board.h"
#include "cartridge.h"
#include "cgbmu.h"
#include "video.h"
#include "dmgcpu.h"
#include "decoder.h"

enum{ SINGLE_FRAME, SINGLE_STEP};


void updateFps(void) {
	static uint32_t fpsupdatetick = 0;
	static uint16_t fps = 0;
	fps++;

	if (GetTick() > fpsupdatetick)
	{
		drawInt(SCREEN_W + 1, 0, fps, 10, 4);
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

/**
 * @brief 
 * 
 */
void runOneStep(void) {
	decode();
	if (video()) {
		updateFps();
	}
	timer();
	// serial
	interrupts();
}

/**
 * @brief 
 * 
 * @param rom 
 */
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
				updateFps();
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
			updateFps();

#if defined(_WIN32) || defined(linux)
			int delta = GetTick() - startTicks;
			if (delta < FRAME_TIME) {
				DelayMs(FRAME_TIME - delta);
			}
#endif
		}
	}	
}
