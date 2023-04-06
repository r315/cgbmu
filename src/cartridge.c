

#include <stdio.h>
#include <stdint.h>
#include <cgbmu.h>
#include "cartridge.h"
#include "dmgcpu.h"

typedef struct mbc1_s{
	uint8_t bank;
	uint8_t ram_en;
	uint8_t mode;
	uint8_t ram[CARTRIDGE_RAM_SIZE]; // 8kB
}mbc1_t;

static mbc1_t _mbc1;

static uint8_t mbc1Read(cpu_t *cpu, uint16_t address) {
	mbc1_t *mbc1 = (mbc1_t *)cpu->cartridge_data;

	switch (address >> 14)
	{
	case 0:  // 0000-3FFF Fixed Rom bank 0
		return cpu->rom0[address];

	case 1:  // 4000-7FFF loadable rom banks		
		return cpu->rombank[address & 0x3FFF];

	case 2:  // A000-BFFF banking Ram
		if (mbc1->ram_en == MCB1_RAM_EN)
			return mbc1->ram[address & (CARTRIDGE_RAM_SIZE - 1)];
	}
	return 0xFF;
}

static void mbc1Write(cpu_t *cpu, uint16_t address, uint8_t data) {
	mbc1_t *mbc1 = (mbc1_t *)cpu->cartridge_data;

	switch (address >> 12)
	{
	case 0: // 0000-1FFF Ram Write enable
	case 1:
		mbc1->ram_en = data;
		break;

	case 2:	// 2000-3FFF Rom bank select area 
	case 3:
		if (data != mbc1->bank){		
			mbc1->bank = (data == 0) ? 1 : data;
			cpu->rombank = cpu->rom0 + (mbc1->bank << 14);
		}
		break;

	case 4: // 4000-5FFF Ram/Rom bank select area
	case 5:
		if (mbc1->mode == 0) {
			cpu->rombank = cpu->rom0 + (mbc1->bank << 14) + (data << 20);
		}// TODO: implement ram banking fo 4Mbit/32k mode
		break;

	case 6: // 6000-7FFF Memory mode select area
	case 7:
		mbc1->mode = data & 1;
		break;

	case 0xA: // A000-BFFF Ram
	case 0xB:
		if(mbc1->ram_en == MCB1_RAM_EN)
			mbc1->ram[address & (CARTRIDGE_RAM_SIZE - 1)] = data;
		break;
	}
}

//----------------------------------------------------
//
//----------------------------------------------------
void cartridgeInit(cpu_t *cpu, const uint8_t *rom) {
	mbc1_t *mbc1;
	cpu->rom0 = (uint8_t *)rom;
	cpu->rombank = (uint8_t *)rom + CARTRIDGE_ROM_SIZE;

	switch (cpu->rom0[CARTRIDGE_TYPE_OFFSET]) {
	case CARTRIDGE_ROM:
	case CARTRIDGE_MBC1:
	case CARTRIDGE_MBC1_RAM:
	case CARTRIDGE_MBC1_RAM_BAT:
		mbc1 = &_mbc1;
		//mbc1 = (mbc1_t *)malloc(sizeof(mbc1_t)); // use for multiple cpu instances
		cpu->cartridge_data = mbc1;
		cpu->cartridgeRead = mbc1Read;
		cpu->cartridgeWrite = mbc1Write;
		mbc1->ram_en = 0;
		mbc1->mode = 0;  // Default 16Mbit/8k
		mbc1->bank = 1;
		break;

	default:  // crash and burn
		cpu->cartridgeRead = NULL;
		cpu->cartridgeWrite = NULL;
	}
}