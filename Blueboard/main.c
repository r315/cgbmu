

#include <display.h>
#include <dmgcpu.h>
#include <video.h>
#include <decoder.h>
#include <lcd.h>
#include <cartridge.h>
#include <debug.h>
#include <button.h>
#include <io.h>
#include <blueboard.h>

void run(void) {
	while (readJoyPad() != 255) {		
		interrupts();
		decode();
		timer();			
		video();
	}
}

void testMain(void);
//-----------------------------------------------------------
//instructions test
//-----------------------------------------------------------
int main (void)
{
	BB_Init();
	BB_ConfigPLL(PLL100);
	//testMain();
	//return 0;

	DISPLAY_puts("Hello\n");

	initCpu();	
	loadRom("mario.gb");
	run();	

	return 0;
}

