/*************************************************
14-07-2011 alterado def de _osc para pll 

**************************************************/

#ifndef _blueboard_h_
#define _blueboard_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <LPC17xx.h>
#include <stdint.h>

/* valores dados por NXP lpc17xx.pll.calculator.xls */	 

#define PLL48   0
#define PLL72   1
#define PLL80   2
#define PLL100  3

#define XTAL        (12000000UL)        /* Oscillator frequency               */
#define OSC_CLK     (      XTAL)        /* Main oscillator frequency          */
#define RTC_CLK     (   32000UL)        /* RTC oscillator frequency           */
#define IRC_OSC     ( 4000000UL)        /* Internal RC oscillator frequency   */
//-----------------------------------------------------
// Joy pins P1.14, p1.15, p1.16, p1.17, p1.28
//-----------------------------------------------------
#define KEYSMASK        (INPUT_A | INPUT_UP | INPUT_DOWN | INPUT_LEFT | INPUT_RIGHT) //0x1003C000
#define INPUT_A         (1<<14) //0x10038000
#if defined(ORIENTATION90)
    #define INPUT_UP    (1<<15) //0x10034000
	#define INPUT_DOWN  (1<<17) //0x1001C000
	#define INPUT_LEFT  (1<<16) //0x1002C000
	#define INPUT_RIGHT (1<<28) //0x0003C000		
#elif defined(ORIENTATION270)
	#define INPUT_UP    (1<<17) //0x1001C000
	#define INPUT_DOWN  (1<<15) //0x10034000
	#define INPUT_LEFT  (1<<28) //0x0003C000
	#define INPUT_RIGHT (1<<16) //0x1002C000 	
#else
	#define INPUT_UP    (1<<16) //0x1002C000
	#define INPUT_DOWN  (1<<28) //0x0003C000
	#define INPUT_LEFT  (1<<17) //0x1001C000
	#define INPUT_RIGHT (1<<15) //0x10034000	
#endif 
//-----------------------------------------------------
#ifndef LEDS
	#define LED1 (1<<29) //P1.29
	#define LED1_ON  LPC_GPIO1->FIOSET = LED1
	#define LED1_OFF LPC_GPIO1->FIOCLR = LED1
	
	#define LED2 (1<<18) //P1.18
	#define LED2_OFF LPC_GPIO1->FIOSET = LED2
	#define LED2_ON  LPC_GPIO1->FIOCLR = LED2
	
	#define LED3 (1<<9)  //P2.9
	#define LED3_OFF LPC_GPIO2->FIOSET = LED3
	#define LED3_ON  LPC_GPIO2->FIOCLR = LED3
#endif

#define ACCEL_CS (1<<6)
#define SELECT_ACCEL()    LPC_GPIO0->FIOCLR = ACCEL_CS
#define DESELECT_ACCEL()  LPC_GPIO0->FIOSET = ACCEL_CS

#define MMC_CS (1<<16)
#define SELECT_CARD()	LPC_GPIO0->FIOCLR = MMC_CS		/* MMC CS = L */
#define	DESELECT_CARD()	LPC_GPIO0->FIOSET = MMC_CS		/* MMC CS = H */
#define	MMC_SEL       !(LPC_GPIO0->FIOPIN & MMC_CS)		/* MMC CS status (true:selected) */



//#define FALSE 0
//#define TRUE 1 
#define HOLD 3
//-----------------------------------------------------
// funcoes tipo arduino
//-----------------------------------------------------
#define pinMode(pin,dir)  LPC_GPIO0->FIODIR |= (1<<pin)
#define digitalWrite(pin,value)

//-----------------------------------------------------

//-----------------------------------------------------
typedef struct{
	int  Key;
	char Flag;
}KEYS;

extern volatile uint32_t systicks;
extern volatile KEYS _KEYS;
//-----------------------------------------------------
char scanKeys(void);
char keyDown(unsigned int key);
void BB_Init(void);
int getSysFreq(void);
unsigned int ADCIN(unsigned char channelNum );
void DelayMs(uint32_t dl);
uint32_t GetTicks(void);
void BB_ConfigPLL(uint8_t fmhz);

//-----------------------------------------------------
// double = float ???
// volatile = variavel na ram
//typedef unsigned char		U8;		/**< unsigned 8-bit */
//typedef unsigned short int	U16;	/**< unsigned 16-bit */
//typedef unsigned int		U32;	/**< unsigned 32-bit */
//-----------------------------------------------------
#ifdef __cplusplus
}
#endif

#endif






