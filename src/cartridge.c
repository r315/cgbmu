

#include <stdio.h>
#include <stdint.h>
#include <cgbmu.h>
#include "cartridge.h"
#include "dmgcpu.h"
#include "debug.h"

static uint8_t bankselect;


void cartridgeInit(const uint8_t *rom) {
	bankselect = 1;
	rom0 = (uint8_t *)rom;
	rombank = (uint8_t *)rom + ROM_SIZE;
}
/***************************************************
// MBC1
***************************************************/
FAST_CODE
uint8_t cartridgeRead(uint16_t address)
{
	switch(address >> 14)
	{		
		case 0:  // 0000-3FFF   fixed Rom bank 0
			return rom0[address]; 

		case 1:  // 4000-7FFF   loadable rom banks
			address &= 0x3FFF;			
			return rombank[address]; 
			
		case 2:  // 8000-C000 banking Ram not implemented
		case 3:
			return 0xFF;//ROMBANK[address - 0x4000];
	}
	return 0xFF;
}
//----------------------------------------------------
//
//----------------------------------------------------
FAST_CODE
void cartridgeWrite(uint16_t address, uint8_t data)
{	
	switch(address >> 12)
	{
		case 2:
		case 3: // MBC1 bank select area 2000-3FFF
			if(data != bankselect)
			{
				bankselect = data;
				rombank = rom0 + (bankselect << 14);
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


