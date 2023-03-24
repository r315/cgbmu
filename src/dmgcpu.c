/*
Resorces:
http://bgb.bircd.org/pandocs.htm#videodisplay
http://fms.komkon.org/EMUL8/HOWTO.html
http://meatfighter.com/gameboy

*/
#include <stdint.h>
#include "board.h"
#include "dmgcpu.h"
#include "video.h"
#include "cartridge.h"
#include "cgbmu.h"
#include "decoder.h"
#include "instrs.h"

// IO Registers
uint8_t IOP1;   // 0xFF00 P1
uint8_t IODIV;  // 0xFF04 timer divider
uint8_t IOTIMA; // 0xFF05 timer counter
uint8_t IOTMA;  // 0xFF06 timer modulo
uint8_t IOTAC;  // 0xFF07 timer control
uint8_t IOIF;   // 0xFF0F interrupt flag
uint8_t IOLCDC; // 0xFF40 lcd control
uint8_t IOSTAT; // 0xFF41 lcd status
uint8_t IOSCY;  // 0xFF42 scroll y
uint8_t IOSCX;  // 0xFF43 scroll x
uint8_t IOLY;   // 0xFF0F LY
uint8_t IOLYC;  // 0xFF0F LY Compare
uint8_t IODMA;  // 0xFF0F
uint8_t IOBGP;  // 0xFF0F Background palette
uint8_t IOOBP0; // 0xFF0F object palette 0
uint8_t IOOBP1; // 0xFF0F object palette 1
uint8_t IOWY;   // 0xFF0F window Y
uint8_t IOWX;   // 0xFF0F window x
uint8_t IOIE;   // 0xFFFF Interrupt Enable

uint8_t vram[0x2000];                    // 0x8000-0x9FFF
uint8_t iram[0x2000];                    // 0xC000-0xBFFF
uint8_t oam[sizeof(Object)*MAX_OBJECTS]; // 0xFE00-0xFEBF
uint8_t hram[128];                       // 0xFF80-0xFFFE
uint8_t *rom0;                           // 0x0000-0x3FFF
uint8_t *rombank;                        // 0x4000-0x7FFF

uint8_t  instr_cycles;
uint32_t machine_cycles = 0;
uint16_t timer_prescaler;
uint8_t halted, stopped;
uint8_t IME;  	// Reset by DI and set by EI instructions

Regs regs;


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

void setTimerPrescaler(void);

//-----------------------------------------
//
//-----------------------------------------

void jumpVector(uint16_t vector)
{
	IME = 0;
	halted = 0;
    PUSH(REG_PC);
    REG_PC = vector;  
}
//-----------------------------------------
//
//-----------------------------------------

void interrupts(void)
{
uint8_t irq;
	
	if(!IME){
		return;
	}

	irq = IOIE & IOIF;
	
	if(irq & V_BLANK_IF){
		IOIF &= ~(V_BLANK_IF);
		jumpVector(0x0040);
	}
	
	if(irq & LCDC_IF){ 	
		IOIF &= ~(LCDC_IF);
		jumpVector(0x0048);
	}	
		
	if(irq & TIMER_IF){		
		IOIF &= ~TIMER_IF;
		jumpVector(0x0050);
	}	
		
	if(irq & SERIAL_IF){ 		
		IOIF &= ~(SERIAL_IF);
		jumpVector(0x0058);
	}	
		
	if(irq & JOYPAD_IF){
		IOIF &= ~(JOYPAD_IF);
		jumpVector(0x0060);
	}			
}
//-----------------------------------------
//
//-----------------------------------------

void setTimerPrescaler(void){
	switch(IOTAC & 3)	{
		case 0: // 4096Hz		
			timer_prescaler = 256 * CLOCK_CYCLE;
			break;
		case 1: // 262144Hz
			timer_prescaler = 4 * CLOCK_CYCLE;
			break;
		case 2: // 65536Hz
			timer_prescaler = 16 * CLOCK_CYCLE;
			break;
		case 3: // 16384Hz
			timer_prescaler = 64 * CLOCK_CYCLE;
			break;		
	}
}
//-----------------------------------------
//
//-----------------------------------------

