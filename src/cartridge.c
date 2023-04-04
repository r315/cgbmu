

#include <stdio.h>
#include <stdint.h>
#include <cgbmu.h>
#include "cartridge.h"
#include "dmgcpu.h"
#include "board.h"

static uint8_t mbc1_bank;
static uint8_t mbc1_ram_en;
static uint8_t mbc1_mode;
static uint8_t mbc1_ram[CARTRIDGE_RAM_SIZE]; // 8kB
static uint8_t(*cRead)(uint16_t address);
static void(*cWrite)(uint16_t address, uint8_t data);

static uint8_t mbc1Read(uint16_t address) {
	switch (address >> 14)
	{
	case 0:  // 0000-3FFF Fixed Rom bank 0
		return rom0[address];

	case 1:  // 4000-7FFF loadable rom banks		
		return rombank[address & 0x3FFF];

	case 2:  // A000-BFFF banking Ram
		if (mbc1_ram_en == MCB1_RAM_EN)
			return mbc1_ram[address & (CARTRIDGE_RAM_SIZE - 1)];
	}
	return 0xFF;
}
extern uint8_t dbg_state;
uint8_t cnt = 0;
static void mbc1Write(uint16_t address, uint8_t data) {
	switch (address >> 12)
	{
	case 0: // 0000-1FFF Ram Write enable
	case 1:
		mbc1_ram_en = data;
		break;

	case 2:	// 2000-3FFF Rom bank select area 
	case 3:
		if (data != mbc1_bank){		
			mbc1_bank = (data == 0) ? 1 :data;
			rombank = rom0 + (mbc1_bank << 14);
		}
		break;

	case 4: // 4000-5FFF Ram/Rom bank select area
	case 5:
		if (mbc1_mode == 0) {
			rombank = rom0 + (mbc1_bank << 14) + (data << 20);
		}// TODO: implement ram banking fo 4Mbit/32k mode
		break;

	case 6: // 6000-7FFF Memory mode select area
	case 7:
		mbc1_mode = data & 1;
		break;

	case 0xA: // A000-BFFF Ram
	case 0xB:
		if(mbc1_ram_en == MCB1_RAM_EN)
			mbc1_ram[address & (CARTRIDGE_RAM_SIZE - 1)] = data;
		break;
	}
}

//----------------------------------------------------
//
//----------------------------------------------------
void cartridgeInit(const uint8_t *rom) {
	mbc1_bank = 1;
	rom0 = (uint8_t *)rom;
	rombank = (uint8_t *)rom + CARTRIDGE_ROM_SIZE;

	switch (rom0[CARTRIDGE_TYPE_OFFSET]) {
	case CARTRIDGE_ROM:
	case CARTRIDGE_MBC1:
	case CARTRIDGE_MBC1_RAM:
	case CARTRIDGE_MBC1_RAM_BAT:
		cRead = mbc1Read;
		cWrite =mbc1Write;
		mbc1_ram_en = 0;
		mbc1_mode = 0;  // Default 16Mbit/8k
		break;

	default:  // crashes
		cRead = NULL;
		cWrite = NULL;
	}

}

uint8_t cartridgeRead(uint16_t address) { return cRead(address); }
void cartridgeWrite(uint16_t address, uint8_t data) { cWrite(address, data); }