#ifndef _liblcd_h_
#define _liblcd_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

//-------------------------------------------------------------------
//	basic colors 
//			16bit
//		BBBBBGGGGGGRRRRR
//-------------------------------------------------------------------
/*****************************************************
// color names
// http://www.w3schools.com/html/html_colornames.asp
*****************************************************/
#define LCD_RED         RGB565(255,0,0)
#define LCD_GREEN       RGB565(0,255,0)
#define LCD_BLUE        RGB565(0,0,255)
#define LCD_BLACK       RGB565(0,0,0)
#define LCD_WHITE       RGB565(255,255,255)
#define LCD_GRAY        RGB565(128,128,128)
#define LCD_YELLOW      RGB565(255,255,0)
#define LCD_ORANGE      RGB565(255,128,0)
#define LCD_CYAN        RGB565(0,255,255)
#define LCD_PINK        RGB565(255,0,255)
#define LCD_FBBLUE      RGB565(56,88,152)
#define LCD_SILVER      0xC618
#define LCD_SKYBLUE     0x867D
#define LCD_ROYALBLUE   0x435C
#define LCD_TOMATO      0xFB08
#define LCD_GOLD        0xFEA0
#define LCD_GREENYELLOW 0xAFE5
#define LCD_DIMGRAY     0x6B4D
#define LCD_DARKORANGE  0xFC60
#define LCD_TEAL        0x0410

#ifdef BGR_MODE
    #define RGB565(r,g,b)  (uint16_t)(((b & 0xF8) << 8) | ((g & 0xFC) << 3) | (r & 0xF8) >> 3)
#else
    #define RGB565(r,g,b)  (uint16_t)(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b & 0xF8) >> 3)
#endif

void LCD_Init(void *param);
void LCD_Rect(uint16_t x, uint16_t y,  uint16_t w, uint16_t h, uint16_t color);
void LCD_FillRect(uint16_t x, uint16_t y,  uint16_t w, uint16_t h, uint16_t color);
void LCD_FillRoundRect(uint16_t x, uint16_t y,  uint16_t w, uint16_t h, uint16_t color);
void LCD_Pixel(uint16_t x, uint16_t y, uint16_t color);
void LCD_WriteArea(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t *data);
void LCD_Line(uint16_t x1, uint16_t y1,  uint16_t x2, uint16_t y2, uint16_t color);
void LCD_HLine(uint16_t x, uint16_t y, uint16_t width, uint16_t color);
void LCD_VLine(uint16_t x, uint16_t y, uint16_t height, uint16_t color);
void LCD_Clear(uint16_t color);
void LCD_Bkl(uint8_t state);
void LCD_Scroll(uint16_t sc);
void LCD_SetOrientation(uint8_t m);
uint16_t LCD_GetWidth(void);
uint16_t LCD_GetHeight(void);
uint32_t LCD_GetSize(void);

#ifdef __cplusplus
};
#endif

#endif