void timer(void)
{
	static uint16_t timer_cycles = 0;
	static uint16_t div_cycles = 0;

	div_cycles += instr_cycles;

	while (div_cycles >= 64 * CLOCK_CYCLE) {
		IODIV++;
		div_cycles -= 64 * CLOCK_CYCLE;
	}

	if(!(IOTAC & TIMER_STOP)) 
		return;
	
	timer_cycles += instr_cycles;

	while (timer_cycles >= timer_prescaler) {
		IOTIMA++;
		if(!IOTIMA){			
			IOTIMA = IOTMA;		// on overflow TIMA is reloaded with TMA
			IOIF |= TIMER_IF;	// and TIMER_IF is set
		}
		timer_cycles -= timer_prescaler;
	}
}
//-----------------------------------------
//
//-----------------------------------------

void dma(uint16_t src){
uint8_t i, *pdst;
	pdst = (uint8_t*)&oam[0];
    for(i = 0; i < DMA_SIZE; i++, pdst++)
		*pdst = memoryRead(src++);           
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

uint8_t joyPad(void) {
	uint8_t buttons;

	buttons = ~readButtons();   // read buttons, 0 means button pressed

	if (!(IOP1 & IOP14)) {		// Check which row was selected
		buttons &= 0x0F;		// joy pad
	}
	else {
		buttons >>= 4;          // select, start, A, B
	}

	IOP1 = (IOP1 & 0xF0) | buttons;
	
	return IOP1;
}

//-----------------------------------------
//
//-----------------------------------------

uint8_t memoryRead(uint16_t address)
{	
	switch(address>>12)
	{
		case 0:
		case 1:
		case 2:
		case 3:
			return rom0[address]; // 16k Rom bank #0  0x0000 - 0x3FFF 
			
		case 4:
		case 5:
		case 6:
		case 7:
			return rombank[address & 0x3FFF]; //16kB switchable ROM bank 4000 - 7FFF
			
		case 8:
		case 9:
			return vram[address & 0x1FFF];   // 8k 0x8000 - 0x9FFF
			
		case 0x0A:
		case 0x0B:
			return cartridgeRead(address);  // 8kB switchable RAM bank 0xA000 - 0xBFFF
		
		case 0x0C:
		case 0x0D:
			return iram[address & 0x1FFF];  // 8kB internal ram 0xC000 - 0xDFFF
			
	//	default: break;
	}	
		
	if((address > 0xDFFF) && (address < 0xFE00))
		return iram[address & 0x1FFF]; // ram fold 8k
	
	if((address > 0xFDFF) && (address < 0xFEA0))
		return oam[address & 0xFF]; // atribute ram 160 bytes	
		
	if((address > 0xFF7F) && (address < 0xFFFF))		
		return hram[address & 0x7f];  // high ram 127 bytes				

	switch(address)
	{
		case 0xFF00: return joyPad();
		case 0xFF04: return IODIV;
		case 0xFF05: return IOTIMA;
		case 0xFF06: return IOTMA;
		case 0xFF07: return IOTAC;
		case 0xFF0F: return IOIF;
	    case 0xFF40: return IOLCDC;
        case 0xFF41: return IOSTAT;
        case 0xFF42: return IOSCY;
        case 0xFF43: return IOSCX;
        case 0xFF44: return IOLY;
        case 0xFF45: return IOLYC;
		case 0xFF46: return IODMA;
        case 0xFF47: return IOBGP;
        case 0xFF48: return IOOBP0;
        case 0xFF49: return IOOBP1;        
        case 0xFF4A: return IOWY;
        case 0xFF4B: return IOWX;
        case 0xFFFF: return IOIE;
      }
	  return 0xFF;
}
//-----------------------------------------
//
//-----------------------------------------

void memoryWrite(uint16_t address, uint8_t data)
{	
	switch(address>>12)
	{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
			cartridgeWrite(address,data);
			return;
			
		case 8:
		case 9:
			vram[address & 0x1FFF] = data; // 8K
			return;
			
		case 0x0A:
		case 0x0B:
			cartridgeWrite(address,data);
			return; 
		
		case 0x0C:
		case 0x0D:
			iram[address & 0x1FFF] = data; // 8k
			return; 
			
		default: break;
	}
		
	if((address > 0xDFFF) && (address < 0xFE00)){
		iram[address - 0xE000]=data; // ram fold
		return;
	}
	
	if((address > 0xFDFF) && (address < 0xFEA0)){
		oam[address & 0xFF ] = data; // atribute ram
		return;
	}
	
	if((address > 0xFF7F) && (address < 0xFFFF)){
		hram[address & 0x7F] = data;
		return ;
	}
		
	switch(address)
	{
		case 0xFF00: IOP1 = data; return;
		case 0xFF04: IODIV = data; return;
		case 0xFF05: IOTIMA = data; return;
		case 0xFF06: IOTMA = data; return;
		case 0xFF07: IOTAC = data; 
					 setTimerPrescaler(); 
		             return;
		case 0xFF0F: IOIF = data; return;
	    case 0xFF40: IOLCDC = data;
					 if(!(data&0x80)){ IOLY= 0; IOSTAT = 0x00;} // reset LY, mode0
			 		 return;
        case 0xFF41: IOSTAT = data; return;
        case 0xFF42: IOSCY = data;	return;
        case 0xFF43: IOSCX = data; 	return;
        case 0xFF44: IOLY = data; return;
        case 0xFF45: IOLYC = data; return;
		case 0xFF46: dma(data<<8); return;			
        case 0xFF47: IOBGP = data; return;
        case 0xFF48: IOOBP0 = data; return;
        case 0xFF49: IOOBP1 = data; return;        
        case 0xFF4A: IOWY = data; return;
        case 0xFF4B: IOWX = data; return;
        case 0xFFFF: IOIE = data; return;
    }
}
//-----------------------------------------
//
//-----------------------------------------

uint16_t memoryRead16(uint16_t address)
{	
	return memoryRead(address) | (memoryRead(address+1)<<8);
}
//-----------------------------------------
//
//-----------------------------------------

void memoryWrite16(uint16_t address, uint16_t data)
{
	memoryWrite(address++, (uint8_t)data);
	memoryWrite(address, (uint8_t)(data>>8));	
}
//-----------------------------------------
//
//-----------------------------------------
void initCpu(void)
{
	REG_A = 0x01;
    REG_B = 0x00;
    REG_C = 0x13;
    REG_D = 0x00;
    REG_E = 0xD8;
    REG_H = 0x01;
    REG_L = 0x4D;
    
    REG_SP = 0xFFFE;
    REG_PC = 0x0100;    
	REG_F =  FZ | FH | FC;
	
    IOTIMA = 0x00;
    IOTMA  = 0x00;
    IOTAC  = 0x00;
    IOLCDC = 0x91;
    IOSCY  = 0x00;
    IOSCX  = 0x00;
    IOLYC  = 0x00;
    IOBGP  = 0xFC;
    IOOBP0 = 0xFF;
    IOOBP1 = 0xFF;
    IOWY   = 0x00;
    IOWX   = 0x00;
    IOIE   = 0x00;    
	IOSTAT = 0x81;
	IOLY   = 0x94; 	 
	IODIV  = 0xAB;
	halted = 0;
}

void bootCpu(void)
{
	REG_PC = 0;

	cartridgeInit(boot_rom);

	do{
		decode();
		timer();
		video();
		interrupts();
	}while(REG_PC != 0x100);
}