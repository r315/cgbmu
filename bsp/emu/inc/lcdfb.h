#ifndef _lcdfb_h_
#define _lcdfb_h_

#include <stdint.h>

#define LCD_W LCD_GetWidth()
#define LCD_H LCD_GetHeight()
#define LCD_SIZE (LCD_W * LCD_H)

#define LCD_PORTRAIT  0
#define LCD_LANDSCAPE 1
#define LCD_REVERSE_PORTRAIT  2
#define LCD_REVERSE_LANDSCAPE 3

#define FRAME_BUFFER "/dev/fb1"

void LCD_Close(void);
void LCD_Data(uint16_t color);
void LCD_Fill(uint32_t n, uint16_t color);
uint16_t LCD_GetWidth(void);
uint16_t LCD_GetHeight(void);
#endif
