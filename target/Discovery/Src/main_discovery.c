#include "board.h"
#include "dmgcpu.h"
#include "cartridge.h"
#include "cgbmu.h"

#if !defined(USE_FS)
extern uint8_t _binary__________roms_mario_gb_start;
uint8_t *cartridge = &_binary__________roms_mario_gb_start;
//extern uint8_t _binary__________roms_tests_cpu_instrs_gb_start;
//uint8_t *cartridge = &_binary__________roms_tests_cpu_instrs_gb_start;
#endif

int loadRom(char *fn)
{
    cartridgeInit(cartridge);
    return ROM_SIZE;
}

int main(void){
    BOARD_Init();
    VIDEO_Init();

    BSP_LCD_SetTextColor(LCD_COLOR_RED);
    BSP_LCD_SetBackColor(LCD_COLOR_BLACK);

    cgbmu();
}

uint8_t readJoyPad(void)
{
uint8_t button = 0;

    return button;
}