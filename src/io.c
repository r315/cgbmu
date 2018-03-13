
#include "dmgcpu.h"
#include "io.h"

#if defined(__EMU__)
	#ifdef WIN32
		#include <SDL.h>
	#else
		#include <SDL2/SDL.h>
	#endif
#elif defined(__BB__)
	#include <blueboard.h>
#endif
//-----------------------------------------
//
//-----------------------------------------
uint8_t readJoyPad(void)
{
#if defined(__EMU__)
SDL_Event ev;
const Uint8 *keys;
static uint8_t button = 0;

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
  
	return button;    
#elif defined(__BB__)
int	keys = LPC_GPIO1->FIOPIN & KEYSMASK;
static uint8_t button = 0;	

	button |= (keys&INPUT_DOWN) ? J_DOWN : 0;
	button |= (keys&INPUT_UP)  ? J_UP : 0;
	button |= (keys&INPUT_LEFT) ? J_LEFT : 0;
	button |= (keys&INPUT_RIGHT) ? J_RIGHT : 0;		
	button |= (keys&INPUT_A) ? J_A : 0;
	
	return button;	
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
