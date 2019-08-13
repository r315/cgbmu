#ifndef _cartridge_h_
#define _cartridge_h_

#include <stdint.h>

#define ROM_SIZE 0x4000		// Rom bank #0 0x0000 - 0x3FFF
#define RAM_SIZE 0x2000     // Internal

#define CARTRIDGE_SIZE	0x8000
//#define MBC1_SIZE		

#define CARTRIDGE_TYPE_OFFSET		0x147
#define CARTRIDGE_ROM_SIZE_OFFSET	0x148

#define CARTRIDGE_ROM			0
#define CARTRIDGE_MBC1			1
#define CARTRIDGE_MBC1_RAM		2

uint8_t cartridgeRead(uint16_t address);
void cartridgeWrite(uint16_t address, uint8_t data);
void cartridgeInit(uint8_t *ptr);
#endif
