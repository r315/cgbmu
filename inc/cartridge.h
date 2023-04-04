#ifndef _cartridge_h_
#define _cartridge_h_

#include <stdint.h>
#include "dmgcpu.h"

#define CARTRIDGE_ROM_SIZE          0x4000  // 16kB ROM (0x0000 - 0x3FFF)
#define CARTRIDGE_ROM_BANK_SIZE     0x4000  // 16kB switchable ROM bank
#define CARTRIDGE_RAM_SIZE          0x2000  // 8kB RAM

#define CARTRIDGE_TITLE_OFFSET      0x134
#define CARTRIDGE_TYPE_OFFSET       0x147
#define CARTRIDGE_SIZE_OFFSET       0x148

#define CARTRIDGE_ROM			0
#define CARTRIDGE_MBC1			1
#define CARTRIDGE_MBC1_RAM		2
#define CARTRIDGE_MBC1_RAM_BAT	3

#define MCB1_RAM_EN				0x0A

void cartridgeInit(cpu_t *cpu, const uint8_t *ptr);
#endif
