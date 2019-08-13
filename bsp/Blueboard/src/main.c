#include <string.h>
#include "board.h"
#include "cgbmu.h"
#include "dmgcpu.h"
#include "video.h"
#include "cartridge.h"
#include "debug.h"
#include "decoder.h"


void cgbmu(uint8_t mode);

#if !defined(USE_FS)
extern uint8_t _binary__________roms_mario_gb_start;
uint8_t *cartridge = &_binary__________roms_mario_gb_start;
//extern uint8_t _binary__________roms_tests_cpu_instrs_gb_start;
//uint8_t *cartridge = &_binary__________roms_tests_cpu_instrs_gb_start;
#endif
//-----------------------------------------
//
//-----------------------------------------
FAST_CODE
uint8_t readJoyPad(void)
{
uint8_t button = 0;
int	keys = ~LPC_GPIO1->FIOPIN & BUTTON_MASK;
	button |= ( keys & BUTTON_DOWN) ? J_DOWN : 0;
	button |= ( keys & BUTTON_UP)  ? J_UP : 0;
	button |= ( keys & BUTTON_LEFT) ? J_LEFT : 0;
	button |= ( keys & BUTTON_RIGHT) ? J_RIGHT : 0;		
	button |= ( keys & BUTTON_A) ? J_START : 0;
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
#if defined(USE_FS)
	DBG_Info(f_error(pf_mount(&drive0)));
	rom0 = (uint8_t*)0x2007C000;
    rombank = (uint8_t*)0x20080000;
#endif    
}
//--------------------------------------------------
//
//--------------------------------------------------
int loadRom(char *fn)
{
#if defined(USE_FS)
	WORD n;
	if(!drive0.fs_type){
		fsInit();
	}
	DBG_Info("Opening");
	DBG_Info(fn);
	DBG_Info(f_error(pf_open(fn)));
	DBG_Info("Loading ROM0");
	DBG_Info(f_error(pf_read(rom0, ROM_SIZE, &n)));
	loadRombank(1);	
	return n;
#else
	cartridgeInit(cartridge);
	return ROM_SIZE;
#endif
}
//--------------------------------------------------
//
//--------------------------------------------------

void testButtons(void) {
	char *b, t;
	//printf("Buttons Test\n");
	while (readJoyPad() != 255) {
		b = "";
		IOP1 = IOP14;
		t = joyPad() & 0x0f;
		switch (t) {
		case 0x0e:  b = "  [ A ]   "; break;
		case 0x0d:  b = "  [ B ]   "; break;
		case 0x0b:  b = "[ SELECT ]"; break;
		case 0x07:  b = "[ START ] "; break;
		case 15: break;
		default:
			DISPLAY_printf("%u\n", t);
			break;
		}
		IOP1 = IOP15;
		switch (joyPad() & 0x0f) {
		case 0x0e:   b = "[ RIGHT ] "; break;
		case 0x0d:   b = "[ LEFT  ] "; break;
		case 0x0b:   b = "[  UP  ]  "; break;
		case 0x07:   b = "[ DOWN ]  "; break;
		}
		if (*b != '\0')
			DISPLAY_printf("%s\n", b);
	}
}
//--------------------------------------------------
//
//--------------------------------------------------
#ifdef SEMI_HOSTING
extern void initialise_monitor_handles(void);
#endif
int main (void){
    
    BOARD_Init();	

	LCD_Rotation(LCD_LANDSCAPE);
	
	DISPLAY_printf("Hello \nClock %uMHz\n", SystemCoreClock/1000000);

	DBG_PIN_INIT;
#ifdef SEMI_HOSTING
	initialise_monitor_handles();
	printf("\n\nTesting semihosting\n\n");
#endif
	if(loadRom("mario.gb"))
		cgbmu(0);
	testButtons();
    
	return 0;
}	
