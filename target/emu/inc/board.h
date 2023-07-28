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

#define LCD_AUTO_UPDATE_TIME    0 //30ms => 33fps, 0: manual update 
#define LCD_W		320
#define LCD_H		144
#define LCD_SIZE    (LCD_W * LCD_H)

#define DBG_PIN_TOGGLE

#define SCREEN_OFFSET_X		0
#define SCREEN_OFFSET_Y		0

enum { OFF = 0, ON, false = OFF, true = ON };

enum {
	LCD_PORTRAIT = 0,
	LCD_LANDSCAPE,
	LCD_REVERSE_PORTRAIT,
	LCD_REVERSE_LANDSCAPE
};

void LCD_Close(void);
void LCD_Init(void *);
void LCD_Update(void);
void LCD_Data(uint16_t color);
void LCD_WriteArea(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t *data);
void LCD_Window(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
#endif
