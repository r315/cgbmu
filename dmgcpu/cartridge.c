

#include <stdio.h>
#include <stdint.h>
#include <cgbmu.h>
#include "cartridge.h"
#include "debug.h"

uint8_t bankSelect;
uint8_t *ROM0;
uint8_t *ROMBANK;

/***************************************************
// MBC1
***************************************************/
uint8_t cartridgeRead(uint16_t address)
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
void cartridgeWrite(uint16_t address, uint8_t data)
{	
	switch(address >> 12)
	{
		case 2:
		case 3: // MBC1 bank select area 2000-3FFF
			if(data != bankselect)
			{
				loadRombank(data);
			}
			break;

		case 0xA:
		case 0xB:			
		default: break;
	}
}
/***************************************************
// MBC2
***************************************************/


