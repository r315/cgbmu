#ifndef _lcd_h_
#define _lcd_h_

#if defined(__EMU__)
	#if defined(__LCDFB__)
	#include <lcdfb.h>
	#else
	#include "lcdsdl.h"
	#endif
#elif defined(__LPCXpresso__)
    #include "ili9341.h"
#else
    #include <LPC17xx.h>
    #define GPIO0 LPC_GPIO0
    #define GPIO1 LPC_GPIO1
    #include "ili9328.h"
    #include <string.h>
    #include <stdio.h>
#endif

#ifndef ON
	#define ON 1
	#define OFF 0
#endif

#ifdef BGR_MODE
    #define RGB(r,g,b)  (int)((b<<11) | (g<<5) | r)
#else
    #define RGB(r,g,b)  (int)((r<<11) | (g<<5) | b)
#endif

//-------------------------------------------------------------------
//	basic colors 
//			16bit
//		BBBBBGGGGGGRRRRR
//-------------------------------------------------------------------
/*****************************************************
// color names
// http://www.w3schools.com/html/html_colornames.asp
*****************************************************/
#define RED         RGB(31,0,0)
#define GREEN       RGB(0,63,0)
#define BLUE        RGB(0,0,31) 
#define BLACK       RGB(0,0,0)
#define WHITE       RGB(31,63,31)
#define GRAY        RGB(16,32,16)
#define YELLOW      RGB(31,63,0)
#define	ORANGE      RGB(31,32,0)
#define CYAN        RGB(0,63,31)
#define PINK        RGB(31,0,31)
#define FBBLUE      RGB(7,22,19)
#define SILVER      0xC618
#define SKYBLUE     0x867D
#define ROYALBLUE   0x435C
#define TOMATO      0xFB08
#define GOLD        0xFEA0
#define GREENYELLOW 0xAFE5
#define DIMGRAY     0x6B4D
#define DARKORANGE  0xFC60
#define TEAL        0x410


/**
 @brief 
 **/
void LCD_Init(void);

/**
 @brief Draws a filled rectangle
 **/
void LCD_FillRect(uint16_t x, uint16_t y,  uint16_t w, uint16_t h, uint16_t color);

/**
 @brief Draws a filled rectangle with round corners
 **/
void LCD_FillRoundRect(uint16_t x, uint16_t y,  uint16_t w, uint16_t h, uint16_t color);

/**
 @brief Draws a rectangle
 **/
void LCD_Rect(uint16_t x, uint16_t y,  uint16_t w, uint16_t h, uint16_t color);

/**
 @brief Opens a window for writing data
 **/
void LCD_Window(uint16_t x, uint16_t y, uint16_t w, uint16_t h);


/**
 @brief Open a 1x1 px window, addresses the pixel location and write data
 **/
void LCD_Pixel(uint16_t x, uint16_t y, uint16_t color);

/**
 @brief Draws a line
 **/
void LCD_Line(uint16_t x1, uint16_t y1,  uint16_t x2, uint16_t y2, uint16_t color);

/**
 @brief 
 **/
void LCD_Clear(uint16_t color);

/**
 @brief 
 **/
void LCD_Bkl(uint8_t state);

/**
* @brief
**/
void LCD_Scroll(uint16_t sc);

#endif
