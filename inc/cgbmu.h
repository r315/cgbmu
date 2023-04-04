#ifndef _cgbmu_h_
#define _cgbmu_h_

#include <stdint.h> 

#define FRAME_TIME 16

void cgbmuExit(void);
void cgbmu(const uint8_t *rom);
uint8_t runOneStep(void);
uint8_t runOneFrame(void);
void updateFps(void);
// Implemented by target
int loadRom(char *fn);
uint8_t readButtons(void);
void pushScanLine(uint8_t *scanline);
int drawInt(int x, int y, unsigned int v, char radix, char digitos);
#endif /* _common_h_ */
