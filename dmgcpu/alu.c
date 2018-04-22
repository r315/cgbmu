
#include "dmgcpu.h"
#include "alu.h"

//-----------------------------------------
// ADD, ADC, SUB, SBC, AND, XOR, OR, CP
// flags affected Z,0,H,C
//-----------------------------------------
void alu(uint8_t op, uint8_t opb)
{
uint16_t sum;
uint8_t ci = 0;
uint8_t opa = REG_A;
	
	switch(op)
	{				
		case 1: // ADC					
			if(PSW & FC) ci = 1;
		case 0: // ADD	
			PSW = 0 ;						
			if( ((opa & 0x0F) + (opb & 0x0f) + ci) > 0x0F) PSW |= FH;
			sum = opa + opb + ci;
			if( (sum & 0xFF00) ) PSW |= FC;				
			opa = (uint8_t)sum;
			break;
		
		case 2: // SUB
			PSW = FN;					
			if(( opa & 0x0F) < (opb&0x0F)) PSW |= FH;
			if(opa < opb) PSW |= FC;					
			opa -= opb;
			break;
			
		case 3: // SBC Source from DMGBoy
			if(PSW & FC) ci = 1;			
			PSW = FN;			
			sum = opb + ci;	
			if( (opa & 0x0f) < (opb & 0x0f))     PSW |= FH;
			else if((opa & 0x0f) < (sum & 0x0f)) PSW |= FH;
			else if( ((opa & 0x0f)==(opb & 0x0f)) && ((opb & 0x0f)==0x0f) && ci) PSW |= FH;			
			if( opa < sum) PSW |= FC;			
			opa -= sum ;
			break;		
			
		case 4: // AND
			PSW = FH;
			opa &= opb;
			break;
			
		case 5: // XOR		
			PSW = 0;
			opa ^= opb;
			break;
			
		case 6: // OR
			PSW = 0;
			opa |= opb;
			break;	
			
		case 7: // CP		
			PSW = FN;	
			if( opa == opb ) PSW |= FZ;
			if((opa & 0x0F) < (opb & 0x0f)) PSW |= FH;
			if(opa < opb)	PSW |= FC;
			return;		
	}
		
	if(!opa)
		PSW |= FZ;
	REG_A = opa;
}
//-----------------------------------------
// decimal adjust
// source form DMGBoy 
// https://code.google.com/p/dmgboy/
//
// Details:
// http://www.emutalk.net/showthread.php?t=41525&page=108
//-----------------------------------------
void daa(void)
{
uint16_t opa = REG_A;
    
    if (!(PSW & FN))
    {
        if ((PSW & FH) || ((opa & 0xF) > 9))
            opa += 0x06;
        
        if ((PSW & FC) || (opa > 0x9F))
            opa += 0x60;
    }
    else
    {
        if (PSW & FH)
            opa = (opa - 6) & 0xFF;
        
        if (PSW & FC)
            opa -= 0x60;
    }
    
	PSW &= ~(FZ | FH);
    
    if ((opa & 0x100) == 0x100)
        PSW |= FC;
    
    REG_A = (uint8_t)opa;
    
    if (!REG_A)
        PSW |= FZ;
}
//-----------------------------------------
// increments 8 bit register
// flags affected Z,0,H,-
//-----------------------------------------
void inc(uint8_t *r)
{
	PSW &= ~(FZ | FN | FH);	
	if((*r & 0x0F) == 0x0F)	PSW |= FH;		
	(*r)++; 
	if(*r == 0) PSW |= FZ;
}
//-----------------------------------------
// decrements 8 bit register
// flags affected Z,1,H,-
//-----------------------------------------
void dec(uint8_t *r)
{
	PSW &= ~(FZ | FH);
	PSW |= FN;
	
	if(!(*r & 0x0f)) PSW |= FH;		
	(*r)--;
    if(*r == 0) PSW |= FZ;
}
//-----------------------------------------
// rotate register left
// flags: Z,0,0,C  b7 to carry  
//-----------------------------------------
void rlc(uint8_t *r)
{
uint8_t LSb;
	LSb = *r >> 7;		
	PSW = (LSb == 0)? 0 : FC;	
    *r = (*r << 1) | LSb;
    if(*r == 0) PSW |= FZ; 
}
//-----------------------------------------
// rotate register right
// flags: Z,0,0,C  b0 to carry   
//-----------------------------------------
void rrc(uint8_t *r)
{
uint8_t MSb;	
	MSb = *r << 7;    	
	PSW = (MSb ==0)? 0 : FC;
    *r = MSb | (*r >> 1);    
    if(*r == 0) PSW |= FZ; 
}
//-----------------------------------------
// rotate register left through carry  
// flags: Z,0,0,C  carry to b0, b7 to carry
//-----------------------------------------
void rl(uint8_t *r)
{
uint8_t LSb;
	LSb = (PSW & FC) ? (1<<0) : 0;
	PSW = (*r & (1<<7))? FC : 0;
	*r = (*r<<1) | LSb;
	if(*r == 0) PSW |= FZ;	
}
//-----------------------------------------
// rotate register right through carry   
// flags: Z,0,0,C  carry to b7, b0 to carry
//-----------------------------------------
void rr(uint8_t *r)
{
uint8_t MSb;
	MSb = (PSW & FC) ? (1<<7) : 0;
	PSW = (*r & (1<<0))? FC : 0;
	*r = MSb | (*r>>1);
	if(*r == 0) PSW |= FZ;
}
//-----------------------------------------
// shift left into carry, LSb = 0
// flags: Z,0,0,C  b7 to carry
//-----------------------------------------
void sla(uint8_t *r)
{
	PSW = (*r & (1<<7)) ? FC : 0;		
	*r = *r<<1;
    if(*r == 0) PSW |= FZ;
}
//-----------------------------------------
// shift right into carry with signal extension
// flags: Z,0,0,C  b0 to carry
//-----------------------------------------
void sra(uint8_t *r)
{
	PSW = (*r & (1<<0)) ? FC : 0;
	*r = (*r & 0x80) | (*r>>1);
	if(*r == 0) PSW |= FZ;	
}
//-----------------------------------------
// shift right into carry MSb = 0
// flags: Z,0,0,C   b0 to carry
//-----------------------------------------
void srl(uint8_t *r)
{
	PSW = (*r & (1<<0)) ? FC : 0;
	*r = (*r>>1);
	if(*r == 0) PSW |= FZ;
}
//-----------------------------------------
// swap nibles
// fags: Z,0,0,0
//-----------------------------------------
void swap(uint8_t *r)
{
	PSW = 0;
	*r = (*r<<4)|(*r>>4);
	if(*r == 0) PSW |= FZ;
}
//-----------------------------------------
// bit test
// flags: Z,0,1,-
//-----------------------------------------
void BiT(uint8_t b, uint8_t *r)
{
uint8_t aux;
	PSW &= ~(FN | FZ);
	PSW |=  FH;
	
	if(r == &PSW)
	{
		aux = memoryRead(REG_HL);
		ADD_CYCLE(ONE_CYCLE);
	}
	else
		aux = *r;
	
	if(!(aux & (1<<b)))	PSW |= FZ;	
	ADD_CYCLE(TWO_CYCLE);
}
//-----------------------------------------
// bit reset
// flags: no flags affected
//-----------------------------------------
void res(uint8_t b, uint8_t *r)
{
uint8_t aux;
	
	if(r == &PSW)
	{
		aux = memoryRead(REG_HL);
		aux &= ~(1<<b);
		memoryWrite(REG_HL,aux);
		ADD_CYCLE(TWO_CYCLE);
	}
	else
		*r &= ~(1<<b);
		
	ADD_CYCLE(TWO_CYCLE);
}
//-----------------------------------------
// bit set
// flags: no flags affected
//-----------------------------------------
void set(uint8_t b, uint8_t *r)
{
uint8_t aux;
	
	if(r == &PSW)
	{
		aux = memoryRead(REG_HL);
		aux |= (1<<b);
		memoryWrite(REG_HL,aux);
		ADD_CYCLE(TWO_CYCLE);
	}
	else	
		*r |= (1<<b);
		
	ADD_CYCLE(TWO_CYCLE);
}
//-----------------------------------------
// add to HL 16bit value
// flags: -,0,H,C
//-----------------------------------------
void addHL(uint16_t v)
{
uint32_t aux;

	PSW &= ~(FN | FH| FC);	
	aux = REG_HL;
	
	if(((aux & 0x0FFF) + (v & 0x0FFF) ) > 0x0FFF) PSW |= FH;
		
	if((aux+v) > 0xFFFF) PSW |= FC;
	
	aux += v;
	REG_HL = aux & 0xFFFF;
}
//-----------------------------------------
// get value from stack (ret instruction)
//-----------------------------------------
uint16_t pop(void)
{
uint16_t v;

	v = memoryRead16(REG_SP);
	REG_SP += 2;
	return v;
}
//-----------------------------------------
// put value to stack (call instruction)
//-----------------------------------------
void push(uint16_t v)
{
	REG_SP -= 2;
	memoryWrite16(REG_SP,v);
}
//-----------------------------------------
// increment 16bit reg
// no flags affected
//-----------------------------------------
void inc16(uint16_t *r)
{
	(*r)++;
}
//-----------------------------------------
// decrement 16bit reg
// no flags affected
//-----------------------------------------

void dec16(uint16_t *r)
{
	(*r)--;
}
