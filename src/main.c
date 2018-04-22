
#include <common.h>
#include <string.h>
#include "display.h"
#include "dmgcpu.h"
#include "video.h"
#include "lcd.h"
#include "cartridge.h"
#include "debug.h"
#include "button.h"
#include "decoder.h"
#include "io.h"
#include "tests.h"

void run(void) {
	int wait = 0;
	while (readJoyPad() != 255) {		
		decode();
		interrupts();
		timer();
		if (video() == ON) {
			//DBG_Fps();
			wait = wait - GetTicks();
			if (wait > 0)
				DelayMs(wait);
			wait = GetTicks() + 16;
		}
	}
}
//-----------------------------------------------------------
//instructions test
//-----------------------------------------------------------
int main (int argc, char *argv[])
{
	#if defined(__BB__)
	BB_Init();
	BB_ConfigPLL(PLL100);	

	LCD_Rotation(LCD_LANDSCAPE);

	DISPLAY_puts("Hello\n");

	initCpu();		

	loadRom("mario.gb");
	run();	

	#elif defined(__ESP03__)
	#define HSPI_DIVIDER 1
	
	int i = 0;
	nosdk8266_init();

	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U,FUNC_GPIO2);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U,FUNC_GPIO5);

	Cache_Read_Disable();	
	Cache_Read_Enable(0,0,1);	

    	//printf( "Starting Display %p\n", &LCD_Init);

    	HSPI_Init(HSPI_DIVIDER, HSPI_MODE_TX);
	//HSPI_Configure_CS(LCD_CS);
	LCD_Init();
	LCD_Clear(RED);
	LCD_Bkl(ON);

	LCD_Rect(0,0,120,120,BLUE);

	GPIO_OUTPUT_SET(0,0);

	float val = 3.141592;
	DISPLAY_printf("Float Test\nPI: %f\n", val);

	LCD_Rotation(LCD_LANDSCAPE);

	initCpu();
	
	//testAll();		


	#elif defined(__EMU__)	
	
	LCD_Init();	 
	//testMain();	

	initCpu();	

	//testMain();

	switch(argc){
		case 1:
			testAll();
			break;
		
		case 2:
			if (!strcmp(argv[1], "test")) {
				testMain();
				break;
			}
			loadRom(argv[1]);
			run();
			break;

		case 3:
			if(!strcmp(argv[1], "debug")){
				loadRom(argv[2]);
				debug();
			}
			else if (!strcmp(argv[1], "test")) {
				testRom(argv[2]);
			}
			else{
				loadRom(argv[2]);
				run();
			}

		default: 
			break;
	}
	

	LCD_Close();
	#endif
	return 0;
}

