
#include <common.h>
#include <stdint.h>
#include <stdio.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "lcdfb.h"

typedef struct{
	int fd;
	struct fb_var_screeninfo orig_vinfo;
	struct fb_var_screeninfo vinfo;
	struct fb_fix_screeninfo finfo;
	uint32_t screensize;
	short *pixels;
}Lcd_Type;

Lcd_Type lcdfb;

uint16_t _grx,_gry;
uint16_t _sx,_ex;
uint16_t _sy,_ey;
uint16_t _offset = 0; // offset de janela para sso

//-------------------------------------------------------------
//
//-------------------------------------------------------------
void LCD_Init(void)
{	
	lcdfb.fd = open(FRAME_BUFFER, O_RDWR);
	if (!lcdfb.fd) {
	    printf("Error: cannot open framebuffer device.\n");
    	return;
  	}

	// Get variable screen information
	if (ioctl(lcdfb.fd, FBIOGET_VSCREENINFO, &lcdfb.vinfo)) {
    	printf("Error reading variable information.\n");
    	goto _exit;
  	}
  	
  	printf("Frame Buffer %dx%d, %dbpp\n", lcdfb.vinfo.xres, lcdfb.vinfo.yres, lcdfb.vinfo.bits_per_pixel );

	// Get fixed screen information
	if (ioctl(lcdfb.fd, FBIOGET_FSCREENINFO, &lcdfb.finfo)) {
    	printf("Error reading fixed information.\n");
    	goto _exit;
  	}
  	
  	lcdfb.screensize = lcdfb.finfo.smem_len;
  	lcdfb.pixels = (short*)mmap(0, lcdfb.screensize,
  								PROT_READ | PROT_WRITE,
			                    MAP_SHARED,
			                    lcdfb.fd, 0);
  	if ((int)lcdfb.pixels == -1) {
    	printf("Failed to mmap.\n");
    	goto _exit;
  	}
  	
  	return;
  	
_exit:
	close(lcdfb.fd);

}

void LCD_Close(void){
	close(lcdfb.fd);
}

//-------------------------------------------------------------
//
//-------------------------------------------------------------
void LCD_Fill( uint32_t n, uint16_t color)
{
   while(n--)   
      LCD_Data(color);
}
//-------------------------------------------------------------
//
//-------------------------------------------------------------
void LCD_Window(int x, int y, int width, int height)
{
    _sx = x + _offset;
    _ex = x + (width - 1) + _offset;
    _sy = y + _offset;
    _ey = y + (height - 1) + _offset;

    _grx = x + _offset;
    _gry = y + _offset;
}
//-------------------------------------------------------------
//        RRRRRGGGGGGBBBBB 
//        BBBBBGGGGGGRRRRR
//RRRRRRRRGGGGGGGGBBBBBBBB
//-------------------------------------------------------------
void LCD_Data(uint16_t color)
{
   
   if(_grx >= _sx && _grx <= _ex && _gry >= _sy && _gry<= _ey)
   {
   
        lcdfb.pixels[(_gry * LCD_GetWidth()) + _grx ] = (short) color;
        //( ((color&0xf800)<<8) | ((color&0x7e0)<<5) | ((color&0x1f)<<3) );   

        if(_grx == _ex)
        {
            _grx = _sx;
            if(_gry == _ey)
                _gry = _sy;
            else
                _gry++;
        }
        else
            _grx++;
    }   
}
//-------------------------------------------------------------
//
//-------------------------------------------------------------
int LCD_Data_Read(void)
{
   return -1;
}
//-------------------------------------------------------------------
//	 
//	 
//-------------------------------------------------------------------
void LCD_Pixel(uint16_t x, uint16_t y, uint16_t color)
{
	LCD_Window(x,y,1,1);
	LCD_Data(color);
}
uint16_t LCD_GetWidth(void){
    return lcdfb.vinfo.xres;
}
uint16_t LCD_GetHeight(void){
    return lcdfb.vinfo.yres;
}

void LCD_Scroll(uint16_t y){
}
void LCD_Bkl(uint8_t state){
}

void LCD_Push(void){
}
void LCD_Pop(void){
}

