/*
Resorces:
http://bgb.bircd.org/pandocs.htm#videodisplay
http://fms.komkon.org/EMUL8/HOWTO.html
http://meatfighter.com/gameboy

*/
#include "dmgcpu.h"
#include "video.h"
#include "cartridge.h"
#include "io.h"

#if defined(WIN32)
#pragma warning(disable:4996)
#endif

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
uint8_t oam[sizeof(Sprite)*MAX_SPRITES]; // 0xFE00-0xFEBF
uint8_t hram[128];                       // 0xFF80-0xFFFE

uint8_t  cycles;
uint32_t machine_cycles = 0;
uint16_t timer_cycles = 0;
uint16_t timer_prescaler;
uint8_t halted, stopped;
uint8_t IME;  

Regs regs;

void push(uint16_t v);
void decode(void);
void setTimerPrescaler(void);

//-----------------------------------------
//
//-----------------------------------------
void jumpVector(uint16_t vector)
{
	IME = 0;
	halted = 0;
    push(REG_PC);
    REG_PC = vector;  
}
//-----------------------------------------
//
//-----------------------------------------
void interrupts(void)
{
uint8_t irq;
	
	if(!IME) 
	{
		if(IOIF & 0x1F)
			halted = 0;
		return;
	}

	irq = IOIE & IOIF;
	
	if(irq & 1){ // V-Blank		
		IOIF &= ~(1<<0);
		jumpVector(0x0040);
	}
	
	if(irq & 2){ // LCDC		
		IOIF &= ~(1<<1);
		jumpVector(0x0048);
	}	
		
	if(irq & TIMER_IE){		
		IOIF &= ~TIMER_IF;
		jumpVector(0x0050);
	}	
		
	if(irq & 8){ // serial		
		IOIF &= ~(1<<3);
		jumpVector(0x0058);
	}	
		
	if(irq & 16){ // buttons		
		IOIF &= ~(1<<4);
		jumpVector(0x0060);
	}			
}

void setTimerPrescaler(void){
	switch(IOTAC & 3)	{
		case 0: // 4096Hz		
			timer_prescaler = 256;
			break;
		case 1: // 262144Hz
			timer_prescaler = 4;
			break;
		case 2: // 65536Hz
			timer_prescaler = 16;
			break;
		case 3: // 16384Hz
			timer_prescaler = 64;
			break;		
	}
}
//-----------------------------------------
//
//-----------------------------------------
void timer(void)
{
	if(!(IOTAC & TIMER_STOP)) return; // timer stopped
	
	timer_cycles += GET_CYCLE();	
	
	while(timer_cycles >= timer_prescaler)	{
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
			return ROM0[address]; // 0 - 3FFF
			
		case 4:
		case 5:
		case 6:
		case 7:
			return ROMBANK[address & 0x3FFF]; //4000 - 7FFF
			
		case 8:
		case 9:
			return vram[address & 0x1FFF];   // 8k
			
		case 0x0A:
		case 0x0B:
			return 0xFF;// cartridgeRead(address); // 8K
		
		case 0x0C:
		case 0x0D:
			return iram[address & 0x1FFF]; // 8k
			
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
		case 0xFF04: return (uint8_t)(machine_cycles>>6);  // = Fosc/256 ~ 16384Hz
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
