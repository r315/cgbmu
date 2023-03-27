/**
* @file		button.h
* @brief	Contains API header for reading buttons.
*     		
* @version	1.0
* @date		3 Nov. 2016
* @author	Hugo Reis
**********************************************************************/

#ifndef _libbutton_h_
#define _libbutton_h_


#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct{
	uint32_t cur;
	uint32_t last;
	uint32_t counter;
	uint32_t events;
    uint32_t htime;
}BUTTON_Controller;

#define BUTTON_DEFAULT_HOLD_TIME 2000   //2 seconds

enum Benvent{
    BUTTON_EMPTY = 0,
    BUTTON_PRESSED,
    BUTTON_TIMING,
    BUTTON_HOLD,
    BUTTON_RELEASED	
};




#if defined(__EMU__)
#define BUTTON_LEFT  	(1<<0)
#define BUTTON_RIGHT 	(1<<1)
#define BUTTON_CENTER	(1<<2)
#define BUTTON_UP    	(1<<3)
#define BUTTON_DOWN  	(1<<4)
#define BUTTON_A        (1<<5)		  
#define LOOP BUTTON_GetEvents() != 256 //SDL_QUIT
#else
#define LOOP 1
#endif


/**
* @brief Faz a iniciação do sistema para permitir o acesso aos botões
* @param ht : mask bitmap com os bits correspondentes aos pinos ligados nos botões
**/
void BUTTON_Init(int ht);

/**
* @brief Devolve o estado dos botões, pressionado, largado, mantido ou em contagem. 
* Não é bloqueante.
**/
int BUTTON_Read(void);

/**
* @brief Devolve o código (bitmap) do botão pressionado. É bloqueante.
*
**/
int BUTTON_Get(void);

/**
* @brief Devolve o ultimo evento registado: 
* 		 pressionado (transição),
*        libertado (transição),
*        repetição (mantido pressionado)
**/
int BUTTON_GetEvents(void);

/**
 * @brief Devolve o codigo (bitmap) da tecla corrente
 *        BUTTON_EMPTY se nenhuma tecla pressionada
 **/
int BUTTON_GetValue(void); 

/**
* @brief Efectua uma espera pelo evento dado no parametro*        
**/
void BUTTON_WaitEvent(int event);

/**
* @brief configura o tempo minimo para que uma tecla 
*        seja considerada como mantida pressionada 
**/
void BUTTON_SetHoldTime(int t);

/**
* @brief verifica se o botao passado no parametro foi premido
*        
**/
char BUTTON_Pressed(int button);



#ifdef __cplusplus
}
#endif

#endif

