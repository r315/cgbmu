/*
07-05-2013 adicionada lcdclr no metodo lcdinit

*/
#include "blueboard.h"
#include <lpc17xx.h>
#include "lcd.h"

//--------------------------------------------------------
//
//--------------------------------------------------------				 
int lcdreaddata(void)
{
uint16_t dta;
	LCDRS1; DATAPORT->FIODIR &= ~0xFF;
	LCDRD0;LCDRD0;LCDRD0; dta = DATAPORT->FIOPIN0 << 8; LCDRD1;
	LCDRD0;LCDRD0;LCDRD0; dta |=DATAPORT->FIOPIN0; LCDRD1;
	DATAPORT->FIODIR |= 0xFF;
return dta;
}
//--------------------------------------------------------
//
//--------------------------------------------------------				 
#if 0
int lcdreadreg(void)
{
uint16_t reg;
	LCDRS0; DATAPORT->FIODIR &= ~0xFF;
	LCDRD0;LCDRD0;LCDRD0; reg = DATAPORT->FIOPIN0 << 8; LCDRD1;
	LCDRD0;LCDRD0;LCDRD0; reg |=DATAPORT->FIOPIN0; LCDRD1;
	DATAPORT->FIODIR |= 0xFF;
return reg;
}
#endif
//--------------------------------------------------------
//write data to LCD
//--------------------------------------------------------
void lcddata(int dat)
{	
	LCDRS1;	
	DATAPORT->FIOPIN0 = dat >> 8; // MSB
	LCDWR0;LCDWR0;LCDWR1;
	DATAPORT->FIOPIN0 = dat;      // LSB
	LCDWR0;LCDWR0;LCDWR1;	
}
//--------------------------------------------------------
//write instruction to LCD
//--------------------------------------------------------
void lcdcmd(char ins) 
{
	LCDRS0;	
	DATAPORT->FIOPIN0 = 0;
	LCDWR0;LCDWR0;LCDWR1;
	DATAPORT->FIOPIN0 = ins;
	LCDWR0;LCDWR0;LCDWR1;
}
//--------------------------------------------------------
//
//--------------------------------------------------------
void solidfill(int count, int color)
{			
	LCDRS1;
	while(count--)
	{
		DATAPORT->FIOPIN0 = color>>8;
		LCDWR0;LCDWR1;
		DATAPORT->FIOPIN0 = color;	
		LCDWR0;LCDWR1;
	}
}
//-----------------------------------------------------*/
//
//------------------------------------------------------
void setgram(int x, int y)
{
	lcdcmd(GRAM_ADX);
	lcddata(x);
   	lcdcmd(GRAM_ADY);
	lcddata(y);
    lcdcmd(LCD_RW_GRAM);
	
}
//-------------------------------------------------------------------
//	 	 
//-------------------------------------------------------------------
void setwrap(int x, int y, int width, int height)
{    
    lcdcmd(START_ADX);
	lcddata(x);
	lcdcmd(END_ADX);
	lcddata(x + width-1);
	lcdcmd(START_ADY);
	lcddata(y);
	lcdcmd(END_ADY);
	lcddata(y + height-1);
}
//-------------------------------------------------------------------
// setwrap(sx,sy,w,h) tem de ser executado antes
// de chamar ESTE drawpixel(x,y,c) 
//-------------------------------------------------------------------
void putpixel(int x, int y, int c)
{
	setgram(x,y);
	lcddata(c);
}
//-----------------------------------------------------------------*/
//	 set VLE bit on GATE_SCAN_CTRL2(61h) first
//-------------------------------------------------------------------
void hwscroll(int y)
{
	while (y < 0)
 		y += 320;
	while (y >= 320)
 		y -= 320;	
 	
	lcdcmd(LCD_GATE_SCAN_CTRL3);
	lcddata(y); 
}
//--------------------------------------------------------
// initialize lcd
//--------------------------------------------------------
void lcdInit(void) //initial LCD
{
	
	LCDCS1;LCDWR1;LCDRD1;
	LCDRST0;		 
	DelayMs(100);
	LCDRST1;
	LCDCS0;

	lcdcmd(0xE5);
	lcddata(0x8000); 					//set the internal vcore voltage
	lcdcmd(LCD_START_OSC);
	lcddata(0x0001); 					//start oscillator
	DelayMs(50);	

	lcdcmd(LCD_DRIV_OUT_CTRL);
	lcddata(SHIFT_DIR);
	lcdcmd(LCD_DRIV_WAV_CTRL);
	lcddata(0x0700); 					//set 1 line inversion
	
	lcdcmd(LCD_ENTRY_MOD);
	lcddata(VAL_ENTRY_MOD);			//set GRAM write direction, BGR=0

	lcdcmd(LCD_RESIZE_CTRL);
	lcddata(0x0000); 					//no resizing

	lcdcmd(LCD_DISP_CTRL2);
	lcddata(0x0202); 					//front & back porch periods = 2
	lcdcmd(LCD_DISP_CTRL3);
	lcddata(0x0000); 					
	lcdcmd(LCD_DISP_CTRL4);
	lcddata(0x0000); 					
	lcdcmd(LCD_RGB_DISP_IF_CTRL1);
	lcddata(0x0000); 					//select system interface				
	lcdcmd(LCD_FRM_MARKER_POS);
	lcddata(0x0000); 					
	lcdcmd(LCD_RGB_DISP_IF_CTRL2);
	lcddata(0x0000);					
	
	lcdcmd(LCD_POW_CTRL1);
	lcddata(0x0000);
	lcdcmd(LCD_POW_CTRL2);
	lcddata(0x0000); 					
	lcdcmd(LCD_POW_CTRL3);
	lcddata(0x0000);
	lcdcmd(LCD_POW_CTRL4);
	lcddata(0x0000); 					
	DelayMs(200);

	lcdcmd(LCD_POW_CTRL1);
	lcddata(0x17B0);
	lcdcmd(LCD_POW_CTRL2);
	lcddata(0x0137); 					
	DelayMs(50);

	lcdcmd(LCD_POW_CTRL3);
	lcddata(0x013C);
	DelayMs(50);

	lcdcmd(LCD_POW_CTRL4);
	lcddata(0x1400);
	lcdcmd(LCD_POW_CTRL7);
	lcddata(0x0007);
	DelayMs(50);	

	lcdcmd(LCD_GAMMA_CTRL1);
	lcddata(0x0007);
	lcdcmd(LCD_GAMMA_CTRL2);
	lcddata(0x0504);
	lcdcmd(LCD_GAMMA_CTRL3);
	lcddata(0x0703);
	lcdcmd(LCD_GAMMA_CTRL4);
	lcddata(0x0002);
	lcdcmd(LCD_GAMMA_CTRL5);
	lcddata(0x0707);
	lcdcmd(LCD_GAMMA_CTRL6);
	lcddata(0x0406);
	lcdcmd(LCD_GAMMA_CTRL7);
	lcddata(0x0006);
	lcdcmd(LCD_GAMMA_CTRL8);
	lcddata(0x0404);
	lcdcmd(LCD_GAMMA_CTRL9);
	lcddata(0x0700);
	lcdcmd(LCD_GAMMA_CTRL10);
	lcddata(0x0A08);

	lcdcmd(LCD_GRAM_HOR_AD);
	lcddata(0x0000);
	lcdcmd(LCD_GRAM_VER_AD);
	lcddata(0x0000);
	lcdcmd(LCD_HOR_START_AD);
	lcddata(0x0000);
	lcdcmd(LCD_HOR_END_AD);
	lcddata(0x00EF);
	lcdcmd(LCD_VER_START_AD);
	lcddata(0x0000);
	lcdcmd(LCD_VER_END_AD);
	lcddata(0x013F);
	lcdcmd(LCD_GATE_SCAN_CTRL1);
	lcddata(VAL_GATE_SCAN);
	lcdcmd(LCD_GATE_SCAN_CTRL2);
	lcddata(0x0003);		//
	lcdcmd(LCD_GATE_SCAN_CTRL3);
	lcddata(0x0000);

	lcdcmd(LCD_PART_IMG1_DISP_POS);
	lcddata(0x0000);
	lcdcmd(LCD_PART_IMG1_START_AD);
	lcddata(0x0000);
	lcdcmd(LCD_PART_IMG1_END_AD);
	lcddata(0x0000);
	lcdcmd(LCD_PART_IMG2_DISP_POS);
	lcddata(0x0000);
	lcdcmd(LCD_PART_IMG2_START_AD);
	lcddata(0x0000);
	lcdcmd(LCD_PART_IMG2_END_AD);
	lcddata(0x0000);

	lcdcmd(LCD_PANEL_IF_CTRL1);
	lcddata(0x0010);
	lcdcmd(LCD_PANEL_IF_CTRL2);
	lcddata(0x0000);
	lcdcmd(LCD_PANEL_IF_CTRL3);
	lcddata(0x0003);
	lcdcmd(LCD_PANEL_IF_CTRL4);
	lcddata(0x0110);
	lcdcmd(LCD_PANEL_IF_CTRL5);
	lcddata(0x0000);
	lcdcmd(LCD_PANEL_IF_CTRL6);
	lcddata(0x0000);

	lcdcmd(LCD_DISP_CTRL1);
	lcddata(0x0173);
	DelayMs(1000);

}

