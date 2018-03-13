#include <stdarg.h>
#include "display.h"

int drawCharSimple(int x, int y, unsigned char *d_char);

static Display _display = {
	.font = (uint8_t*)(FONTDEFAULT+2),
	.font_w = FONTDEFAULT_W,
	.font_h = FONTDEFAULT_H,
	.forecolor = WHITE,
	.backcolor = BLACK,
	.cx = 0,
	.cy = 0,
	.sc = 0,
	.xputc = DISPLAY_putc,
	.drawChar = drawCharSimple,
};


void DISPLAY_SetColors(uint16_t fc, uint16_t bc){ _display.forecolor = fc; _display.backcolor = bc;}
void DISPLAY_SetFcolor(uint16_t color){ _display.forecolor = color;}
void DISPLAY_SetBcolor(uint16_t color){ _display.backcolor = color;}
int DISPLAY_GetFontWidth(void){ return _display.font_w; }
int DISPLAY_GetFontHeight(void){ return _display.font_h; }



void DISPLAY_GotoAbsolute(uint16_t x, uint16_t y){
   _display.cx = x;
	_display.cy = y;
}
//-----------------------------------------------------------
//
//-----------------------------------------------------------
void DISPLAY_Goto(uint16_t x, uint16_t y){
	_display.cx = x * _display.font_w;
	_display.cy = y * _display.font_h;
}
//----------------------------------------------------------
//stdout calls this function
//----------------------------------------------------------
void DISPLAY_putc(char c)
{
	if (c == '\n' || _display.cx == LCD_GetWidth()){
		_display.cx = 0;
		_display.cy += _display.font_h;
		if(_display.cy == LCD_GetHeight()){
			_display.cy = 0;
			if(!_display.sc)
				_display.sc = _display.font_h;
		}
		if(_display.sc){			
			LCD_Scroll(_display.sc);			
			_display.sc += _display.font_h;

			_display.cx = LCD_GetWidth();
			while(_display.cx){			// limpa a proxima linha
				DISPLAY_Char(_display.cx,_display.cy,' ');
				_display.cx -= _display.font_w;
			}
		}
		if(c== '\n')
			return;	
	}
	if(c == '\r'){
		_display.cx = 0;
		return;
	}	
	_display.cx = DISPLAY_Char(_display.cx, _display.cy, c);
}
//-----------------------------------------------------------
//
//-----------------------------------------------------------
void DISPLAY_puts(const char *s){   
    while(*s)
        DISPLAY_putc(*s++);	
}
//-----------------------------------------------------------
//
//-----------------------------------------------------------
void xitoa (long val, signed char radix, signed char len){
	unsigned char c, r, sgn = 0, pad = ' ';
	unsigned char s[16], i = 0;
	unsigned int v;

	if (radix < 0) {
		radix = -radix;
		if (val < 0) {
			val = -val;
			sgn = '-';
		}
	}
	
	v = val;
	r = radix;
	
	if (len < 0) {
		len = -len;
		pad = '0';
	}
	
	if (len > 16) len = 16;
	
	do {
		c = (unsigned char)(v % r);
		if (c >= 10) c += 7;
		c += '0';
		s[i++] = c;
		v /= r;
	} while (v);
	
	if(sgn) 
		s[i++] = sgn;
	
	while (i < len)
		s[i++] = pad;
		
	do{
		DISPLAY_putc(s[--i]);
	}while (i);
}
//-----------------------------------------------------------
//https://en.wikipedia.org/wiki/Single-precision_floating-point_format
//https://wirejungle.wordpress.com/2011/08/06/displaying-floating-point-numbers
//-----------------------------------------------------------
void xftoa(double f, char places){
long int_part, frac_part;
char prec;
  
    int_part = (long)(f);  
    
	if(places > FLOAT_MAX_PRECISION)
		places = FLOAT_MAX_PRECISION;
		
	frac_part = 0;
	prec = 0;
	while ((prec++) < places){
			f *= 10;
			frac_part = (frac_part * 10) + ((long)f%10);			
	}

    xitoa(int_part, -10, 0);
    DISPLAY_putc('.');   
    xitoa(abs(frac_part), 10, -places);  
}
//-----------------------------------------------------------
//
//-----------------------------------------------------------
void DISPLAY_printf(const char* str, ...){
	va_list arp;
	char d, w, s, l, f;
	signed char r;

	va_start(arp, str);

	while ((d = *str++) != 0) {
		if (d != '%') {
			DISPLAY_putc(d); 
			continue;
		}
		
		d = *str++; 
		w = r = s = l = f = 0;
		
		if(d == '.'){
			d = *str++; f = 1;
		}		
		
		if (d == '0') {
			d = *str++; s = 1;
		}
		
		while ((d >= '0')&&(d <= '9')) {
			w += w * 10 + (d - '0');
			d = *str++;
		}		
		
		if (d == 'l') {
			l = 1;
			d = *str++;
		}
		
		if (!d) break;
		
		if (d == 's') {
			DISPLAY_puts(va_arg(arp, char*));
			continue;
		}
		
		if (d == 'c') {
			DISPLAY_putc(va_arg(arp, int));
			continue;
		}
		
		if (d == 'u') r = 10;
		if (d == 'd') r = -10;
		if (d == 'X') r = 16;
		if (d == 'b') r = 2;		
		
		if(d == 'f'){
			if(!f)
				w = 6;						// dafault 6 decimal places
			xftoa(va_arg(arp, double), w);			
			continue;
		}
		
		if (!r) break;
		
		if (s) w = -w;
		
		if (l) {
			xitoa(va_arg(arp, long), r, w);
		} else {
			if (r > 0)
				xitoa((long)va_arg(arp, unsigned int), r, w);
			else
				xitoa((long)va_arg(arp, int), r, w);
		}
	}

	va_end(arp);
}
//----------------------------------------------------------
//draws a character without any attributes
//TODO: optimize
//----------------------------------------------------------
int drawCharSimple(int x, int y, unsigned char *d_char){
unsigned char w,h;
	for (h=0; h < _display.font_h; h++){ 	
		for(w=0; w < _display.font_h; w++){
			if(*d_char & (0x80 >> w))
				LCD_Data(_display.forecolor);			
			else
				LCD_Data(_display.backcolor);			
		}
		d_char += 1;
 	}
	return x+(_display.font_w);
}

