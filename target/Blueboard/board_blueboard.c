
#ifdef __cplusplus
extern "C" {
#endif

#include "board.h"
#include "liblcd.h"
#include "libbutton.h"
#include "lib2d.h"

#define XTAL        (12000000UL)        /* Oscillator frequency               */
#define OSC_CLK     (      XTAL)        /* Main oscillator frequency          */
#define RTC_CLK     (   32000UL)        /* RTC oscillator frequency           */
#define IRC_OSC     ( 4000000UL)        /* Internal RC oscillator frequency   */

#define PLL48   	0
#define PLL72   	1
#define PLL80   	2
#define PLL100  	3

#define SC_PCONP_PCTIM3	(1 << 23)
#define TIM_TCR_CEN		(1 << 0)
#define TIM_TCR_CRST	(1 << 1)

// CCLK Dividers
#define PCLK_1 		1
#define PCLK_2 		2
#define PCLK_4 		0
#define PCLK_8 		3

void BB_InitTimeBase(void){
#if (USE_TIMER_SYSTICK == 1)
	LPC_SC->PCONP |= SC_PCONP_PCTIM3;
	LPC_SC->PCLKSEL1 &= ~(3 << 14);
    LPC_SC->PCLKSEL1 |= (PCLK_1 << 14);
	LPC_TIM3->TCR = TIM_TCR_CRST;
	LPC_TIM3->CCR = 0;				// Timer mode
	LPC_TIM3->PR = (SystemCoreClock / 1000 - 1);
	LPC_TIM3->TCR = TIM_TCR_CEN;
#else
	SysTick_Config((SystemCoreClock / 1000) - 1); // config 1000us
#endif
}

//---------------------------------------------------
//	
//---------------------------------------------------
void BOARD_Init(void)
{	
	SystemInit();
	SystemCoreClockUpdate(); 

	BB_InitTimeBase();

	LEDS_INIT;
	BUTTON_Init(BUTTON_DEFAULT_HOLD_TIME);
	LCD_Init(NULL);
	
	LCD_FillRect(0, 0, LCD_GetWidth(), LCD_GetHeight(), LCD_BLACK);
	LCD_Bkl(ON);

	LIB2D_Init();
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

void _SPI_Init (void) 
{
  // Initialize and enable the SSP Interface module.
  LPC_SC->PCONP       |= (1 << 21);           /* Enable power to SSPI0 block */ 
  LPC_SSP0->CR0        = 0x0000;
  LPC_SSP0->CR1        = 0x0000; 

  // SCK, MISO, MOSI are SSP pins. 
  LPC_PINCON->PINSEL0 &= ~(3UL<<30);          /* P0.15 cleared               */
  LPC_PINCON->PINSEL0 |=  (2UL<<30);          /* P0.15 SCK0                  */
  LPC_PINCON->PINSEL1 &= ~((3<<2) | (3<<4)); /* P0.17, P0.18 cleared        */
  LPC_PINCON->PINSEL1 |=  ((2<<2) | (2<<4)); /* P0.17 MISO0, P0.18 MOSI0    */

  LPC_SC->PCLKSEL1    &= ~(3<<10);            /* PCLKSP0 = CCLK/4 ( 25MHz)   */
  LPC_SC->PCLKSEL1    |=  (1<<10);            /* PCLKSP0 = CCLK   (100MHz)   */

  LPC_SSP0->CPSR       = 250;                 /* 100MHz / 250 = 400kBit      */
                                              /* maximum of 18MHz is possible*/    
  LPC_SSP0->CR0        = 0x0007;              /* 8Bit, CPOL=0, CPHA=0        */
  LPC_SSP0->CR1        = 0x0002;              /* SSP0 enable, master         */

} 

#if (USE_TIMER_SYSTICK == 1)
inline uint32_t GetTick(void){
	return LPC_TIM3->TC;
}

void DelayMs(uint32_t dl){
	uint32_t n;
	while(dl--){
		n = LPC_TIM3->TC;
		while (n == LPC_TIM3->TC);		
	} 
}
#else
static volatile uint32_t systicks = 0;
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
uint32_t GetTick(void)
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
#endif

uint32_t ElapsedTicks(uint32_t start_ticks){
	int32_t delta = GetTick() - start_ticks;
    return (delta < 0) ? -delta : delta;
}
//-----------------------------------------------------		
//
//-----------------------------------------------------		
void __debugbreak(void){
	 asm volatile
    (
        "bkpt #01 \n"
    );
}

#ifdef __cplusplus
}
#endif
