/**
* @file     st7735.h
* @brief    Contains the st7735 lcd controller API headers and symbols.
*           
* @version  1.0
* @date     8 Jun. 2018
* @author   Hugo Reis
**********************************************************************/

#ifndef _st7735_h_
#define _st7735_h_

#include <stdint.h>

#define TFT_ST7735S

// System Function Commands
#define ST7735_NOP     0x00
#define ST7735_SWRESET 0x01
#define ST7735_RDDID   0x04
#define ST7735_RDDST   0x09

#define ST7735_SLPIN   0x10
#define ST7735_SLPOUT  0x11
#define ST7735_PTLON   0x12
#define ST7735_NORON   0x13

#define ST7735_INVOFF  0x20
#define ST7735_INVON   0x21
#define ST7735_DISPOFF 0x28
#define ST7735_DISPON  0x29
#define ST7735_CASET   0x2A
#define ST7735_RASET   0x2B
#define ST7735_RAMWR   0x2C
#define ST7735_RAMRD   0x2E

#define ST7735_PTLAR   0x30
#define ST7735_COLMOD  0x3A
#define ST7735_MADCTL  0x36
#define ST7735_VSCSAD  0x37

#define ST7735_RDID1   0xDA
#define ST7735_RDID2   0xDB
#define ST7735_RDID3   0xDC
#define ST7735_RDID4   0xDD

// Panel Function Commands
#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR  0xB4
#define ST7735_DISSET5 0xB6

#define ST7735_PWCTR1  0xC0
#define ST7735_PWCTR2  0xC1
#define ST7735_PWCTR3  0xC2
#define ST7735_PWCTR4  0xC3
#define ST7735_PWCTR5  0xC4
#define ST7735_VMCTR1  0xC5
#define ST7735_VMOFCTR 0xC7

#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1

#define ST7735_PWCTR6  0xFC

// -----------------------------

#define MADCTL_MY  (1<<7)
#define MADCTL_MX  (1<<6)
#define MADCTL_MV  (1<<5)
#define MADCTL_ML  (1<<4)
#define MADCTL_RGB (1<<3)
#define MADCTL_MH  (1<<2)

// Display Memory settings
// For JD-T1800 GM is 0
#define ST7753_GM       0 // 0: 132 x 18-bits x 162, 3: 128 x 18-bits x 160

enum {
    LCD_PORTRAIT = 0,
    LCD_LANDSCAPE,
    LCD_REVERSE_PORTRAIT,
    LCD_REVERSE_LANDSCAPE
};

void LCD_Init(void *ptr);
void LCD_Write(uint16_t *data, uint32_t count);
void LCD_Scroll(uint16_t sc);
void LCD_Data(uint16_t data);
void LCD_Pixel(uint16_t x, uint16_t y, uint16_t color);
void LCD_Window(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void LCD_SetOrientation(uint8_t m);
void LCD_Bkl(uint8_t state);
uint16_t LCD_GetWidth(void);
uint16_t LCD_GetHeight(void);
uint32_t LCD_GetSize(void);

#endif

