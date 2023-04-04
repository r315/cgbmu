#ifndef _cgbmu_h_
#define _cgbmu_h_

#include <stdint.h>
#include "dmgcpu.h" 

#define FRAME_TIME 16

void cgbmuExit(void);
void cgbmu(const uint8_t *rom);
uint8_t runOneStep(void);
uint8_t runOneFrame(void);
void updateFps(void);

// Implemented by target
uint32_t GetTick(void);
void DelayMs(uint32_t ms);
int loadRom(const uint8_t **dst, const char *rom_name);
uint8_t readButtons(void);
void pushScanLine(cpu_t *cpu);
int drawInt(int x, int y, unsigned int v, char radix, char digitos);
#endif /* _common_h_ */
