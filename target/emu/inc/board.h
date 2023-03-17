#ifndef _board_h_
#define _board_h_

#include <libemb.h>
#include <lcdsdl.h>

#ifdef _WIN32
	#include <SDL.h>
	//#pragma warning(disable:4996)
	#include <direct.h>
#elif defined(__linux__)
	#include <SDL.h>
#endif

#define LCD_W 160
#define LCD_H 144

#define FAST_CODE
#define DBG_PIN_TOGGLE
#endif
