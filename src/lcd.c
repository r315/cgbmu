//#include <inttypes.h>
#include <stdint.h> // MCUXpresso
#include <lcd.h>


void LCD_Clear(uint16_t color){
	LCD_Window(0,0, LCD_GetWidth(), LCD_GetHeight());
	LCD_Fill(LCD_SIZE, color);
}

void LCD_FillRect(uint16_t x, uint16_t y,  uint16_t w, uint16_t h, uint16_t color){
    LCD_Window(x,y,w,h);    
    LCD_Fill(w * h, color);
}

void LCD_FillRoundRect(uint16_t x, uint16_t y,  uint16_t w, uint16_t h, uint16_t color){

    LCD_FillRect(x + 1, y, w-2, h, color);

    h -= 2;
    LCD_Window(x, y+1, 1, h);
    LCD_Fill(h, color);

    LCD_Window(x+w-1, y+1, 1, h);
    LCD_Fill(h, color);
}

void LCD_Rect(uint16_t x, uint16_t y,  uint16_t w, uint16_t h, uint16_t color){
    LCD_Window(x, y, w, 1);
    LCD_Fill(w, color);
	
	LCD_Window(x + w , y, 1, h+1);
    LCD_Fill(h+1, color);
	
	LCD_Window(x, y + h , w, 1);
    LCD_Fill(w, color);
	
	LCD_Window(x ,y ,1, h);
    LCD_Fill(h, color);
}

/**
 * 
 **/ 
void LCD_Line(uint16_t x1, uint16_t y1,  uint16_t x2, uint16_t y2, uint16_t color){
{
    signed int dy = y2 - y1;
    signed int dx = x2 - x1;
    signed int stepx, stepy;
    signed int fraction;

    if (dy < 0) { dy = -dy;  stepy = -1; } else { stepy = 1; }
    if (dx < 0) { dx = -dx;  stepx = -1; } else { stepx = 1; }
    dy <<= 1;
    dx <<= 1;
    
    LCD_Pixel(x1, y1,color);

    if (dx > dy) 
    {
        fraction = dy - (dx >> 1);
        while (x1 != x2) 
        {
            if (fraction >= 0) 
            {
                y1 += stepy;
                fraction -= dx;
            }
            x1 += stepx;
            fraction += dy;	
            LCD_Pixel(x1, y1,color);
        }
    } 
    else 
    {
        fraction = dx - (dy >> 1);
        while (y1 != y2) 
        {
            if (fraction >= 0) 
            {
                x1 += stepx;
                fraction -= dy;
            }
            y1 += stepy;
            fraction += dx;
            LCD_Pixel(x1, y1,color);
            
        }
    }
}    
}



