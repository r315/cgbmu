#ifndef _common_h_
#define _common_h_


#if defined(__EMU__)
	#ifdef _WIN32
		#include <SDL.h>
		#pragma warning(disable:4996)
		#include <direct.h>
	#elif defined(__x86_64__)
		#include <SDL2/SDL.h>
	#endif
	
	#include "disassembler.h"
	#define GetTicks() SDL_GetTicks()
	#define DelayMs(x) SDL_Delay(x)
	#define FRAME_TIME 16
	
#elif defined(__BB__)
	#include <blueboard.h>
	#define REGISTERS_ROW 11
	#define LCD_Push()
	#define LCD_Pop()
#endif

#endif /* _common_h_ */
