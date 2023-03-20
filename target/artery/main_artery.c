#include "board.h"
#include "libbutton.h"
#include "lib2d.h"
#include "liblcd.h"
#include "cgbmu.h"
#include "cartridge.h"
#include "video.h"
#include "dmgcpu.h"



#if !defined(USE_FS)
#define CARTRIDGE_NAME _binary_rom_start
extern const uint8_t CARTRIDGE_NAME;
#endif


// Dark - light
// RGB(0,19, 25), RGB(60,127,38), RGB(170, 204, 71), RGB(248, 255, 178)
const uint16_t lcd_pal[] = { 0xE7DA,0x8E0E,0x334A,0x08C4 };
static uint16_t tft_line[SCREEN_W];

void pushScanLine(uint8_t *scanline){

    uint8_t *end = scanline + SCREEN_W;
    uint8_t pixel = 0;

    while(scanline < end){
		tft_line[pixel++] = lcd_pal[*scanline++];
    }
    
    LCD_WriteArea(SCREEN_OFFSET_X, SCREEN_OFFSET_Y + IOLY, SCREEN_W, 1, tft_line);
}

void prepareFrame(void){
   
}

int loadRom(char *fn)
{
    cartridgeInit(&CARTRIDGE_NAME);
    return ROM_SIZE;
}

uint8_t readJoyPad(void)
{
    uint8_t button = 0;
    //int	keys = ~LPC_GPIO1->FIOPIN & BUTTON_MASK;
    
    /*button |= ( keys & BUTTON_DOWN) ? J_DOWN : 0;
    button |= ( keys & BUTTON_UP)  ? J_UP : 0;
    button |= ( keys & BUTTON_LEFT) ? J_LEFT : 0;
    button |= ( keys & BUTTON_RIGHT) ? J_RIGHT : 0;		
    button |= ( keys & BUTTON_A) ? J_START : 0;
    */
    return button;
}


int main(void)
{
    BOARD_Init();

    LCD_SetOrientation(LCD_LANDSCAPE);
    
    LIB2D_Init();
    LIB2D_Print("CPU %uMHz\n", SystemCoreClock/1000000);
    
    loadRom(NULL);
    
    cgbmu();
    
    while (1)
    {
        LED1_TOGGLE;
        DelayMs(200);
    }

    return 0;
}