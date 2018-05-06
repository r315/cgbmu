#ifndef _cgbmu_h_
#define _cgbmu_h_

#include <stdint.h> 

#define FRAME_TIME 16

#if defined(__EMU__)
	#ifdef _WIN32
		#include <SDL.h>
		#pragma warning(disable:4996)
		#include <direct.h>
	#elif defined(__linux__)
		#include <SDL2/SDL.h>
	#endif
	
	#include "disassembler.h"
	#define GetTicks() SDL_GetTicks()
	#define DelayMs(x) SDL_Delay(x)
	#define FRAME_TIME 16
	
#else
	#define REGISTERS_ROW 11
	#define LCD_Push()
	#define LCD_Pop()
#endif

// these functions must be implemented by the target arch
uint8_t readJoyPad(void);
int loadRom(char *fn);
int loadRombank(uint8_t bank);

#endif /* _common_h_ */
