/*
Resorces:
http://bgb.bircd.org/pandocs.htm#videodisplay
http://fms.komkon.org/EMUL8/HOWTO.html
http://meatfighter.com/gameboy

*/
#include <stdint.h>
#include "dmgcpu.h"
#include "video.h"
#include "cartridge.h"
#include "cgbmu.h"
#include "decoder.h"
#include "instrs.h"


static const uint8_t boot_rom [] = {
	0x31,0xFE,0xFF,0xAF,0x21,0xFF,0x9F,0x32,0xCB,0x7C,0x20,0xFB,0x21,0x26,0xFF,0x0E,
	0x11,0x3E,0x80,0x32,0xE2,0x0C,0x3E,0xF3,0xE2,0x32,0x3E,0x77,0x77,0x3E,0xFC,0xE0,
	0x47,0x11,0x04,0x01,0x21,0x10,0x80,0x1A,0xCD,0x95,0x00,0xCD,0x96,0x00,0x13,0x7B,
	0xFE,0x34,0x20,0xF3,0x11,0xD8,0x00,0x06,0x08,0x1A,0x13,0x22,0x23,0x05,0x20,0xF9,
	0x3E,0x19,0xEA,0x10,0x99,0x21,0x2F,0x99,0x0E,0x0C,0x3D,0x28,0x08,0x32,0x0D,0x20,
	0xF9,0x2E,0x0F,0x18,0xF3,0x67,0x3E,0x64,0x57,0xE0,0x42,0x3E,0x91,0xE0,0x40,0x04,
	0x1E,0x02,0x0E,0x0C,0xF0,0x44,0xFE,0x90,0x20,0xFA,0x0D,0x20,0xF7,0x1D,0x20,0xF2,
	0x0E,0x13,0x24,0x7C,0x1E,0x83,0xFE,0x62,0x28,0x06,0x1E,0xC1,0xFE,0x64,0x20,0x06,
	0x7B,0xE2,0x0C,0x3E,0x87,0xE2,0xF0,0x42,0x90,0xE0,0x42,0x15,0x20,0xD2,0x05,0x20,
	0x4F,0x16,0x20,0x18,0xCB,0x4F,0x06,0x04,0xC5,0xCB,0x11,0x17,0xC1,0xCB,0x11,0x17,
	0x05,0x20,0xF5,0x22,0x23,0x22,0x23,0xC9,0xCE,0xED,0x66,0x66,0xCC,0x0D,0x00,0x0B,
	0x03,0x73,0x00,0x83,0x00,0x0C,0x00,0x0D,0x00,0x08,0x11,0x1F,0x88,0x89,0x00,0x0E,
	0xDC,0xCC,0x6E,0xE6,0xDD,0xDD,0xD9,0x99,0xBB,0xBB,0x67,0x63,0x6E,0x0E,0xEC,0xCC,
	0xDD,0xDC,0x99,0x9F,0xBB,0xB9,0x33,0x3E,0x3C,0x42,0xB9,0xA5,0xB9,0xA5,0x42,0x3C,
	0x21,0x04,0x01,0x11,0xA8,0x00,0x1A,0x13,0xBE,0x20,0xFE,0x23,0x7D,0xFE,0x34,0x20,
	0xF5,0x06,0x19,0x78,0x86,0x23,0x05,0x20,0xFB,0x86,0x20,0xFE,0x3E,0x01,0xE0,0x50,
	0x00,0xC3,0x50,0x01,0xCE,0xED,0x66,0x66,0xCC,0x0D,0x00,0x0B,0x03,0x73,0x00,0x83,
	0x00,0x0C,0x00,0x0D,0x00,0x08,0x11,0x1F,0x88,0x89,0x00,0x0E,0xDC,0xCC,0x6E,0xE6,
	0xDD,0xDD,0xD9,0x99,0xBB,0xBB,0x67,0x63,0x6E,0x0E,0xEC,0xCC,0xDD,0xDC,0x99,0x9F,
	0xBB,0xB9,0x33,0x3E,0x54,0x45,0x54,0x52,0x49,0x53,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x0B,0x89,0xB5,
	0xC3,0x8B,0x02,0xCD,0x2B,0x2A,0xF0,0x41,0xE6,0x03,0x20,0xFA,0x46,0xF0,0x41,0xE6,
	0x03,0x20,0xFA,0x7E,0xA0,0xC9,0x7B,0x86,0x27,0x22,0x7A,0x8E,0x27,0x22,0x3E,0x00,
	0x8E,0x27,0x77,0x3E,0x01,0xE0,0xE0,0xD0,0x3E,0x99,0x32,0x32,0x77,0xC9,0xF5,0xE5,
};

