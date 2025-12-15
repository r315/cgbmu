#include "board.h"
#include "spi.h"
#include "gpio.h"
#include "libbutton.h"
#include "drvlcd.h"

static spibus_t spidev;
static drvlcdspi_t drvlcd;

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

    RCC->APB2EN |= RCC_APB2EN_AFIOEN | RCC_APB2EN_GPIOAEN | RCC_APB2EN_GPIOBEN | RCC_APB2EN_GPIOCEN;
    AFIO->MAP = AFIO_MAP_SWJTAG_CONF_JTAGDISABLE;

	InitTimeBase();

    spidev.bus = SPI_BUS1;
    spidev.freq = SPI_FREQ;
    spidev.cfg = SPI_CFG_DMA;
    SPI_Init(&spidev);

	LED1_PIN_INIT;
    LCD_PIN_INIT;

	BUTTON_Init(BUTTON_DEFAULT_HOLD_TIME);

    #if defined(BOARD_PWRKT)
    drvlcd.w = 240;
    drvlcd.h = 240;
    drvlcd.cs = PB_12;
    drvlcd.cd = PA_15;
    drvlcd.rst = 255;
    drvlcd.bkl = PB_2;
    drvlcd.spidev = &spidev;

    GPIO_Config(drvlcd.cs, GPO_PP);
    GPIO_Config(drvlcd.cd, GPO_PP);
    GPIO_Config(drvlcd.bkl, GPO_PP);

    AFIO->MAP = AFIO_MAP_SWJTAG_CONF_JTAGDISABLE;
    LCD_Init(&drvlcd);
    LCD_SetOrientation(LCD_LANDSCAPE);
    #endif

	LCD_Bkl(ON);
	LCD_FillRect(0,0, drvlcd.w, drvlcd.h, LCD_BLACK);
}

void __debugbreak(void){
	 asm volatile
    (
        "bkpt #01 \n"
    );
}