#include "board.h"
#include "cartridge.h"
#include "cgbmu.h"
#include "video.h"
#include "dmgcpu.h"
#include "decoder.h"

enum{ SINGLE_FRAME, SINGLE_STEP};

static uint8_t done;
static cpu_t dmgcpu;

void cgbmuExit(void){
	done = true;
}

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

#if 0
/**
 * @brief 
 * 
 * @param nTicks 
 */
static void runCpu(uint32_t nTicks) {
	static uint32_t elapsed_cycles = 0;
	while (elapsed_cycles < nTicks) {
		decode(&dmgcpu);
		timer(&dmgcpu);
		serial(&dmgcpu);
		interrupts(&dmgcpu);
		elapsed_cycles += dmgcpu.instr_cycles;
	}
	elapsed_cycles -= nTicks;
}

/**
 * @brief should be faster, however does not work
 * properly
 * 
 */
uint8_t runOneFrame(void) {

	dmgcpu.IOSTAT &= 0xFC;

	for (dmgcpu.IOLY = 0; dmgcpu.IOLY < SCREEN_H; IOLY++) {

		checkLine(dmgcpu.IOLY);

		dmgcpu.IOSTAT |= V_M2;  			// Mode2 scan OAM
		if (IOSTAT & OAM_IE)
			setInt(LCDC_IF);

		runCpu(V_M2_CYCLE);
		scanOAM();

		IOSTAT |= V_M3;  			// Mode3 scan VRAM
		runCpu(V_M3_CYCLE);
		scanline();

		IOSTAT &= ~(V_MODE_MASK); 	// Change to Mode0 H-Blank
		if (IOSTAT & HB_IE)			// check H-Blank IE
			setInt(LCDC_IF);
		runCpu(V_M0_CYCLE);
	}

	IOSTAT |= V_M1;  		// Change to Mode 1
	IOIF |= V_BLANK_IF;		// V-Blank Flag is Always activated
	if (IOSTAT & VB_IE)		// LCD Flag is activated if IE is enabled
		setInt(LCDC_IF);

	while (IOLY < (SCREEN_H + VBLANK_LINES)) {
		IOLY = checkLine(IOLY + 1);
		runCpu(V_M1_CYCLE);
	}

	return 1;
}
#endif
/**
 * @brief 
 * 
 */
uint8_t runOneStep(void) {
	uint8_t frame;
	decode(&dmgcpu);
	frame = video(&dmgcpu);
	timer(&dmgcpu);
	serial(&dmgcpu);
	interrupts(&dmgcpu);
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
	done = false;
	
	if(rom == NULL){
		bootCpu(&dmgcpu);
	}else{
		initCpu(&dmgcpu);
		cartridgeInit(&dmgcpu, rom);
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
			//runOneFrame();
			//updateFps();
		}
	}	
}