//-----------------------------------------
//
//-----------------------------------------
void setInt(cpu_t *cpu, uint8_t irq) {
	cpu->IOIF |= irq;
	if (cpu->IOIE & irq) {
		cpu->halt = 0;
	}
}

void interrupts(cpu_t *cpu)
{
	uint8_t irq;
	
	if(!cpu->IME){
		return;
	}

	irq = cpu->IOIE & cpu->IOIF;

	if (!irq) {
		return;
	}

	cpu->IME = 0;
	PUSH(cpu, cpu->PC);

	if(irq & V_BLANK_IF){ // Priority 1
		cpu->IOIF &= ~(V_BLANK_IF);
		REG_PC = 0x0040;
	}else if(irq & LCDC_IF){ // Priority 2
		cpu->IOIF &= ~(LCDC_IF);
		REG_PC = 0x0048;
	}else if(irq & TIMER_IF){ // Priority 3
		cpu->IOIF &= ~TIMER_IF;
		REG_PC = 0x0050;
	}else if(irq & SERIAL_IF){ // Priority 4
		cpu->IOIF &= ~(SERIAL_IF);
		REG_PC = 0x0058;
	}else if(irq & JOYPAD_IF){ // Priority 5
		cpu->IOIF &= ~(JOYPAD_IF);
		REG_PC = 0x0060;
	}
}
//-----------------------------------------
//
//-----------------------------------------
void writeTAC(cpu_t *cpu, uint8_t newtac){
	switch(newtac & 3)	{
		case 0: // 4096Hz		
			cpu->timer_ovf = 256 * CLOCK_CYCLE;
			break;
		case 1: // 262144Hz
			cpu->timer_ovf = 4 * CLOCK_CYCLE;
			break;
		case 2: // 65536Hz
			cpu->timer_ovf = 16 * CLOCK_CYCLE;
			break;
		case 3: // 16384Hz
			cpu->timer_ovf = 64 * CLOCK_CYCLE;
			break;		
	}

	cpu->IOTAC = newtac;
}

void timer(cpu_t *cpu)
{
	cpu->div_cycles += cpu->instr_cycles;

	while (cpu->div_cycles >= 64 * CLOCK_CYCLE) {
		cpu->IODIV++;
		cpu->div_cycles -= 64 * CLOCK_CYCLE;
	}

	if(!(cpu->IOTAC & TIMER_STOP))
		return;
	
	cpu->timer_cycles += cpu->instr_cycles;

	while (cpu->timer_cycles >= cpu->timer_ovf) {
		cpu->IOTIMA++;
		if(!cpu->IOTIMA){
			cpu->IOTIMA = cpu->IOTMA;		// on overflow TIMA is reloaded with TMA
			setInt(cpu, TIMER_IF);	// and TIMER_IF is set
		}
		cpu->timer_cycles -= cpu->timer_ovf;
	}
}

//-----------------------------------------
//
//-----------------------------------------
void serial(cpu_t *cpu)
{
	if ((cpu->IOSC & (SC_TRF | SC_CLKI)) != (SC_CLKI | SC_TRF)) {
		return;
	}

	cpu->serial_cycles += cpu->instr_cycles;

	if (cpu->serial_cycles >= SERIAL_CYCLE) {
		if (!(cpu->serial_bit--)) {
			cpu->IOSC &= ~SC_TRF;
			setInt(cpu, SERIAL_IF);
			cpu->serial_cycles = 0;
			return;
		}

		cpu->serial_cycles -= SERIAL_CYCLE;
		cpu->IOSB = (cpu->IOSB << 1) | 1;
	}
}

void writeSC(cpu_t *cpu, uint8_t new_sc) {
	new_sc |= ~(SC_TRF | SC_CLKI);

	if (new_sc & SC_TRF) {
		cpu->serial_bit = 8;
	}

	cpu->IOSC = new_sc;
}
//-----------------------------------------
//
//-----------------------------------------
void writeBGP(cpu_t *cpu, uint8_t newbgp) {
	cpu->IOBGP = newbgp;

	for (uint8_t i = 0; i < 4; i++) {
		cpu->bgpal[i] = newbgp & 3;
		newbgp >>= 2;
	}
}

void writeOBP0(cpu_t *cpu, uint8_t newbgp) {
	cpu->IOOBP0 = newbgp;

	for (uint8_t i = 0; i < 4; i++) {
		cpu->obj0pal[i] = newbgp & 3;
		newbgp >>= 2;
	}
}

void writeOBP1(cpu_t *cpu, uint8_t newbgp) {
	cpu->IOOBP1 = newbgp;

	for (uint8_t i = 0; i < 4; i++) {
		cpu->obj1pal[i] = newbgp & 3;
		newbgp >>= 2;
	}
}