int drawCharInverted(int x, int y, unsigned char *d_char){
unsigned char w,h;
	for (h=0; h < _display.font_h; h++){ 	
		for(w=0; w < _display.font_h; w++){
			if(*d_char & (0x80 >> w))
				LCD_Data(_display.backcolor);
			else
				LCD_Data(_display.forecolor);
		}
		d_char += 1;		
 	}
	return x+(_display.font_w);
}
int drawCharTransparent(int x, int y, unsigned char *d_char)
{
char w,h;
	for (h=0; h < _display.font_h; h++){
		for(w=0;w<_display.font_w; w++){
			if(*d_char & (0x80 >> w))
				DISPLAY_Pixel(x+w,y+h);							
		}	
		d_char += 1;
	}
	return x+(_display.font_w);
}
int drawCharDouble(int x, int y, unsigned char *d_char)
{
unsigned char w,h;
    LCD_Window(x,y,_display.font_w * 2, _display.font_h * 2);
	for (h=0;h<_display.font_h;h++){			    // altura
		for(w=0;w<_display.font_w;w++){			// primeira linha
			if(*d_char & (0x80 >> w))	// se pixel
			{
				LCD_Data(_display.forecolor);			// desenha 2 px w de FC
				LCD_Data(_display.forecolor);
			}
	        else
			{
    	        LCD_Data(_display.backcolor);			// desenha 2 px w de BC
				LCD_Data(_display.backcolor);
			}
		}
		for(w=0;w<_display.font_w;w++)			// segunda linha igual a primeira
		{
			if(*d_char & (0x80 >> w))
			{
				LCD_Data(_display.forecolor);
				LCD_Data(_display.forecolor);
			}
	        else
			{
    	        LCD_Data(_display.backcolor);					
				LCD_Data(_display.backcolor);
			}
		}
		d_char += 1;
 	}	
	return x+(_display.font_w * 2);
}
//----------------------------------------------------------
//
//----------------------------------------------------------
int DISPLAY_Char(int x, int y, unsigned char c)
{
#ifdef UPPER_CASE_FONT
    if(c >= 'a' )
        c=toupper(c);	
#endif

   // if(!_display.font)
   // 	DISPLAY_SetFont(FONT_INIT);

    c -= 0x20;    
    LCD_Window(x,y,_display.font_w,_display.font_h);	
	return _display.drawChar(x, y, (uint8_t*)_display.font+(c*_display.font_h));
}
//----------------------------------------------------------
//
//----------------------------------------------------------
int DISPLAY_Text(int x, int y, const char *s)
{   
    while(*s){
        x = DISPLAY_Char(x,y,*s++);
    }
	return x;
}
//----------------------------------------------------------
//
//----------------------------------------------------------
void *DISPLAY_GetReference(void){
	DISPLAY_Init(OFF);
	return &_display;
}

