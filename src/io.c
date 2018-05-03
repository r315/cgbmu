
#include <common.h>
#include "dmgcpu.h"
#include "io.h"

//-----------------------------------------
//
//-----------------------------------------
uint8_t readJoyPad(void)
{
static uint8_t button = 0;	
#if defined(__EMU__)

#if defined(__arm__)

#else
SDL_Event ev;
const Uint8 *keys;

	if(!SDL_PollEvent( &ev )) 
	    return button;	    
	    
	if(ev.type == SDL_QUIT)
	    return 255;
	
	keys  = SDL_GetKeyboardState(NULL);

	if(keys[SDL_SCANCODE_ESCAPE]) 
	    return 255;	

	if (ev.type == SDL_KEYDOWN || ev.type == SDL_KEYUP) {		
		button = 0;
	}
	else
	{
		return button;
	}

	button |= keys[SDL_SCANCODE_DOWN] ? J_DOWN : 0;
	button |= keys[SDL_SCANCODE_UP]  ? J_UP : 0;
	button |= keys[SDL_SCANCODE_LEFT] ? J_LEFT : 0;
	button |= keys[SDL_SCANCODE_RIGHT] ? J_RIGHT : 0;
		
	button |= keys[SDL_SCANCODE_RETURN] ? J_START : 0;
	button |= keys[SDL_SCANCODE_BACKSPACE] ? J_SELECT : 0;
	button |= keys[SDL_SCANCODE_A] ? J_B : 0;
    button |= keys[SDL_SCANCODE_S ] ? J_A : 0;

    button |= keys[SDL_SCANCODE_SPACE ] ? J_A : 0;
#endif /* __arm__ */
#elif defined(__BB__)
int	keys = ~LPC_GPIO1->FIOPIN & KEYSMASK;

	button |= ( keys & INPUT_DOWN) ? J_DOWN : 0;
	button |= ( keys & INPUT_UP)  ? J_UP : 0;
	button |= ( keys & INPUT_LEFT) ? J_LEFT : 0;
	button |= ( keys & INPUT_RIGHT) ? J_RIGHT : 0;		
	button |= ( keys & INPUT_A) ? J_START : 0;
#elif defined(__ESP03__) /* __EMU__ */

#endif
	return button;	
}
//-----------------------------------------
//
//-----------------------------------------
uint8_t joyPad(void){
uint8_t buttons = ~readJoyPad();

	IOP1 &= 0xF0;				// Clr lower 4 bits


	if (!(IOP1 & IOP14)) {		// Check wich row was selected
		buttons &= 0x0F;
	}
	else {
		buttons >>= 4;
	}	

	IOP1 |= buttons;
	return IOP1;
}
