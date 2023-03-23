#include "board.h"
#include "dmgcpu.h"
#include "cartridge.h"
#include "cgbmu.h"

#if !defined(USE_FS)
extern uint8_t _binary_rom_start;
uint8_t *cartridge = &_binary_rom_start;
#endif

int loadRom(char *fn)
{
    //cartridgeInit(cartridge);
    //return ROM_SIZE;
    return 0;
}

int main(void){
    BOARD_Init();
    VIDEO_Init();

    BSP_LCD_SetTextColor(LCD_COLOR_RED);
    BSP_LCD_SetBackColor(LCD_COLOR_BLACK);

    cgbmu(cartridge);
}

uint8_t readButtons(void)
{
uint8_t button = 0;

    return button;
}