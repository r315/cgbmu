// 01-2011 adicionada a fun��o Atributo e removidas as rotinas drawChar e solidchar
// 14-03-2012 bug ao defenir MULTIPLE_FONTS � defenido FONT_TYPE_4 que define UPPER_CASE_FONT e
//            deixa de haver minusculas


#include "lcd.h"

#include "graphics.h"
volatile char g_attribute;
volatile const char *g_char;
volatile unsigned int _FC,_BC;
#ifdef MULTIPLE_FONTS
const char *FONT;
char FONT_W,FONT_H;
//----------------------------------------------------------
//
//----------------------------------------------------------
void setFont(char fnt)
{
	switch(fnt)
	{
		case 0:
			FONT = FONTNORMAL;
			FONT_W = 8;
			FONT_H =8;
			break;
		case 1:
			FONT = FONTBOLD;
			FONT_W = 8;
			FONT_H =7;
			break;
		case 2:
			FONT = FONTLCD;
			FONT_W = 8;
			FONT_H =7;
			break;

		case 3:
			FONT = FONTPIXELDUST;
			FONT_W = 7;
			FONT_H = 7;
			break;
	}			
}
#endif
//----------------------------------------------------------
//
//----------------------------------------------------------
void drawDoubleChar(int x, int y)
{
unsigned char w,h;

    LCD_Window(x,y,FONT_W*2,FONT_H*2);

	for (h=0;h<FONT_H;h++)			// altura
	{ 	
		for(w=0;w<FONT_W;w++)			// primeira linha
		{
			if(*g_char & (0x80 >> w))	// se pixel
			{
				LCD_Data(_FC);			// desenha 2 px w de FC
				LCD_Data(_FC);
			}
	        else
			{
    	        LCD_Data(_BC);			// desenha 2 px w de BC
				LCD_Data(_BC);
			}
		}
		for(w=0;w<FONT_W;w++)			// segunda linha igual a primeira
		{
			if(*g_char & (0x80 >> w))
			{
				LCD_Data(_FC);
				LCD_Data(_FC);
			}
	        else
			{
    	        LCD_Data(_BC);					
				LCD_Data(_BC);
			}
		}
		*g_char++;
 	}	
}
//----------------------------------------------------------
//
//----------------------------------------------------------
void drawTransparentChar(int x, int y)
{
char w,h;
	for (h=0;h<FONT_H;h++)
 	{
		for(w=0;w<FONT_W;w++){
			if(*g_char & (0x80 >> w))
				drawPixel(x+w,y+h);							
	}
	*g_char++;
 }
}
//----------------------------------------------------------
//
//----------------------------------------------------------
void _drawChar(char n)
{
unsigned char w,h;
	for (h=0;h<FONT_H;h++)
	{ 	
		for(w=0;w<FONT_W;w++)
		{
			if(*g_char & (0x80 >> w))
				LCD_Data(n?_BC:_FC);			
			else
				LCD_Data(n?_FC:_BC);
		}
		*g_char++;
 	}	
}
//----------------------------------------------------------
//
//----------------------------------------------------------
int drawChar(int x, int y, unsigned char c)
{
#ifdef UPPER_CASE_FONT
    if(c>0x60)
        c=toupper(c);	
#endif

#ifdef MULTIPLE_FONTS
    if(!FONT)
    	setFont(NORMAL);
#endif

    c -= 0x20;      	
    g_char = FONT + ( c * FONT_H); 
    LCD_Window(x,y,FONT_W,FONT_H);
    
    switch(g_attribute)
    {
		case g_normal:	    _drawChar(0); break;
		case g_inverted:    _drawChar(1); break;
		case g_transparent: drawTransparentChar(x,y); break;
		case g_double:	    drawDoubleChar(x,y);return x+(FONT_W*2);
    }
    return x+FONT_W;
}
//----------------------------------------------------------
//
//----------------------------------------------------------
int drawString(int x, int y,const char *s)
{   
    while(*s)
        x = drawChar(x,y,*s++);
    return x;
}
//----------------------------------------------------------
// 
//----------------------------------------------------------
int drawNumber(int x, int y, signed long v, signed char radix)
{
unsigned char i=0,c,dig[8],sgn=0;

	if(radix < 0)
	{
		radix = -radix;
		if(v<0)
		{	
			v = -v;
			sgn = '-';
		}
	}
	 
	
	do{
		c = (unsigned char)(v % radix);
		if (c >= 10)c += 7;		
		c += '0';
		v /= radix;
		dig[i++]=c;
	} while(v);
	
	if(sgn) dig[i++]= sgn;

	while(i--)
	x = drawChar(x,y,dig[i]);		
	return x;
}
//----------------------------------------------------------
// setwrap(sx,sy,w,h) tem de ser chamado anteriormente
// a ESTE drawPixel(x,y) pixel com cor Foreground color
//----------------------------------------------------------
void drawPixel(int x0, int y0)
{
    LCD_Pixel(x0,y0,_FC);
}
//----------------------------------------------------------
// linha de cor Foreground color
//----------------------------------------------------------
#if 0
#include <math.h>
void drawLine(int x1, int y1, int x2, int y2)
{
int i, deltax, deltay, numpixels;
int d, dinc1, dinc2;
int x, xinc1, xinc2;
int y, yinc1, yinc2;	
	
	deltax = abs(x2 - x1);
    deltay = abs(y2 - y1);

	setwrap(0,0,MAX_W,MAX_H);	

    if (deltax >= deltay)
    {
		numpixels = deltax + 1;
       	d = (2 * deltay) - deltax;
       	dinc1 = deltay << 1;
       	dinc2 = (deltay - deltax) << 1;
       	xinc1 = 1;
       	xinc2 = 1;
       	yinc1 = 0;
       	yinc2 = 1;
    }
    else
    {
       	numpixels = deltay + 1;
       	d = (2 * deltax) - deltay; 
       	dinc1 = deltax << 1;
       	dinc2 = (deltax - deltay) << 1;		
		xinc1 = 0;
		yinc2 = 1;
       	xinc2 = 1;
       	yinc1 = 1;
    }

    if(x1 > x2)
    {
       	xinc1 = -xinc1;
       	xinc2 = -xinc2;
    }
    if(y1 > y2)
    {
     	yinc1 = -yinc1;
       	yinc2 = -yinc2;
    }
    x = x1;
    y = y1;
    for(i = 1; i < numpixels; i++)
    {
    	drawPixel(x, y);
	   	if(d < 0)
    	{
       		d = d + dinc1;
	       	x = x + xinc1;
          	y = y + yinc1;
    	}
       	else
	   	{
          	d = d + dinc2;
        	x = x + xinc2;
	       	y = y + yinc2;
       	}
    }	
}
//--------------------------------------------------------*/
//drawLine(0,0,16,0); 
//desenha uma linha horizontal do pixel 0 ao pixel 16
//----------------------------------------------------------
#else
void drawLine(int x0, int y0, int x1, int y1) 
{
    signed int dy = y1 - y0;
    signed int dx = x1 - x0;
    signed int stepx, stepy;
    signed int fraction;

    if (dy < 0) { dy = -dy;  stepy = -1; } else { stepy = 1; }
    if (dx < 0) { dx = -dx;  stepx = -1; } else { stepx = 1; }
    dy <<= 1;
    dx <<= 1;	

	LCD_Window(0,0,LCD_W,LCD_H);
    drawPixel(x0, y0);
    if (dx > dy) 
    {
        fraction = dy - (dx >> 1);
        while (x0 != x1) 
        {
            if (fraction >= 0) 
            {
                y0 += stepy;
                fraction -= dx;
            }
            x0 += stepx;
            fraction += dy;	
            drawPixel(x0, y0);
        }
    } 
    else 
    {
        fraction = dx - (dy >> 1);
        while (y0 != y1) 
        {
            if (fraction >= 0) 
            {
                x0 += stepx;
                fraction -= dy;
            }
            y0 += stepy;
            fraction += dx;
            drawPixel(x0, y0);
            
        }
    }
}
#endif
//-----------------------------------------------------------*/
// drawBox usa Backcolor como enchimento
//-------------------------------------------------------------
void drawBox(int sx, int sy, int width, int height)
{
	LCD_Window(sx,sy,width,height);           
	LCD_Fill(width*height,_BC);
}
//-------------------------------------------------------------
//
//-------------------------------------------------------------
void drawRectangle(int x1, int y1, int width, int height)
{
	width  -= 1;
	height -= 1;
	drawLine(x1,y1,x1+width,y1);              // top horizontal
	drawLine(x1+width,y1,x1+width,y1+height); // right vertical
	drawLine(x1,y1+height,x1+width,y1+height);// bottom horizontal
	drawLine(x1,y1,x1,y1+height);             // left vertical
}
//-------------------------------------------------------------
//
//-------------------------------------------------------------
void drawCircule(int xCenter,int yCenter,char radius)
{
	unsigned int x = 0;
	unsigned char y = radius;
	short p = 3 - 2 * radius;

	LCD_Window(0,0,LCD_W,LCD_H);

	while (x <= y)
	{
		drawPixel(xCenter + x, yCenter + y);
		drawPixel(xCenter - x, yCenter + y);
		drawPixel(xCenter + x, yCenter - y);
		drawPixel(xCenter - x, yCenter - y);
		drawPixel(xCenter + y, yCenter + x);
		drawPixel(xCenter - y, yCenter + x);
		drawPixel(xCenter + y, yCenter - x);
		drawPixel(xCenter - y, yCenter - x);
		if (p < 0)
			p += 4 * x++ + 6;
		else
			p += 4 * (x++ - y--) + 10;
	}
}


