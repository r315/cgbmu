#include "board.h"
#include "spi.h"
#include "gpio.h"
#include "libbutton.h"
#include "liblcd.h"
#include "lib2d.h"

static spibus_t spibus;

static void InitTimeBase(void){
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

#if (USE_TIMER_SYSTICK == 1)
#else
static volatile uint32_t ticms;
void SysTick_Handler(void){
    ticms++;
}

void DelayMs(uint32_t ms){
    __IO uint32_t end = ticms + ms;
    while (ticms < end){ }
}

uint32_t ElapsedTicks(uint32_t start_ticks){
	int32_t delta = GetTick() - start_ticks;
    return (delta < 0) ? -delta : delta;
}

inline uint32_t GetTick(void)
{
    return ticms;
}
#endif

void BOARD_Init(void)
{	
	SystemInit();
	SystemCoreClockUpdate(); 

	InitTimeBase();

    spibus.bus = SPI_BUS0;
    spibus.freq = SPI_FREQ;
    spibus.flags = SPI_HW_CS;
    SPI_Init(&spibus);

	LED1_PIN_INIT;
    LCD_PIN_INIT;

	BUTTON_Init(BUTTON_DEFAULT_HOLD_TIME);
    
    LCD_Init(&spibus);

	LIB2D_Init();
	LCD_Clear(LCD_RED);

    LCD_Bkl(ON);
}