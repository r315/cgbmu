/**
* @file		button.c
* @brief	Contains API source code for reading buttons.
*     		
* @version	1.0
* @date		3 Nov. 2016
* @author	Hugo Reis
**********************************************************************/
#include "board.h"
#include "libbutton.h"

static BUTTON_Controller __button;

/**
 * @brief Initialise internal state variables
 * 
 * @param ht : time in ms before HOLD state
 */
void BUTTON_Init(int ht){    
    __button.cur  = BUTTON_EMPTY;
    __button.last = BUTTON_EMPTY;
    __button.events = BUTTON_EMPTY;
    __button.htime = ht;
}

int BUTTON_Read(void){
uint32_t cur;

    cur = BUTTON_HW_READ;

    switch(__button.events){

        case BUTTON_EMPTY:
            if(cur == BUTTON_EMPTY)
                break;            
            __button.cur = cur;
            __button.events = BUTTON_PRESSED;
            break;

        case BUTTON_PRESSED:
            if(cur == BUTTON_EMPTY){
                __button.events = BUTTON_RELEASED;
                break;
            }
            if(cur == __button.cur){             // same key still pressed
                __button.events = BUTTON_TIMING; // start timer
                __button.counter = GetTick();
                break;
            }
            __button.cur = cur; // another key was pressed 
            break;              // TODO: optionally implement if one key is released

        case BUTTON_TIMING:
            if(cur == BUTTON_EMPTY){
                __button.events = BUTTON_RELEASED;
                break;
            }            
            if(cur == __button.cur){
                if(ElapsedTicks(__button.counter) > __button.htime){
                    __button.events = BUTTON_HOLD;
                }   
                break;
            }
            __button.cur = cur; // another key was pressed 
            __button.events = BUTTON_PRESSED;
            break;

        case BUTTON_HOLD:
            if(cur == BUTTON_EMPTY){
                __button.events = BUTTON_RELEASED;
                break;
            }
            if(cur == __button.cur)
                break;            
            __button.cur = cur; // another key was pressed 
            __button.events = BUTTON_PRESSED;            
            break;
            
        case BUTTON_RELEASED:
            __button.last= __button.cur;
            __button.cur = BUTTON_EMPTY;
            __button.events = BUTTON_EMPTY;
            break;
            
        default: break;
    }
    return __button.events;
}

void BUTTON_WaitEvent(int event){
 do{
     BUTTON_Read();
 }while(__button.events != (uint32_t)event);    
}

int BUTTON_Get(void){
	BUTTON_WaitEvent(BUTTON_PRESSED);
	return __button.cur; 
}

int BUTTON_GetEvents(void){	
	return __button.events;
}

int BUTTON_GetValue(void){
	return __button.cur;
}

void BUTTON_SetHoldTime(int t){
    __button.htime = t;
}

char BUTTON_Pressed(int button){
	return (__button.cur & button) != 0 ? 1 : 0;
}



