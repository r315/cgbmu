#ifndef _cgbmu_h_
#define _cgbmu_h_

#include <stdint.h> 

#define FRAME_TIME 16

void cgbmu(const uint8_t *rom);
int loadRom(char *fn);
void pushScanLine(uint8_t *scanline);
uint8_t readButtons(void);
#endif /* _common_h_ */
