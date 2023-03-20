#include <string.h>
#include "board.h"
#include "cgbmu.h"
#include "dmgcpu.h"
#include "video.h"
#include "cartridge.h"
#include "debug.h"
#include "decoder.h"
#include "lib2d.h"

#if !defined(USE_FS)
//https://balau82.wordpress.com/2012/02/19/linking-a-binary-blob-with-gcc
extern uint8_t _binary__________roms_mario_gb_start;
uint8_t *cartridge = &_binary__________roms_mario_gb_start;
//extern uint8_t _binary__________roms_tests_cpu_instrs_gb_start;
//uint8_t *cartridge = &_binary__________roms_tests_cpu_instrs_gb_start;
#endif

const unsigned short lcd_pal[] = { 0xE7DA,0x8E0E,0x334A,0x08C4 };

// Dark - light
// RGB(0,19, 25), RGB(60,127,38), RGB(170, 204, 71), RGB(248, 255, 178)
//-----------------------------------------
//
//-----------------------------------------
void pushScanLine(uint8_t *scanline){
    uint8_t *end = scanline + SCREEN_W;

    while(scanline < end){
		LCD_Data(lcd_pal[*scanline++]);
    }
}
//-----------------------------------------
//
//-----------------------------------------
inline void prepareFrame(void){
    LCD_Window(SCREEN_OFFSET_X, SCREEN_OFFSET_Y, SCREEN_W, SCREEN_H);  //3us
}
//-----------------------------------------
//
//-----------------------------------------
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
#if defined(USE_FS)
char* f_error(FRESULT res)
{
    static FATFS drive0;
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
#endif
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
            LIB2D_Print("%u\n", t);
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
            LIB2D_Print("%s\n", b);
    }
}
//--------------------------------------------------
//
//--------------------------------------------------
int main (void){
    
    BOARD_Init();	

    LCD_SetOrientation(LCD_LANDSCAPE);
    
    LIB2D_Print("CPU %uMHz\n", SystemCoreClock/1000000);

    DBG_PIN_INIT;

    loadRom("mario.gb");
    
    cgbmu();

    testButtons();
    
    return 0;
}	
