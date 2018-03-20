
#include <common.h>
#include <string.h>
#include "graphics.h"
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
		interrupts();
		decode();
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

