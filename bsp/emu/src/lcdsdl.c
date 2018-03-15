
#include <common.h>
#include "lcdsdl.h"

SDL_Surface* screen ;
Uint32 _pixel_pointer;

uint16_t _grx,_gry;
uint16_t _sx,_ex;
uint16_t _sy,_ey;
uint16_t _offset = 0; // offset de janela para sso

//-------------------------------------------------------------
//
//-------------------------------------------------------------
void LCD_Init(void)
{
    screen = NULL;
    SDL_Init( SDL_INIT_EVERYTHING );    
    screen = SDL_SetVideoMode(LCD_W,LCD_H,32, SDL_SWSURFACE );
	//SDL_WM_SetCaption("Blueboard Emulator",NULL);    
}
void LCD_Close(void){
	SDL_Quit(); 
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
   Uint32 *pixels = (Uint32 *)screen->pixels;    
   
   if(_grx >= _sx && _grx <= _ex && _gry >= _sy && _gry<= _ey)
   {
   #ifdef BGR_MODE
        pixels[ (_gry*LCD_W)+_grx ] =(Uint32) ( ((color&0xf800)>>8) | ((color&0x7e0)<<5) | ((color&0x1f)<<19) );   
   #else
        pixels[ (_gry*LCD_W)+_grx ] =(Uint32) ( ((color&0xf800)<<8) | ((color&0x7e0)<<5) | ((color&0x1f)<<3) );   
   #endif
        if(_grx==_ex)
        {
            _grx=_sx;
            if(_gry==_ey)
                _gry=_sy;
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
   Uint32 tmp, *pixels = (Uint32 *)screen->pixels;   
   tmp = pixels[ (_gry*LCD_W)+_grx ];
   
   return (tmp&0xF80000)>>19|(tmp&0xFC00)>>5|(tmp&0xF8)<<8;
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
    return LCD_W;
}
uint16_t LCD_GetHeight(void){
    return LCD_H;
}

void LCD_Scroll(uint16_t y){
}
void LCD_Bkl(uint8_t state){
}

void LCD_Push(void){
}
void LCD_Pop(void){
}


