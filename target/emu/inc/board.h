#ifndef _board_h_
#define _board_h_

//#include "libemb.h"
//#include "lcdsdl.h"

#ifdef _WIN32
	#include <SDL.h>
	//#pragma warning(disable:4996)
	#include <direct.h>
#elif defined(__linux__)
	#include <SDL.h>
#endif

#define LCD_W		160
#define LCD_H		144
#define LCD_SIZE    (LCD_W * LCD_H)

#define FAST_CODE
#define DBG_PIN_TOGGLE

#define GetTick SDL_GetTicks
#define DelayMs	SDL_Delay

#define SCREEN_OFFSET_X		80
#define SCREEN_OFFSET_Y		48

enum { OFF = 0, ON };

enum {
	LCD_PORTRAIT = 0,
	LCD_LANDSCAPE,
	LCD_REVERSE_PORTRAIT,
	LCD_REVERSE_LANDSCAPE
};

void LCD_Close(void);
void LCD_Init(void);
void LCD_Update(void);
void LCD_Data(uint16_t color);
void LCD_Push(void);
void LCD_Pop(void);
#endif
