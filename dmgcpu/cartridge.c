

#include <stdio.h>
#include <cgbmu.h>
#include "cartridge.h"
#include "debug.h"

unsigned char bankSelect;
unsigned char *ROM0;
unsigned char *ROMBANK;

/***************************************************
// MBC1
***************************************************/
unsigned char cartridgeRead(unsigned short address)
{
	switch(address >> 14)
	{
		case 0:  // 0000-3FFF   Rom bank 0
			return ROM0[address]; 
			
		case 1:  // 4000-7FFF Nota romBank = 0/1 deve de retornar ROM1
			address &= 0x3FFF;			
			return ROMBANK[address]; 
			
		case 2:  // 8000-C000 banking Ram not implemented
		case 3:
			return 0xFF;//ROMBANK[address - 0x4000];
	}
	return 0xFF;
}
//----------------------------------------------------
//
//----------------------------------------------------
void cartridgeWrite(unsigned short address, char data)
{	
	switch(address >> 13)
	{
		case 1: // 2000-3FFF
			if(data != bankselect)
			{
				loadRombank(data);
			}
			break;
			
		default: break;
	}
}
/***************************************************
// MBC2
***************************************************/