void writeDMA(cpu_t *cpu, uint8_t newdma){
	uint16_t src = newdma << 8;
	uint8_t i, *pdst;
	pdst = cpu->oam;
    for(i = 0; i < DMA_SIZE; i++, pdst++)
		*pdst = memoryRead(cpu, src++);           
}

/**----------------------------------------s
*           P14           P15
*            |            |
* P10 -------+-[Right]----+-[A]
*            |            |
* P11 -------+-[Left]-----+-[B]
*            |            |
* P12 -------+-[Up]-------+-[Select]
*            |            |
* P13 -------+-[Down]-----+-[Start]
*            |            |
//----------------------------------------- */

uint8_t joyPad(cpu_t *cpu) {
	uint8_t buttons;
	uint8_t p1;

	buttons = ~readButtons();   // read buttons, 0 means button pressed
	p1 = cpu->IOP1;             // Old key presses are cleared on IOP1 write
	if (!(p1 & IOP15)) {
		buttons >>= 4;			// shift nible when P15 is active, filters dpad keys
	}
	
	buttons |= 0xF0;            // create mask
	
	cpu->IOP1 = p1 & buttons;   // Filter pressed keys
	
	return cpu->IOP1;
}

//-----------------------------------------
//
//-----------------------------------------

uint8_t memoryRead(cpu_t *cpu, uint16_t address)
{	
	switch(address>>12)
	{
		case 0: // 0000-3FFF, 16k Rom bank #0
		case 1:
		case 2:
		case 3:
			return cpu->rom0[address];
			
		case 4: // 4000-7FFF, 16kB switchable ROM bank
		case 5:
		case 6:
		case 7:
			return cpu->rombank[address & 0x3FFF];
			
		case 8: // 8000-9FFF, 8k video ram
		case 9:
			return cpu->vram[address & 0x1FFF];
			
		case 0x0A: // A000-BFFF, 8kB switchable RAM bank
		case 0x0B:
			return cpu->cartridgeRead(cpu, address);  
		
		case 0x0C: // C000-DFFF, kB internal ram
		case 0x0D:
			return cpu->iram[address & 0x1FFF];
			
		default: break;
	}	
	// E000-EDFF, 8k ram fold
	if((address > 0xDFFF) && (address < 0xFE00))
		return cpu->iram[address & 0x1FFF];
	// FE00-FE9F, 160bytes oam
	if ((address > 0xFDFF) && (address < 0xFEA0))
		return cpu->oam[address & 0xFF];
	// FF80-FFFE, 127 Bytes hram	
	if((address > 0xFF7F) && (address < 0xFFFF))		
		return cpu->hram[address & 0x7f];

	switch(address)
	{
		case 0xFF00: return joyPad(cpu);
		case 0xFF01: return cpu->IOSB;
		case 0xFF02: return cpu->IOSC;
		case 0xFF04: return cpu->IODIV;
		case 0xFF05: return cpu->IOTIMA;
		case 0xFF06: return cpu->IOTMA;
		case 0xFF07: return cpu->IOTAC;
		case 0xFF0F: return cpu->IOIF;
	    case 0xFF40: return cpu->IOLCDC;
        case 0xFF41: return cpu->IOSTAT;
        case 0xFF42: return cpu->IOSCY;
        case 0xFF43: return cpu->IOSCX;
        case 0xFF44: return cpu->IOLY;
        case 0xFF45: return cpu->IOLYC;
		case 0xFF46: return 0xFF; //IODMA;
        case 0xFF47: return cpu->IOBGP;
        case 0xFF48: return cpu->IOOBP0;
        case 0xFF49: return cpu->IOOBP1;
        case 0xFF4A: return cpu->IOWY;
        case 0xFF4B: return cpu->IOWX;
        case 0xFFFF: return cpu->IOIE;
      }
	  return 0xFF;
}
//-----------------------------------------
//
//-----------------------------------------

