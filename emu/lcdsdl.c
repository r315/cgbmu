
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
void lcdInit(void)
{
    screen = NULL;
    SDL_Init( SDL_INIT_EVERYTHING );    
    screen = SDL_SetVideoMode(LCD_W,LCD_H,32, SDL_SWSURFACE );
	//SDL_WM_SetCaption("Blueboard Emulator",NULL);    
}
//-------------------------------------------------------------
//
//-------------------------------------------------------------
void solidfill(int count, int c)
{
   while(count--)   
      lcddata(c);
}
//-------------------------------------------------------------
//
//-------------------------------------------------------------
void setgram(int x, int y)
{
    _grx = x+_offset;
    _gry = y+_offset;    
}
//-------------------------------------------------------------
//
//-------------------------------------------------------------
void setwrap(int x, int y, int width, int height)
{
    _sx = x+_offset;
    _ex = x + (width-1)+_offset;
    _sy = y+_offset;
    _ey = y + (height-1)+_offset;
}
//-------------------------------------------------------------
//        RRRRRGGGGGGBBBBB 
//        BBBBBGGGGGGRRRRR
//RRRRRRRRGGGGGGGGBBBBBBBB
//-------------------------------------------------------------
void lcddata(unsigned int color)
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
int lcdreaddata(void)
{
   Uint32 tmp, *pixels = (Uint32 *)screen->pixels;   
   tmp = pixels[ (_gry*LCD_W)+_grx ];
   
   return (tmp&0xF80000)>>19|(tmp&0xFC00)>>5|(tmp&0xF8)<<8;
}
//-------------------------------------------------------------------
//	 
//	 
//-------------------------------------------------------------------
void putpixel(int x, int y, int c)
{
	setgram(x,y);
	lcddata(c);
}
void hwscroll(int y){    
}

