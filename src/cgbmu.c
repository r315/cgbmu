#include <stddef.h>
#include "cartridge.h"
#include "cgbmu.h"
#include "video.h"
#include "dmgcpu.h"
#include "decoder.h"

enum cgbmustates {
    EMU_STATE_INIT,
    EMU_STATE_BOOT_ROM,
    EMU_STATE_SINGLE_FRAME,
    EMU_STATE_LIMITED_RUN,
    EMU_STATE_FAST_RUN,
    EMU_STATE_ENDED
};

static cpu_t dmgcpu;
static enum cgbmustates state;
static uint16_t fps, fps_counter;

static uint8_t updateFps(void)
{
    static uint32_t fpsupdatetick = 0;
    fps_counter++;

	if (GetTick() > fpsupdatetick){
		fpsupdatetick = GetTick() + 1000;
        fps = fps_counter;
		fps_counter = 0;
        return 1;
	}

    return 0;
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
 * @param
 * @return
 */
enum videoint cgbmuSingle(void)
{
    enum videoint vid;

    decode(&dmgcpu);
    vid = video(&dmgcpu);
    timer(&dmgcpu);
    serial(&dmgcpu);
    interrupts(&dmgcpu);

    return vid;
}

/**
 * @brief
 * @param
 * @return
 */
uint16_t cgbmuFps(void)
{
    return fps;
}

/**
 * @brief
 * @param
 * @return
 */
const uint8_t* cgbmuLine(void)
{
    return dmgcpu.screen_line;
}

/**
 * @brief
 */
enum emures cgbmuInit(const uint8_t *rom)
{
    if(rom == NULL){
        cartridgeInit(&dmgcpu, boot_rom);
		state = EMU_STATE_BOOT_ROM;
	}else{
		cartridgeInit(&dmgcpu, rom);
        state = EMU_STATE_INIT;
	}

    return EMU_RES_OK;
}

/**
 * @brief
 *
 * @param rom
 */
enum emures cgbmu(void)
{
	static uint32_t ticks = 0;
    static uint8_t frame = 0;
    enum emures res = EMU_RES_OK;
    enum videoint vid = VIDEO_NONE;

    switch(state){
        case EMU_STATE_BOOT_ROM:
            initCpu(&dmgcpu);
            dmgcpu.PC = 0;
            state = EMU_STATE_LIMITED_RUN;
            break;

        case EMU_STATE_INIT:
            initCpu(&dmgcpu);
            state = EMU_STATE_FAST_RUN;
            break;

        case EMU_STATE_LIMITED_RUN:

            if (!frame){
                vid = cgbmuSingle();
                frame = vid == VIDEO_VBLANK ? 1 : 0;
            }

            if(GetTick() - ticks > FRAME_TIME)
            {
                ticks = GetTick();
                frame = 0;
            }
            break;

        case EMU_STATE_FAST_RUN:
            vid = cgbmuSingle();
            break;

        case EMU_STATE_ENDED:
            return EMU_RES_END;

        default:
            break;
    }

    if(vid == VIDEO_VBLANK){
        if(updateFps()){
            return EMU_RES_VBLANK;
        }
    }else if(vid == VIDEO_HBLANK){
        scanlineDraw(&dmgcpu);
        return EMU_RES_HBLANK;
    }

    return res;
}

/**
 * @brief
 * @param
 */
void cgbmuAbort(void)
{
	state = EMU_STATE_ENDED;
}