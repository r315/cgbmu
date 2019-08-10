
#ifdef __cplusplus
extern "C" {
#endif

#include "board.h"

uint32_t SystemCoreClock;

static volatile uint32_t systicks = 0;

void BB_InitTimeBase(void){
#if defined(USE_SYSTICK_TIMER)
	SysTick_Config((SystemCoreClock / 1000) - 1); // config 1000us
#else
	TIMER_PowerUp(PCTIM3);
	LPC_SC->PCLKSEL1 &= ~(3 << 14);
    LPC_SC->PCLKSEL1 |= (PCLK_1 << 14);
	LPC_TIM3->TCR = TIMER_RESET;
	LPC_TIM3->CCR = 0;				// Timer mode
	LPC_TIM3->PR = (SystemCoreClock / 1000 - 1);
	LPC_TIM3->TCR = TIMER_ENABLE;
#endif
}

//---------------------------------------------------
//	
//---------------------------------------------------
void BOARD_Init(void)
{	
    BB_Init();

	BB_ConfigPLL(PLL100);
	BB_InitTimeBase();

	LCD_Init();
	SPI_Init();
	LCD_Clear(BLACK);
	LCD_Bkl(ON);
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
/* valores dados por NXP lpc17xx.pll.calculator.xls */	
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

		default:
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
	while (!(LPC_SC->PLL0STAT & (1 << 26))); /* Wait for PLOCK0                    */
	LPC_SC->PLL0CON = 0x03; /* PLL0 Enable & Connect              */
	LPC_SC->PLL0FEED = 0xAA;
	LPC_SC->PLL0FEED = 0x55;
	LPC_SC->PCLKSEL0 = 0xAAAAAAAA; /* Peripheral Clock Selection CCLK/2  */
	LPC_SC->PCLKSEL1 = 0xAAAAAAAA; /* 00=CCLK/4 01=CCLK 10=CCLK/2 11=CCLK/8 */

	SystemCoreClockUpdate(); 
}

#if defined(USE_SYSTICK_TIMER)
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
#else
uint32_t GetTicks(void){
	return LPC_TIM3->TC;
}

void DelayMs(uint32_t dl){
uint32_t n;
	while(dl--)
	{
		n = LPC_TIM3->TC;
		while (n == LPC_TIM3->TC);		
	} 
}
#endif
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
