#ifndef _cgbmu_h_
#define _cgbmu_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "dmgcpu.h"

#define FRAME_TIME 16

enum emures{
    EMU_RES_OK,
    EMU_RES_BOOT_ROM,
    EMU_RES_VBLANK,
    EMU_RES_HBLANK,
    EMU_RES_END
};

enum emures cgbmuInit(const uint8_t *rom);
enum emures  cgbmu(void);
void cgbmuAbort(void);
uint16_t cgbmuFps(void);
const uint8_t* cgbmuLine(void);

// Implemented by target
uint32_t GetTick(void);
void DelayMs(uint32_t ms);
uint8_t readButtons(void);
void scanlineDraw(cpu_t *cpu);

#ifdef __cplusplus
}
#endif

#endif /* _common_h_ */
