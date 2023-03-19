#ifndef _cgbmu_h_
#define _cgbmu_h_

#include <stdint.h> 

#define FRAME_TIME 16

void cgbmu(void);
int loadRom(char *fn);
void prepareFrame(void);
void pushScanLine(uint8_t *scanline);
uint8_t readJoyPad(void);

#endif /* _common_h_ */
