#include "board.h"
#include "libbutton.h"
#include "lib2d.h"
#include "liblcd.h"
#include "cgbmu.h"
#include "cartridge.h"



#if !defined(USE_FS)
extern const uint8_t _binary__________roms_mario_gb_start __attribute__((section(".rodata")));
uint8_t *cartridge = (uint8_t*)&_binary__________roms_mario_gb_start;
//extern uint8_t _binary__________roms_tests_cpu_instrs_gb_start;
//uint8_t *cartridge = &_binary__________roms_tests_cpu_instrs_gb_start;
#endif

int loadRom(char *fn)
{
    cartridgeInit(cartridge);
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

    cgbmu();
    
    while (1)
    {
        LED1_TOGGLE;
        DelayMs(200);
    }

    return 0;
}