/**
* @file     st7789.h
* @brief    Contains the st7789 lcd controller API headers and symbols.
*           
* @version  1.0
* @date     1 April 2023
* @author   Hugo Reis
* */

#ifndef _st7789_h_
#define _st7789_h_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TFT_ST7789

#define TFT_W 240
#define TFT_H 240

// commands
#define ST7789_NOP       0x00
#define ST7789_SWRESET   0x01
#define ST7789_RDDID     0x04
#define ST7789_RDDST     0x09

#define ST7789_SLPIN     0x10
#define ST7789_SLPOUT    0x11
#define ST7789_PTLON     0x12
#define ST7789_NORON     0x13

#define ST7789_INVOFF    0x20
#define ST7789_INVON     0x21
#define ST7789_GAMSET    0x26
#define ST7789_DISPOFF   0x28
#define ST7789_DISPON    0x29
#define ST7789_CASET     0x2A
#define ST7789_RASET     0x2B
#define ST7789_RAMWR     0x2C
#define ST7789_RAMRD     0x2E

#define ST7789_PTLAR     0x30
#define ST7789_VSCRDEF   0x33
#define ST7789_TEOFF     0x34
#define ST7789_TEON      0x35
#define ST7789_MADCTL    0x36
#define ST7789_VSCSAD    0x37
#define ST7789_IDMOFF    0x38
#define ST7789_IDMON     0x39
#define ST7789_COLMOD    0x3A
#define ST7789_RAMWRC    0x3C
#define ST7789_RAMRDC    0x3E

#define ST7789_TESCAN    0x44
#define ST7789_RDTESCAN  0x45

#define ST7789_WRDISBV   0x51
#define ST7789_RDDISBV   0x52
#define ST7789_WRCTRLD   0x53
#define ST7789_RDCTRLD   0x54
#define ST7789_WRCACE    0x55
#define ST7789_RDCABC    0x56
#define ST7789_WRCABCMB  0x5E
#define ST7789_RDCABCMB  0x5F

#define ST7789_RDABCSDR  0x68

#define ST7789_RDID1     0xDA
#define ST7789_RDID2     0xDB
#define ST7789_RDID3     0xDC

/* System function control */
#define ST7789_RAMCTRL   0xB0
#define ST7789_RGBCTRL   0xB1
#define ST7789_PORCTRL   0xB2
#define ST7789_FRMCTR1   0xB3
#define ST7789_PARCTRL   0xB5
#define ST7789_GCTRL     0xB7
#define ST7789_GTADJ     0xB8
#define ST7789_DGMEN     0xBA
#define ST7789_VCOMS     0xBB
#define ST7789_LCMCTRL   0xC0
#define ST7789_IDSET     0xC1
#define ST7789_VDVVRHEN  0xC2
#define ST7789_VRHS      0xC3
#define ST7789_VDSET     0xC4



/* Reg bits */
#define ST7789_MADCTL_MY  0x80  // Page Address Order
#define ST7789_MADCTL_MX  0x40  // Column Address Order
#define ST7789_MADCTL_MV  0x20  // Page/Column Order
#define ST7789_MADCTL_ML  0x10  // Line Address Order
#define ST7789_MADCTL_MH  0x04  // Display Data Latch Order
#define ST7789_MADCTL_RGB 0x00
#define ST7789_MADCTL_BGR 0x08

#define ST7789_WRCTRLD_BCTRL   0x20
#define ST7789_WRCTRLD_DD      0x08
#define ST7789_WRCTRLD_BL      0x04

#define OPTIONS_WRAP_V    0x01
#define OPTIONS_WRAP      0x03
#define OPTIONS_WRAP_H    0x02

// color modes
#define COLOR_MODE_65K    0x50
#define COLOR_MODE_262K   0x60
#define COLOR_MODE_12BIT  0x03
#define COLOR_MODE_16BIT  0x05
#define COLOR_MODE_18BIT  0x06
#define COLOR_MODE_16M    0x07

enum {
    LCD_PORTRAIT = 0,
    LCD_LANDSCAPE,
    LCD_REVERSE_PORTRAIT,
    LCD_REVERSE_LANDSCAPE
};

void LCD_Init(void *ptr);
void LCD_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void LCD_WriteArea(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t *data);
void LCD_Pixel(uint16_t x, uint16_t y, uint16_t color);
void LCD_Scroll(uint16_t sc);
void LCD_SetOrientation(uint8_t m);
void LCD_Window(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void LCD_Data(uint16_t data);
void LCD_Bkl(uint8_t state);
uint16_t LCD_GetWidth(void);
uint16_t LCD_GetHeight(void);
uint32_t LCD_GetSize(void);

#ifdef  __cplusplus
}
#endif /*  __cplusplus */

#endif

