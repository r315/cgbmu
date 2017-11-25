
#include "dmgcpu.h"
#include "io.h"
#ifdef __EMU__
#ifdef WIN32
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif
#endif
//-----------------------------------------
//
//-----------------------------------------
uint8_t readJoyPad(void)
{
#ifdef __EMU__
SDL_Event ev;
const Uint8 *keys;
static uint8_t button = 0;

	if(!SDL_PollEvent( &ev )) 
	    return button;	    
	    
	if(ev.type == SDL_QUIT)
	    return -1;
	
	keys  = SDL_GetKeyboardState(NULL);

	if(keys[SDL_SCANCODE_ESCAPE]) 
	    return -1;	

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
  
	return button;    
#else
int	keys = LPC_GPIO1->FIOPIN & KEYSMASK;
	_IOP14 = 0;
	_IOP15 = 7;	
	if(keys&INPUT_DOWN) _IOP14 |= (1<<3);
	if(keys&INPUT_UP) _IOP14 |= (1<<2);
	if(keys&INPUT_LEFT) _IOP14 |= (1<<1);
	if(keys&INPUT_RIGHT) _IOP14 |= (1<<0);	
	if(keys&INPUT_A) _IOP15 |= (1<<3);
#endif
}
//-----------------------------------------
//
//-----------------------------------------
uint8_t joyPad(void){
uint8_t buttons = readJoyPad();

	if (IOP1 & IOP15) {		
		buttons &= 0x0f; 
		buttons |= IOP14;
	}
	else {
		buttons >>= 4;
		buttons |= IOP15;
	}	

	IOP1 = (~buttons);
	return IOP1;
}