void memoryWrite(cpu_t *cpu, uint16_t address, uint8_t data)
{	
	switch(address>>12)
	{
		case 0: // 0000-7FFF
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
			cpu->cartridgeWrite(cpu, address, data);
			return;
			
		case 8:     // 8000-9FFF, 8k
		case 9:
			cpu->vram[address & 0x1FFF] = data;
			return;
			
		case 0x0A:	// A000-BFFF, 8k
		case 0x0B:
			cpu->cartridgeWrite(cpu, address, data);
			return; 
		
		case 0x0C:  // C000-DFFF, 8k
		case 0x0D:
			cpu->iram[address & 0x1FFF] = data;
			return; 
			
		default: break;
	}
	// E000-EDFF, 8k ram fold
	if((address > 0xDFFF) && (address < 0xFE00)){
		cpu->iram[address - 0xE000] = data;
		return;
	}
	// FE00-FE9F, 160bytes oam
	if((address > 0xFDFF) && (address < 0xFEA0)){
		cpu->oam[address & 0xFF ] = data;
		return;
	}
	// FF80-FFFE, 127 Bytes hram
	if((address > 0xFF7F) && (address < 0xFFFF)){
		cpu->hram[address & 0x7F] = data;
		return ;
	}
		
	switch(address)
	{
		case 0xFF00: cpu->IOP1 = (data | 0xCF); 	return;
		case 0xFF01: cpu->IOSB = data; return;
		case 0xFF02: writeSC(cpu, data); return;
		case 0xFF04: cpu->IODIV = 0; return;		// any write clears it
		case 0xFF05: cpu->IOTIMA = data; return;
		case 0xFF06: cpu->IOTMA = data; return;
		case 0xFF07: writeTAC(cpu, data); return;
		case 0xFF0F: cpu->IOIF = data; return;
	    case 0xFF40: writeLCDC(cpu, data); return;
        case 0xFF41: writeSTAT(cpu, data); return;
        case 0xFF42: cpu->IOSCY = data;	return;
        case 0xFF43: cpu->IOSCX = data; 	return;
        case 0xFF44: return;				// read only
        case 0xFF45: writeLYC(cpu, data); return;
		case 0xFF46: writeDMA(cpu, data); return;			
        case 0xFF47: writeBGP(cpu, data); return;
        case 0xFF48: writeOBP0(cpu, data); return;
        case 0xFF49: writeOBP1(cpu, data); return;
        case 0xFF4A: cpu->IOWY = data; return;
        case 0xFF4B: cpu->IOWX = data; return;
        case 0xFFFF: cpu->IOIE = data; return;
    }
}
//-----------------------------------------
//
//-----------------------------------------

uint16_t memoryRead16(cpu_t *cpu, uint16_t address)
{	
	return memoryRead(cpu, address) | (memoryRead(cpu, address+1) << 8);
}
//-----------------------------------------
//
//-----------------------------------------

void memoryWrite16(cpu_t *cpu, uint16_t address, uint16_t data)
{
	memoryWrite(cpu, address++, (uint8_t)data);
	memoryWrite(cpu, address, (uint8_t)(data>>8));	
}
//-----------------------------------------
//
//-----------------------------------------
void initCpu(cpu_t *cpu)
{
	REG_A = 0x01;
    REG_B = 0x00;
    REG_C = 0x13;
    REG_D = 0x00;
    REG_E = 0xD8;
    cpu->H = 0x01;
    cpu->L = 0x4D;
    
    cpu->SP = 0xFFFE;
    cpu->PC = 0x0100;    
	cpu->F =  FZ | FH | FC;
	
    cpu->IOTIMA = 0x00;
    cpu->IOTMA  = 0x00;
    cpu->IOTAC  = 0xF8;
	cpu->IODIV  = 0xAB;

    cpu->IOLCDC = 0x91;
	cpu->IOSTAT = 0x85;
    cpu->IOSCY  = 0x00;
    cpu->IOSCX  = 0x00;
	cpu->IOLY   = 0x00; 	 
    cpu->IOLYC  = 0x00;
	cpu->IODMA  = 0xFF;
	writeBGP(cpu, 0xFC);
	writeOBP0(cpu, 0xFF);
	writeOBP1(cpu, 0xFF);    
    cpu->IOWY   = 0x00;
    cpu->IOWX   = 0x00;
    
	cpu->IOIF   = 0xE1;
    cpu->IOIE   = 0x00;

	cpu->IOP1   = 0xCF;

	cpu->IOSC = 0x7E;
	cpu->IOSB = 0x00;

	cpu->halt = 0;

	cpu->div_cycles = 0;
	cpu->timer_cycles = 0;
	cpu->serial_cycles = 0;
	cpu->video_cycles = 0;

	cpu->visible_objs = cpu->_visible_objs;
	cpu->screen_line = cpu->_screen_line;
	cpu->oam = cpu->_oam;
	cpu->hram = cpu->_hram;
	cpu->vram = cpu->_vram;
	cpu->iram = cpu->_iram;

}

void bootCpu(cpu_t *cpu)
{

	cartridgeInit(cpu, boot_rom);
    initCpu(cpu);
	cpu->PC = 0;

	do{
		decode(cpu);
		timer(cpu);
		video(cpu);
		interrupts(cpu);
	}while(cpu->PC != 0x100);
}