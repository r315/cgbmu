//Using SDL and standard IO 

#include <SDL.h>

#include <stdio.h> 
#include <string.h>
#include "board.h"

#define UPDATE_TIME 30 //30ms => 33fps

#define WINDOW_W	LCD_W
#define WINDOW_H	LCD_H

typedef struct _Wnd{
	SDL_Window *window;
	SDL_Surface *surface;
    char *title;
	uint32_t w;
	uint32_t h;
	uint32_t wx;			//wrap x
	uint32_t wy;			//wrap y
	uint32_t ww;			//wrap w
	uint32_t wh;			//wrap h
	uint32_t mx;			//memory current x
	uint32_t my;			//memory current y
    uint32_t auto_update;
}Lcd;

static Lcd lcd;

void LCD_Scroll(uint16_t y){}
void LCD_Bkl(uint8_t state){}


uint32_t LCD_Auto_Update(uint32_t interval, void *ptr){
Lcd *plcd = (Lcd*)ptr;    

	if(SDL_UpdateWindowSurface(plcd->window) < 0){
		printf("SDL_UdateWindowSurface failed: %s\n", SDL_GetError());
		return 0;
	}

    if(!plcd->auto_update)
      return 0;	

    return interval;
}

void LCD_Update(void){
    SDL_UpdateWindowSurface(lcd.window);
}

SDL_Window *Window_Init(Lcd *plcd){

    if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER ) < 0 ) { 
        fprintf(stdout, "LCDSDL: SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
        return NULL; 
    } 
      
    plcd->window = SDL_CreateWindow(  (plcd->title == NULL)? "":plcd->title, 
                                    SDL_WINDOWPOS_UNDEFINED,
                                    SDL_WINDOWPOS_UNDEFINED, 
                                    plcd->w, 
                                    plcd->h, 
                                    SDL_WINDOW_SHOWN); 
    if( plcd->window == NULL ) { 
        fprintf(stdout, "LCDSDL: Window could not be created! SDL_Error: %s\n", SDL_GetError() );
        return NULL;
    }
        
    plcd->surface = SDL_GetWindowSurface( plcd->window );
     
    fprintf(stdout,"LCDSDL: Window size %dx%d %dbpp\n",plcd->surface->w, plcd->surface->h, plcd->surface->format->BitsPerPixel);    
      
    SDL_FillRect(plcd->surface, NULL, SDL_MapRGB(plcd->surface->format, 0x00, 0x0, 0x0 ) );    
    
    plcd->auto_update = 1;
  
    SDL_AddTimer(UPDATE_TIME, LCD_Auto_Update, plcd);

	SDL_SetWindowPosition(plcd->window, 20, 50);
    return plcd->window;
}

void LCD_Init(void){
	lcd.w = WINDOW_W;
	lcd.h = WINDOW_H;
    lcd.title = "lcd emulator";
    Window_Init(&lcd);	
}

void LCD_Close(void){
    lcd.auto_update = 0;    
	SDL_DestroyWindow(lcd.window); 
	SDL_Quit(); 
}

void LCD_Rotation(uint8_t m){
    LCD_Close();
     switch (m) {
        case LCD_PORTRAIT:            
             lcd.w = WINDOW_W;
             lcd.h = WINDOW_H;            
            break;
            
        case LCD_LANDSCAPE:
            lcd.h = WINDOW_W;
            lcd.w = WINDOW_H; 
            break;
            
        case LCD_REVERSE_PORTRAIT:            
            break;
            
        case LCD_REVERSE_LANDSCAPE:            
            break;

        default:
            return;
    }
    Window_Init(&lcd);
}

void LCD_Fill(uint32_t n, uint16_t color){
	while(n--)
		LCD_Data(color);
}

void LCD_IndexedColor(uint16_t *colors, uint8_t *index, uint32_t size){
	while(size--){
        LCD_Data(colors[*index]);
		index += 1;
    }
}

void LCD_Window(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    lcd.mx = x;
    lcd.my = y;   

    lcd.wx = x;
    lcd.wy = y;
    lcd.ww = x + (w-1);
    lcd.wh = y + (h-1);    
}

void LCD_Pixel(uint16_t x, uint16_t y, uint16_t color){
    LCD_Window(x,y,1,1);
    LCD_Data(color);
}
//-------------------------------------------------------------
//        RRRRRGGGGGGBBBBB 
//        BBBBBGGGGGGRRRRR
//RRRRRRRRGGGGGGGGBBBBBBBB
//-------------------------------------------------------------
void LCD_Data(uint16_t color)
{
   uint32_t *pixels = (uint32_t *)lcd.surface->pixels;    
   
	if(lcd.mx >= lcd.wx && lcd.mx <= lcd.ww && 
       lcd.my >= lcd.wy && lcd.my <= lcd.wh &&
       lcd.mx < lcd.w && lcd.my < lcd.h)
	{
    	pixels[ (lcd.my * lcd.w) + lcd.mx] = (uint32_t) ( ((color&0xf800)<<8) | ((color&0x7e0)<<5) | ((color&0x1f)<<3) );   
        if(lcd.mx == lcd.ww){
            lcd.mx = lcd.wx;
            if(lcd.my == lcd.wh) lcd.my = lcd.wy;
            else lcd.my++;
        }
        else lcd.mx++;
    }   
}

void LCD_WriteArea(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t *data) {
	LCD_Window(x, y, w, h);
	for (int i = 0; i < w * h; i++)
	{
		LCD_Data(*data++);
	}
}

uint16_t LCD_GetWidth(void){
    return lcd.w;
}
uint16_t LCD_GetHeight(void){
    return lcd.h;
}

void LCD_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {

}