void DISPLAY_Init(uint8_t initlcd){
	_display.font = (uint8_t*)(FONTDEFAULT+2);
	_display.font_w = FONTDEFAULT[0];
	_display.font_h = FONTDEFAULT[1];
	_display.forecolor = WHITE;
	_display.backcolor = BLACK;
	_display.cx = 0;
	_display.cy = 0;
	_display.sc = 0;
	_display.xputc = DISPLAY_putc;
	_display.drawChar = drawCharSimple;
	if(initlcd){
		LCD_Init();
		LCD_Clear(_display.backcolor);
		LCD_Bkl(ON);
	}
}
//----------------------------------------------------------
//TODO: fix ignore atribute when setAttribute is called before
//any drawChar
//----------------------------------------------------------
void DISPLAY_SetFont(uint8_t fnt)
{
#ifndef MULTIPLE_FONTS
	if(_display.drawChar != drawCharSimple)
		DISPLAY_Init(OFF);
#else
	switch(fnt)
	{
		default:
			_display.forecolor = WHITE;
			_display.backcolor = BLACK;
			if(!_display.drawChar)
				_display.drawChar = drawCharSimple;	
		case FONT_DEFAULT:
			_display.font = (uint8_t*)(FONTDEFAULT);			
			break;
		case FONT_BOLD:
			_display.font = (uint8_t*)(FONTBOLD);			
			break;
		case FONT_LCD:
			_display.font = (uint8_t*)(FONTLCD);			
			break;
		case FONT_PIXELDUST:
			_display.font = (uint8_t*)(FONTPIXELDUST);			
			break;
	}	
	_display.font_w = _display.font[0];
	_display.font_h = _display.font[1];
	_display.font += 2;	
#endif			
}
//----------------------------------------------------------
//
//----------------------------------------------------------
void DISPLAY_SetAttribute(uint8_t atr){
	switch(atr){
		default:
		case FONT_NORMAL: _display.drawChar = drawCharSimple;break;
		case FONT_INVERTED: _display.drawChar = drawCharInverted;break;
		case FONT_DOUBLE: _display.drawChar = drawCharDouble;break;
		case FONT_TRANSPARENT: _display.drawChar = drawCharTransparent;break;
	}
}
//----------------------------------------------------------
// setwrap(sx,sy,w,h) tem de ser chamado anteriormente
// a ESTE drawPixel(x,y) pixel com cor Foreground color
//----------------------------------------------------------
void DISPLAY_Pixel(int x0, int y0)
{
    LCD_Pixel(x0,y0,_display.forecolor);
}

