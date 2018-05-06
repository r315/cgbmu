#include <cgbmu.h>
#include <string.h>
#include "blueboard.h"
#include "display.h"
#include "dmgcpu.h"
#include "video.h"
#include "lcd.h"
#include "cartridge.h"
#include "debug.h"
#include "button.h"
#include "decoder.h"
#include <pff/pff.h>

void cgbmu(uint8_t mode);

//-----------------------------------------
//
//-----------------------------------------
uint8_t readJoyPad(void)
{
static uint8_t button = 0;
    int	keys = ~LPC_GPIO1->FIOPIN & KEYSMASK;

	button |= ( keys & INPUT_DOWN) ? J_DOWN : 0;
	button |= ( keys & INPUT_UP)  ? J_UP : 0;
	button |= ( keys & INPUT_LEFT) ? J_LEFT : 0;
	button |= ( keys & INPUT_RIGHT) ? J_RIGHT : 0;		
	button |= ( keys & INPUT_A) ? J_START : 0;
    return button;
}

//--------------------------------------------------
//
//--------------------------------------------------
FATFS drive0;
char* f_error(FRESULT res)
{
	switch(res)
	{
		case FR_OK: return "ok"  ;   
			
		case FR_DISK_ERR: 
			return "disk error";
		case FR_NOT_READY:
			return "disk not ready";
		case FR_NO_FILE:  
			return "no file";
		case FR_NO_PATH:  
			return "invalid path";
		case FR_NOT_OPENED: 
			return "cant open file";
		case FR_NOT_ENABLED:
			return "not enable";
		case FR_NO_FILESYSTEM:
			return "file system";
	}
	return "";
}
//-----------------------------------------------------------
//
//-----------------------------------------------------------
void fsInit(void)
{
	DBG_Info(f_error(pf_mount(&drive0)));
	ROM0 = (unsigned char*)0x2007C000;
    ROMBANK = (unsigned char*)0x20080000;
}
//--------------------------------------------------
//
//--------------------------------------------------
int loadRombank(uint8_t bank)
{
WORD n;	
	//drawNumber(232,0,bankSelect,10);
	pf_lseek(bank << 14);
	pf_read(ROMBANK, ROM_SIZE, &n);
	//DISPLAY_printf("Loaded %u bytes into Rom Bank %u\n",n, bankSelect);
	//drawChar(232,0,' ');
	bankselect = bank;		// save current
	return n;
}
//--------------------------------------------------
//
//--------------------------------------------------
int loadRom(char *fn)
{
WORD n;
	if(!drive0.fs_type){
		fsInit();
	}
	DBG_Info("Opening");
	DBG_Info(fn);
	DBG_Info(f_error(pf_open(fn)));
	DBG_Info("Loading ROM0");
	DBG_Info(f_error(pf_read(ROM0, ROM_SIZE, &n)));
	loadRombank(1);	
	return n;
}

int main (void){
    
    BB_Init();
	BB_ConfigPLL(PLL100);	

	LCD_Rotation(LCD_LANDSCAPE);

	DISPLAY_puts("Hello\n");

	initCpu();		

	if(loadRom("mario.gb"))
		cgbmu(1);

    return 0;
}	
