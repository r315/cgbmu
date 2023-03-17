#ifndef _cgbmu_h_
#define _cgbmu_h_

#include <stdint.h> 

#define FRAME_TIME 16

#if defined(__EMU__)	
	#include "disassembler.h"
	#define GetTick() SDL_GetTicks()
	#define DelayMs(x) SDL_Delay(x)
	#define FRAME_TIME 16	
#else
	#define REGISTERS_ROW 11
	#define LCD_Push()
	#define LCD_Pop()
#endif

uint8_t readJoyPad(void);
int loadRom(char *fn);
int loadRombank(uint8_t bank);
void cgbmu(void);

#endif /* _common_h_ */
