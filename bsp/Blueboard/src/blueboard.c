// File Name: blueboard.c
// Date:  	  6-10-2010
/* 
07-05-2013 void getSysFreq(void) -> int getSysFreq(void)
			removida a chamada lcdInit() no metodo blueboardInit()
TODO: inicializar clock usb
*/

#include "blueboard.h"
#include <lcd.h>
#include "spi.h"

#ifdef __cplusplus
extern "C" {
#endif

uint32_t SystemCoreClock;
volatile uint32_t systicks = 0;
volatile KEYS _KEYS;
//---------------------------------------------------
//	
//---------------------------------------------------
void BB_Init(void)
{	
    // Init LEDS
	LPC_GPIO1->FIODIR |= LED1|LED2;
	LPC_GPIO2->FIODIR |= LED3;
	LED1_OFF;
	LED2_OFF;
	LED3_OFF;

    // accelerometer cs pin
    LPC_GPIO0->FIODIR   |= ACCEL_CS;  // en cs pin   	
    LPC_PINCON->PINSEL0 &= ~(3<<12);  // P0.6 (used as GPIO)
	DESELECT_ACCEL();    

    // mmc cs pin
	LPC_GPIO0->FIODIR   |=  MMC_CS;   /* SET MMC_CS pin  as output */	
	LPC_PINCON->PINSEL1 &= ~(3<<0);  /* P0.16 (used as GPIO)   */ 	
	DESELECT_CARD();	

	LCD_Init();
	SPI_Init();
	LCD_BKL_ON;
}
//---------------------------------------------------
//	
//---------------------------------------------------
void BB_ClockOut(uint8_t en){
	if(en){
		LPC_SC->CLKOUTCFG = (1<<4)|/* CCLK/2 */ (1<<8);/* CLKOU_EN*/
		LPC_PINCON->PINSEL3 |= (1<<22);// P1.27 CLKOUT 
	}else{
		LPC_SC->CLKOUTCFG = (1<<4)|/* CCLK/2 */ (1<<8);/* CLKOU_EN*/
		LPC_PINCON->PINSEL3 &= ~(1<<22);
	}
}
//---------------------------------------------------
//	
//---------------------------------------------------
void BB_ConfigPLL(uint8_t fmhz)
{
uint8_t Mvalue, Nvalue, CCLKdiv;

switch (fmhz)
{
	case PLL48:
		/* Fcclk = 48Mhz, Fcco = 288Mhz */
		Mvalue = 12; Nvalue = 1; CCLKdiv = 6; break;

	case PLL72:
		/* Fcclk = 72Mhz, Fcco = 288Mhz */
		Mvalue = 24; Nvalue = 2; CCLKdiv = 4; break;

	case PLL100:
		/* Fcclk = 100Mhz, Fcco = 300Mhz */
		Mvalue = 25; Nvalue = 2; CCLKdiv = 3; break;

	case PLL80:
		/* Fcclk = 80Mhz, Fcco = 400Mhz */
		Mvalue = 50; Nvalue = 3; CCLKdiv = 5; break;
}

LPC_SC->SCS = 0x20; /* Main Oscillator enable             */
while (!(LPC_SC->SCS & 0x40));								/* Wait for Oscillator to be ready    */
LPC_SC->CCLKCFG = CCLKdiv - 1; /* Setup Clock Divider                */
LPC_SC->CLKSRCSEL = 1;				/* Main oscillator as Clock Source    */
LPC_SC->PLL0CFG = (Nvalue - 1) << 16 | (Mvalue - 1);
LPC_SC->PLL0CON = 0x01; /* PLL0 Enable                        */
LPC_SC->PLL0FEED = 0xAA;
LPC_SC->PLL0FEED = 0x55;
while (!(LPC_SC->PLL0STAT & (1 << 26)))
	;					/* Wait for PLOCK0                    */
LPC_SC->PLL0CON = 0x03; /* PLL0 Enable & Connect              */
LPC_SC->PLL0FEED = 0xAA;
LPC_SC->PLL0FEED = 0x55;
LPC_SC->PCLKSEL0 = 0xAAAAAAAA; /* Peripheral Clock Selection CCLK/2  */
LPC_SC->PCLKSEL1 = 0xAAAAAAAA; /* 00=CCLK/4 01=CCLK 10=CCLK/2 11=CCLK/8 */

SystemCoreClockUpdate();
SysTick_Config((SystemCoreClock / 1000) - 1); // config 1000us 
}

//---------------------------------------------------
//	
//---------------------------------------------------
char keyDown(unsigned int key)
{
	if(!_KEYS.Flag)			   // any key pressed?
		return 0;		       // return FALSE

	if((_KEYS.Key & key))	   // is this key pressed? keys read '0'
		return 0;		       // not this key pressed
	else
		return _KEYS.Flag;	   // return key state TRUE or HOLD
}
//---------------------------------------------------
//	
//---------------------------------------------------
char scanKeys(void)
{
	_KEYS.Key = ~LPC_GPIO1->FIOPIN & KEYSMASK;

	if(_KEYS.Key == KEYSMASK) // any key pressed?
	{
		_KEYS.Flag = 0;			
		return 0;       // no key pressed
	}
		
	if(!_KEYS.Flag)			// first press?
	{
		_KEYS.Flag = 1;	// return true first press
		return 1;
	}
	_KEYS.Flag = HOLD;	    // key still pressed return hold
	return HOLD;       	
}
//---------------------------------------------------
//	
/*/---------------------------------------------------
char KeyDown(unsigned int key)
{
	if((_KEYS & key))	   // key pressed = '0'
		return FALSE;
	else
		return TRUE;
}
//---------------------------------------------------
//	
//---------------------------------------------------
char ScanKeys(void)
{
	_KEYS = LPC_GPIO1->FIOPIN & KEYSMASK;

	if(_KEYS == KEYSMASK)			
		return FALSE;       // no key pressed
	
	else		
		return TRUE;       //new key pressed
		
}*/
//-----------------------------------------------------									   
// SysTick Interrupt Handler (1ms)   
//-----------------------------------------------------
void SysTick_Handler(void)
{
	systicks++;
	//LPC_GPIO1->FIOPIN ^= TEST_LED; 
}
//-----------------------------------------------------		
//
//-----------------------------------------------------		
uint32_t GetTicks(void)
{
	return systicks;
}
//-----------------------------------------------------		
//
//-----------------------------------------------------		
void DelayMs(uint32_t dl)
{
uint32_t n;
	while(dl--)
	{
		n = systicks;
		while (n == systicks);		
	} 
}
//-----------------------------------------------------		
//
//-----------------------------------------------------		
void SystemCoreClockUpdate (void)
{

	SystemCoreClock = IRC_OSC;

 /* Determine clock frequency according to clock register values             */
	if (((LPC_SC->PLL0STAT >> 24)&3)==3) 
	{
		switch (LPC_SC->CLKSRCSEL & 0x03) 
		{
			case 0:
      		case 3:
        	SystemCoreClock = (IRC_OSC * 
                          ((2 * ((LPC_SC->PLL0STAT & 0x7FFF) + 1)))  /
                          (((LPC_SC->PLL0STAT >> 16) & 0xFF) + 1)    /
                          ((LPC_SC->CCLKCFG & 0xFF)+ 1));
						  break;

      		case 1: /* Main oscillator => PLL0            */
        	SystemCoreClock = (OSC_CLK * 
                          ((2 * ((LPC_SC->PLL0STAT & 0x7FFF) + 1)))  /
                          (((LPC_SC->PLL0STAT >> 16) & 0xFF) + 1)    /
                          ((LPC_SC->CCLKCFG & 0xFF)+ 1));
						  break;

			case 2: /* RTC oscillator => PLL0             */
        	SystemCoreClock = (RTC_CLK * 
                          ((2 * ((LPC_SC->PLL0STAT & 0x7FFF) + 1)))  /
                          (((LPC_SC->PLL0STAT >> 16) & 0xFF) + 1)    /
                          ((LPC_SC->CCLKCFG & 0xFF)+ 1));
						  break;
    	}
  	} 
	else 
	{
    	switch (LPC_SC->CLKSRCSEL & 0x03) 
		{
      		case 0:
      		case 3:/* Reserved, default to Internal RC   */
        	SystemCoreClock = IRC_OSC / ((LPC_SC->CCLKCFG & 0xFF)+ 1);
			break;

			case 1:/* Main oscillator => PLL0            */
        	SystemCoreClock = OSC_CLK / ((LPC_SC->CCLKCFG & 0xFF)+ 1);
			break;

	 		case 2:/* RTC oscillator => PLL0             */
        	SystemCoreClock = RTC_CLK / ((LPC_SC->CCLKCFG & 0xFF)+ 1);
			break;
    	}
	}
}

#ifdef __cplusplus
}
#endif

