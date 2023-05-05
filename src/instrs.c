#include "dmgcpu.h"
#include "instrs.h"

enum aluops_e{
	ALU_ADD = 0,
	ALU_ADC,
	ALU_SUB,
	ALU_SBC,
	ALU_AND,
	ALU_XOR,
	ALU_OR,
	ALU_CP
};


//-----------------------------------------
// ADD, ADC, SUB, SBC, AND, XOR, OR, CP
// flags affected Z,0,H,C
//-----------------------------------------
void alu(cpu_t *cpu, uint8_t op, uint8_t opb)
{
uint16_t sum;
uint8_t ci = 0;
uint8_t opa = REG_A;
	
	switch(op)
	{				
		case ALU_ADC:
			if(PSW & FC) ci = 1;
		case ALU_ADD:
			PSW = 0 ;						
			if( ((opa & 0x0F) + (opb & 0x0f) + ci) > 0x0F) PSW |= FH;
			sum = opa + opb + ci;
			if( (sum & 0xFF00) ) PSW |= FC;				
			opa = (uint8_t)sum;
			break;
		
		case ALU_SUB:
			PSW = FN;					
			if(( opa & 0x0F) < (opb&0x0F)) PSW |= FH;
			if(opa < opb) PSW |= FC;					
			opa -= opb;
			break;
			
		case ALU_SBC: // Source from DMGBoy
			if(PSW & FC) ci = 1;			
			PSW = FN;			
			sum = opb + ci;	
			if( (opa & 0x0f) < (opb & 0x0f))     PSW |= FH;
			else if((opa & 0x0f) < (sum & 0x0f)) PSW |= FH;
			else if( ((opa & 0x0f)==(opb & 0x0f)) && ((opb & 0x0f)==0x0f) && ci) PSW |= FH;			
			if( opa < sum) PSW |= FC;			
			opa -= sum ;
			break;		
			
		case ALU_AND:
			PSW = FH;
			opa &= opb;
			break;
			
		case ALU_XOR:
			PSW = 0;
			opa ^= opb;
			break;
			
		case ALU_OR:
			PSW = 0;
			opa |= opb;
			break;	
			
		case ALU_CP:
			PSW = FN;	
			if( opa == opb ) { PSW |= FZ; return; }
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
uint8_t daa(cpu_t *cpu){
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

	return ONE_CYCLE;
}
#if TABLE_DECODER
//-----------------------------------------
// rotate register left
// flags: Z,0,0,C  b7 to carry  
//-----------------------------------------
static uint8_t rlc(cpu_t *cpu, uint8_t r)
{
	uint8_t LSb = (r >> 7);
	PSW = (LSb) ? FC : 0;
    r = (r << 1) | LSb;
    if(r == 0) PSW |= FZ;
	return r;
}
//-----------------------------------------
// rotate register right
// flags: Z,0,0,C  b0 to carry   
//-----------------------------------------
static uint8_t rrc(cpu_t *cpu, uint8_t r)
{
	uint8_t MSb;	
	MSb = r << 7;    	
	PSW = (MSb ==0)? 0 : FC;
    r = MSb | (r >> 1);    
    if(r == 0) PSW |= FZ; 
	return r;
}
//-----------------------------------------
// rotate register left through carry  
// flags: Z,0,0,C  carry to b0, b7 to carry
//-----------------------------------------
static uint8_t rl(cpu_t *cpu, uint8_t r)
{
	uint8_t LSb;
	LSb = (PSW & FC) ? (1<<0) : 0;
	PSW = (r & (1<<7))? FC : 0;
	r = (r<<1) | LSb;
	if(r == 0) PSW |= FZ;
	return r;
}
//-----------------------------------------
// rotate register right through carry   
// flags: Z,0,0,C  carry to b7, b0 to carry
//-----------------------------------------
static uint8_t rr(cpu_t *cpu, uint8_t r)
{
	uint8_t MSb;
	MSb = (PSW & FC) ? (1<<7) : 0;
	PSW = (r & (1<<0))? FC : 0;
	r = MSb | (r>>1);
	if(r == 0) PSW |= FZ;
	return r;
}
//-----------------------------------------
// shift left into carry, LSb = 0
// flags: Z,0,0,C  b7 to carry
//-----------------------------------------
static uint8_t sla(cpu_t *cpu, uint8_t r)
{
	PSW = (r & (1<<7)) ? FC : 0;		
	r = r << 1;
    if(r == 0) PSW |= FZ;
	return r;
}
//-----------------------------------------
// shift right into carry with signal extension
// flags: Z,0,0,C  b0 to carry
//-----------------------------------------
static uint8_t sra(cpu_t *cpu, uint8_t r)
{
	PSW = (r & (1<<0)) ? FC : 0;
	r = (r & 0x80) | (r>>1);
	if(r == 0) PSW |= FZ;
	return r;
}
//-----------------------------------------
// shift right into carry MSb = 0
// flags: Z,0,0,C   b0 to carry
//-----------------------------------------
static uint8_t srl(cpu_t *cpu, uint8_t r)
{
	PSW = (r & (1<<0)) ? FC : 0;
	r = (r >> 1);
	if(r == 0) PSW |= FZ;
	return r;
}
//-----------------------------------------
// swap nibles
// fags: Z,0,0,0
//-----------------------------------------
static uint8_t swap(cpu_t *cpu, uint8_t r)
{
	PSW = 0;
	r = (r<<4)|(r>>4);
	if(r == 0) PSW |= FZ;
	return r;
}
//-----------------------------------------
// bit test
// flags: Z,0,1,-
//-----------------------------------------
static void bit(cpu_t *cpu, uint8_t b, uint8_t r){
	PSW = (PSW & FC) | FH;
	if(!(r & (1 << b)))	PSW |= FZ;
}

/**
 * @brief Misc/control instructions
 */
uint8_t nop(cpu_t *cpu){ (void)cpu; return ONE_CYCLE;}
uint8_t stop(cpu_t *cpu){ cpu->stopped = 1; return ONE_CYCLE; } // TODO: Implement properly
uint8_t scf(cpu_t *cpu){ PSW &= ~(FN | FH); PSW |= FC; return ONE_CYCLE; }
uint8_t cpl(cpu_t *cpu){ REG_A = ~REG_A; PSW |= FN | FH; return ONE_CYCLE; }
uint8_t ccf(cpu_t *cpu){ PSW &= ~( FN | FH); PSW ^= FC; return ONE_CYCLE; }
uint8_t di(cpu_t *cpu){ cpu->IME = 0; return ONE_CYCLE; }
uint8_t ei(cpu_t *cpu){ cpu->IME = 1; return ONE_CYCLE; }
uint8_t halt(cpu_t *cpu){ cpu->halt = 1; return ONE_CYCLE; }

/**
 * @brief 16bit arithmetic/logical instructions
 */
uint8_t inc_bc(cpu_t *cpu){ REG_BC++; return TWO_CYCLE;}
uint8_t inc_de(cpu_t *cpu){ REG_DE++; return TWO_CYCLE;}
uint8_t inc_hl(cpu_t *cpu){ REG_HL++; return TWO_CYCLE;}
uint8_t inc_sp(cpu_t *cpu){ REG_SP++; return TWO_CYCLE;}
uint8_t add_hl_bc(cpu_t *cpu){
	PSW &= ~(FN | FH| FC);
	if(((REG_HL & 0x0FFF) + (REG_BC & 0x0FFF) ) > 0x0FFF) PSW |= FH;
	if((REG_HL + REG_BC) > 0xFFFF) PSW |= FC;
	REG_HL += REG_BC;
	return TWO_CYCLE;
}
uint8_t add_hl_de(cpu_t *cpu){
	PSW &= ~(FN | FH| FC);
	if(((REG_HL & 0x0FFF) + (REG_DE & 0x0FFF) ) > 0x0FFF) PSW |= FH;
	if((REG_HL + REG_DE) > 0xFFFF) PSW |= FC;
	REG_HL += REG_DE;
	return TWO_CYCLE;
}
uint8_t add_hl_hl(cpu_t *cpu){
	PSW &= ~(FN | FH| FC);
	if(((REG_HL & 0x0FFF) + (REG_HL & 0x0FFF) ) > 0x0FFF) PSW |= FH;
	if((REG_HL + REG_HL) > 0xFFFF) PSW |= FC;
	REG_HL += REG_HL;
	return TWO_CYCLE;
}
uint8_t add_hl_sp(cpu_t *cpu){
	PSW &= ~(FN | FH| FC);
	if(((REG_HL & 0x0FFF) + (REG_SP & 0x0FFF) ) > 0x0FFF) PSW |= FH;
	if((REG_HL + REG_SP) > 0xFFFF) PSW |= FC;
	REG_HL += REG_SP;
	return TWO_CYCLE;
}
uint8_t dec_bc(cpu_t *cpu){ REG_BC--; return TWO_CYCLE; }
uint8_t dec_de(cpu_t *cpu){ REG_DE--; return TWO_CYCLE; }
uint8_t dec_hl(cpu_t *cpu){ REG_HL--; return TWO_CYCLE; }
uint8_t dec_sp(cpu_t *cpu){ REG_SP--; return TWO_CYCLE; }
uint8_t add_sp_s8(cpu_t *cpu){
	uint8_t aux = memoryRead(cpu, REG_PC++);
	PSW = 0;
	if( ((REG_SP & 0xff) + aux) > 0xFF) PSW = FC;
	if( ((REG_SP & 0x0f) + (aux & 0x0f)) > 0x0f) PSW |= FH;	
	REG_SP += (signed char)aux;
	return FOUR_CYCLE;
}
/**
 * @brief Jumps/calls
 */
uint8_t jr_s8(cpu_t *cpu){
	REG_PC = REG_PC + (signed char)memoryRead(cpu, REG_PC) + 1;
	return THREE_CYCLE;
}
uint8_t jr_nz_s8(cpu_t *cpu){	
	if (!(PSW & FZ)) {
		REG_PC = REG_PC + (signed char)memoryRead(cpu, REG_PC) + 1;
		return THREE_CYCLE;
	}else
		REG_PC++;				
	return TWO_CYCLE;
}
uint8_t jr_nc_s8(cpu_t *cpu){	
	if (!(PSW & FC)) {
		REG_PC = REG_PC + (signed char)memoryRead(cpu, REG_PC) + 1;
		return THREE_CYCLE;
	}else
		REG_PC++;				
	return TWO_CYCLE;
}
uint8_t jr_z_s8(cpu_t *cpu){	
	if (PSW & FZ) {
		REG_PC = REG_PC + (signed char)memoryRead(cpu, REG_PC) + 1;
		return THREE_CYCLE;
	}else
		REG_PC++;				
	return TWO_CYCLE;
}
uint8_t jr_c_s8(cpu_t *cpu){
	if (PSW & FC) {
		REG_PC = REG_PC + (signed char)memoryRead(cpu, REG_PC) + 1;
		return THREE_CYCLE;
	}else
		REG_PC++;				
	return TWO_CYCLE;
}
uint8_t ret_nz(cpu_t *cpu){
	if (!(PSW & FZ)) {
		POP(cpu, REG_PC);
		return FIVE_CYCLE;
	}
	return TWO_CYCLE;
}
uint8_t ret_nc(cpu_t *cpu){
	if (!(PSW & FC)) {
		POP(cpu, REG_PC);
		return FIVE_CYCLE;
	}
	return TWO_CYCLE;
}
uint8_t jp_nz_a16(cpu_t *cpu){
	if (!(PSW & FZ)) {
		REG_PC = memoryRead16(cpu, REG_PC);
		return FOUR_CYCLE;
	}
	else	
		REG_PC += 2;
	return THREE_CYCLE;
}
uint8_t jp_nc_a16(cpu_t *cpu){
	if (!(PSW & FC)) {
		REG_PC = memoryRead16(cpu, REG_PC);
		return FOUR_CYCLE;
	}
	else	
		REG_PC += 2;
	return THREE_CYCLE;
}
uint8_t jp_a16(cpu_t *cpu){REG_PC = memoryRead16(cpu, REG_PC); return FOUR_CYCLE;}
uint8_t call_nz_a16(cpu_t *cpu){
	if(!(PSW & FZ))	{
		PUSH(cpu, REG_PC + 2);
		REG_PC = memoryRead16(cpu, REG_PC);
		return SIX_CYCLE;
	}
	else
		REG_PC += 2;
	return THREE_CYCLE;
}
uint8_t call_nc_a16(cpu_t *cpu){
	if(!(PSW & FC))	{
		PUSH(cpu, REG_PC + 2);
		REG_PC = memoryRead16(cpu, REG_PC);
		return SIX_CYCLE;
	}
	else
		REG_PC += 2;
	return THREE_CYCLE;
}
uint8_t rst_0(cpu_t *cpu){PUSH(cpu, REG_PC); REG_PC = 0x00; return FOUR_CYCLE;}
uint8_t rst_1(cpu_t *cpu){PUSH(cpu, REG_PC); REG_PC = 0x08; return FOUR_CYCLE;}
uint8_t rst_2(cpu_t *cpu){PUSH(cpu, REG_PC); REG_PC = 0x10; return FOUR_CYCLE;}
uint8_t rst_3(cpu_t *cpu){PUSH(cpu, REG_PC); REG_PC = 0x18; return FOUR_CYCLE;}
uint8_t rst_4(cpu_t *cpu){PUSH(cpu, REG_PC); REG_PC = 0x20; return FOUR_CYCLE;}
uint8_t rst_5(cpu_t *cpu){PUSH(cpu, REG_PC); REG_PC = 0x28; return FOUR_CYCLE;}
uint8_t rst_6(cpu_t *cpu){PUSH(cpu, REG_PC); REG_PC = 0x30; return FOUR_CYCLE;}
uint8_t rst_7(cpu_t *cpu){PUSH(cpu, REG_PC); REG_PC = 0x38; return FOUR_CYCLE;}
uint8_t ret_z(cpu_t *cpu){
	if (PSW & FZ) {
		POP(cpu, REG_PC);
		return FIVE_CYCLE;
	}
	return TWO_CYCLE;
}
uint8_t ret_c(cpu_t *cpu){
	if (PSW & FC) {
		POP(cpu, REG_PC);
		return FIVE_CYCLE;
	}
	return TWO_CYCLE;
}
uint8_t ret(cpu_t *cpu){ POP(cpu, REG_PC); return FOUR_CYCLE;}
uint8_t reti(cpu_t *cpu){ cpu->IME = 1; POP(cpu, REG_PC); return FOUR_CYCLE;}
uint8_t jp_hl(cpu_t *cpu){REG_PC = REG_HL; return ONE_CYCLE;}
uint8_t jp_z_a16(cpu_t *cpu){
	if (PSW & FZ) {
		REG_PC = memoryRead16(cpu, REG_PC);
		return FOUR_CYCLE;
	}
	else	
		REG_PC += 2;
	return THREE_CYCLE;
}
uint8_t jp_c_a16(cpu_t *cpu){
	if (PSW & FC) {
		REG_PC = memoryRead16(cpu, REG_PC);
		return FOUR_CYCLE;
	}
	else	
		REG_PC += 2;
	return THREE_CYCLE;
}
uint8_t call_z_a16(cpu_t *cpu){
	if (PSW & FZ) {
		PUSH(cpu, REG_PC+2);
		REG_PC = memoryRead16(cpu, REG_PC);
		return SIX_CYCLE;
	}
	else	
		REG_PC += 2;
	return THREE_CYCLE;
}
uint8_t call_c_a16(cpu_t *cpu){
	if (PSW & FC) {
		PUSH(cpu, REG_PC+2);
		REG_PC = memoryRead16(cpu, REG_PC);
		return SIX_CYCLE;
	}
	else	
		REG_PC += 2;
	return THREE_CYCLE;
}
uint8_t call_a16(cpu_t *cpu){
	PUSH(cpu, REG_PC+2);
	REG_PC = memoryRead16(cpu, REG_PC);
	return SIX_CYCLE;
}
/**
 * @brief 8bit arithmetic/logical instructions
 */	
uint8_t inc_a(cpu_t *cpu){
	PSW &= ~(FZ | FN | FH);	
	if((REG_A & 0x0F) == 0x0F) PSW |= FH;
	if(++REG_A == 0) PSW |= FZ;
	return ONE_CYCLE;
}
uint8_t inc_b(cpu_t *cpu){
	PSW &= ~(FZ | FN | FH);	
	if((REG_B & 0x0F) == 0x0F) PSW |= FH;
	if(++REG_B == 0) PSW |= FZ;
	return ONE_CYCLE;
}
uint8_t inc_c(cpu_t *cpu){
	PSW &= ~(FZ | FN | FH);	
	if((REG_C & 0x0F) == 0x0F)	PSW |= FH;
	if(++REG_C == 0) PSW |= FZ;
	return ONE_CYCLE;
}
uint8_t inc_d(cpu_t *cpu){
	PSW &= ~(FZ | FN | FH);	
	if((REG_D & 0x0F) == 0x0F) PSW |= FH;
	if(++REG_D == 0) PSW |= FZ;
	return ONE_CYCLE;
}
uint8_t inc_e(cpu_t *cpu){
	PSW &= ~(FZ | FN | FH);	
	if((REG_E & 0x0F) == 0x0F) PSW |= FH;
	if(++REG_E == 0) PSW |= FZ;
	return ONE_CYCLE;
}
uint8_t inc_h(cpu_t *cpu){
	PSW &= ~(FZ | FN | FH);	
	if((REG_H & 0x0F) == 0x0F) PSW |= FH;		
	if(++REG_H == 0) PSW |= FZ;
	return ONE_CYCLE;
}
uint8_t inc_l(cpu_t *cpu){
	PSW &= ~(FZ | FN | FH);
	if((REG_L & 0x0F) == 0x0F) PSW |= FH;
	if(++REG_L == 0) PSW |= FZ;
	return ONE_CYCLE;
}
uint8_t inc_ind_hl(cpu_t *cpu){
	PSW &= ~(FZ | FN | FH);
	uint8_t aux = memoryRead(cpu, REG_HL);
	if((aux & 0x0F) == 0x0F) PSW |= FH;
	if(++aux == 0) PSW |= FZ;
	memoryWrite(cpu, REG_HL, aux);
	return THREE_CYCLE;
}
uint8_t dec_a(cpu_t *cpu) {
	PSW &= ~(FZ | FH);
	PSW |= FN;
	if(!(REG_A & 0x0f)) PSW |= FH;
    if(--REG_A == 0) PSW |= FZ;
	return ONE_CYCLE;
}
uint8_t dec_b(cpu_t *cpu) {
	PSW &= ~(FZ | FH);
	PSW |= FN;
	if(!(REG_B & 0x0f)) PSW |= FH;		
    if(--REG_B == 0) PSW |= FZ;
	return ONE_CYCLE;
}
uint8_t dec_c(cpu_t *cpu){
	PSW &= ~(FZ | FH);
	PSW |= FN;	
	if(!(REG_C & 0x0f)) PSW |= FH;		
    if(--REG_C == 0) PSW |= FZ;
	return ONE_CYCLE;
}
uint8_t dec_d(cpu_t *cpu) {
	PSW &= ~(FZ | FH);
	PSW |= FN;
	if(!(REG_D & 0x0f)) PSW |= FH;		
    if(--REG_D == 0) PSW |= FZ;
	return ONE_CYCLE;
}
uint8_t dec_e(cpu_t *cpu) {
	PSW &= ~(FZ | FH);
	PSW |= FN;
	if(!(REG_E & 0x0f)) PSW |= FH;
    if(--REG_E == 0) PSW |= FZ;
	return ONE_CYCLE;
}
uint8_t dec_h(cpu_t *cpu) {
	PSW &= ~(FZ | FH);
	PSW |= FN;
	if(!(REG_H & 0x0f)) PSW |= FH;
    if(--REG_H == 0) PSW |= FZ;
	return ONE_CYCLE;
}
uint8_t dec_l(cpu_t *cpu) {
	PSW &= ~(FZ | FH);
	PSW |= FN;
	if(!(REG_L & 0x0f)) PSW |= FH;		
    if(--REG_L == 0) PSW |= FZ;
	return ONE_CYCLE;
}
uint8_t dec_ind_hl(cpu_t *cpu){
	uint8_t aux = memoryRead(cpu, REG_HL);
	PSW &= ~(FZ | FH);
	PSW |= FN;
	if(!(aux & 0x0F)) PSW |= FH;
	if(--aux == 0) PSW |= FZ;
	memoryWrite(cpu, REG_HL, aux);
	return THREE_CYCLE;
}

uint8_t add_a_a(cpu_t *cpu){alu(cpu, ALU_ADD, REG_A); return ONE_CYCLE;}
uint8_t add_a_b(cpu_t *cpu){alu(cpu, ALU_ADD, REG_B); return ONE_CYCLE;}
uint8_t add_a_c(cpu_t *cpu){alu(cpu, ALU_ADD, REG_C); return ONE_CYCLE;}
uint8_t add_a_d(cpu_t *cpu){alu(cpu, ALU_ADD, REG_D); return ONE_CYCLE;}
uint8_t add_a_e(cpu_t *cpu){alu(cpu, ALU_ADD, REG_E); return ONE_CYCLE;}
uint8_t add_a_h(cpu_t *cpu){alu(cpu, ALU_ADD, REG_H); return ONE_CYCLE;}
uint8_t add_a_l(cpu_t *cpu){alu(cpu, ALU_ADD, REG_L); return ONE_CYCLE;}
uint8_t add_a_d8(cpu_t *cpu){alu(cpu, ALU_ADD, memoryRead(cpu, REG_PC++)); return TWO_CYCLE;}
uint8_t add_a_ind_hl(cpu_t *cpu){alu( cpu, ALU_ADD, memoryRead(cpu, REG_HL)); return TWO_CYCLE;}
uint8_t adc_a_a(cpu_t *cpu){alu(cpu, ALU_ADC, REG_A); return ONE_CYCLE;}
uint8_t adc_a_b(cpu_t *cpu){alu(cpu, ALU_ADC, REG_B); return ONE_CYCLE;}
uint8_t adc_a_c(cpu_t *cpu){alu(cpu, ALU_ADC, REG_C); return ONE_CYCLE;}
uint8_t adc_a_d(cpu_t *cpu){alu(cpu, ALU_ADC, REG_D); return ONE_CYCLE;}
uint8_t adc_a_e(cpu_t *cpu){alu(cpu, ALU_ADC, REG_E); return ONE_CYCLE;}
uint8_t adc_a_h(cpu_t *cpu){alu(cpu, ALU_ADC, REG_H); return ONE_CYCLE;}
uint8_t adc_a_l(cpu_t *cpu){alu(cpu, ALU_ADC, REG_L); return ONE_CYCLE;}
uint8_t adc_a_d8(cpu_t *cpu){alu(cpu, ALU_ADC, memoryRead(cpu, REG_PC++)); return TWO_CYCLE;}
uint8_t adc_a_ind_hl(cpu_t *cpu){alu(cpu, ALU_ADC, memoryRead(cpu, REG_HL)); return TWO_CYCLE;}
uint8_t sub_a(cpu_t *cpu){alu(cpu, ALU_SUB, REG_A); return ONE_CYCLE;}
uint8_t sub_b(cpu_t *cpu){alu(cpu, ALU_SUB, REG_B); return ONE_CYCLE;}
uint8_t sub_c(cpu_t *cpu){alu(cpu, ALU_SUB, REG_C); return ONE_CYCLE;}
uint8_t sub_d(cpu_t *cpu){alu(cpu, ALU_SUB, REG_D); return ONE_CYCLE;}
uint8_t sub_e(cpu_t *cpu){alu(cpu, ALU_SUB, REG_E); return ONE_CYCLE;}
uint8_t sub_h(cpu_t *cpu){alu(cpu, ALU_SUB, REG_H); return ONE_CYCLE;}
uint8_t sub_l(cpu_t *cpu){alu(cpu, ALU_SUB, REG_L); return ONE_CYCLE;}
uint8_t sub_d8(cpu_t *cpu){alu(cpu, ALU_SUB, memoryRead(cpu, REG_PC++)); return TWO_CYCLE;}
uint8_t sub_ind_hl(cpu_t *cpu){alu(cpu, ALU_SUB, memoryRead(cpu, REG_HL)); return TWO_CYCLE;}
uint8_t sbc_a_a(cpu_t *cpu){alu(cpu, ALU_SBC, REG_A); return ONE_CYCLE;}
uint8_t sbc_a_b(cpu_t *cpu){alu(cpu, ALU_SBC, REG_B); return ONE_CYCLE;}
uint8_t sbc_a_c(cpu_t *cpu){alu(cpu, ALU_SBC, REG_C); return ONE_CYCLE;}
uint8_t sbc_a_d(cpu_t *cpu){alu(cpu, ALU_SBC, REG_D); return ONE_CYCLE;}
uint8_t sbc_a_e(cpu_t *cpu){alu(cpu, ALU_SBC, REG_E); return ONE_CYCLE;}
uint8_t sbc_a_h(cpu_t *cpu){alu(cpu, ALU_SBC, REG_H); return ONE_CYCLE;}
uint8_t sbc_a_l(cpu_t *cpu){alu(cpu, ALU_SBC, REG_L); return ONE_CYCLE;}
uint8_t sbc_a_d8(cpu_t *cpu){alu(cpu, ALU_SBC, memoryRead(cpu, REG_PC++)); return TWO_CYCLE;}
uint8_t sbc_a_ind_hl(cpu_t *cpu){alu(cpu, ALU_SBC, memoryRead(cpu, REG_HL)); return TWO_CYCLE;}
uint8_t and_a(cpu_t *cpu){alu(cpu, ALU_AND, REG_A); return ONE_CYCLE;}
uint8_t and_b(cpu_t *cpu){alu(cpu, ALU_AND, REG_B); return ONE_CYCLE;}
uint8_t and_c(cpu_t *cpu){alu(cpu, ALU_AND, REG_C); return ONE_CYCLE;}
uint8_t and_d(cpu_t *cpu){alu(cpu, ALU_AND, REG_D); return ONE_CYCLE;}
uint8_t and_e(cpu_t *cpu){alu(cpu, ALU_AND, REG_E); return ONE_CYCLE;}
uint8_t and_h(cpu_t *cpu){alu(cpu, ALU_AND, REG_H); return ONE_CYCLE;}
uint8_t and_l(cpu_t *cpu){alu(cpu, ALU_AND, REG_L); return ONE_CYCLE;}
uint8_t and_d8(cpu_t *cpu){alu(cpu, ALU_AND, memoryRead(cpu, REG_PC++)); return TWO_CYCLE;}
uint8_t and_ind_hl(cpu_t *cpu){alu(cpu, ALU_AND, memoryRead(cpu, REG_HL)); return TWO_CYCLE;}
uint8_t xor_a(cpu_t *cpu){alu(cpu, ALU_XOR, REG_A); return ONE_CYCLE;}
uint8_t xor_b(cpu_t *cpu){alu(cpu, ALU_XOR, REG_B); return ONE_CYCLE;}
uint8_t xor_c(cpu_t *cpu){alu(cpu, ALU_XOR, REG_C); return ONE_CYCLE;}
uint8_t xor_d(cpu_t *cpu){alu(cpu, ALU_XOR, REG_D); return ONE_CYCLE;}
uint8_t xor_e(cpu_t *cpu){alu(cpu, ALU_XOR, REG_E); return ONE_CYCLE;}
uint8_t xor_h(cpu_t *cpu){alu(cpu, ALU_XOR, REG_H); return ONE_CYCLE;}
uint8_t xor_l(cpu_t *cpu){alu(cpu, ALU_XOR, REG_L); return ONE_CYCLE;}
uint8_t xor_d8(cpu_t *cpu){alu(cpu, ALU_XOR, memoryRead(cpu, REG_PC++)); return TWO_CYCLE;}
uint8_t xor_ind_hl(cpu_t *cpu){alu(cpu, ALU_XOR, memoryRead(cpu, REG_HL)); return TWO_CYCLE;}
uint8_t or_a(cpu_t *cpu){alu(cpu, ALU_OR, REG_A); return ONE_CYCLE;}
uint8_t or_b(cpu_t *cpu){alu(cpu, ALU_OR, REG_B); return ONE_CYCLE;}
uint8_t or_c(cpu_t *cpu){alu(cpu, ALU_OR, REG_C); return ONE_CYCLE;}
uint8_t or_d(cpu_t *cpu){alu(cpu, ALU_OR, REG_D); return ONE_CYCLE;}
uint8_t or_e(cpu_t *cpu){alu(cpu, ALU_OR, REG_E); return ONE_CYCLE;}
uint8_t or_h(cpu_t *cpu){alu(cpu, ALU_OR, REG_H); return ONE_CYCLE;}
uint8_t or_l(cpu_t *cpu){alu(cpu, ALU_OR, REG_L); return ONE_CYCLE;}
uint8_t or_d8(cpu_t *cpu){alu(cpu, ALU_OR, memoryRead(cpu, REG_PC++)); return TWO_CYCLE;}
uint8_t or_ind_hl(cpu_t *cpu){alu(cpu, ALU_OR, memoryRead(cpu, REG_HL)); return TWO_CYCLE;}
uint8_t cp_a(cpu_t *cpu){alu(cpu, ALU_CP, REG_A); return ONE_CYCLE;}
uint8_t cp_b(cpu_t *cpu){alu(cpu, ALU_CP, REG_B); return ONE_CYCLE;}
uint8_t cp_c(cpu_t *cpu){alu(cpu, ALU_CP, REG_C); return ONE_CYCLE;}
uint8_t cp_d(cpu_t *cpu){alu(cpu, ALU_CP, REG_D); return ONE_CYCLE;}
uint8_t cp_e(cpu_t *cpu){alu(cpu, ALU_CP, REG_E); return ONE_CYCLE;}
uint8_t cp_h(cpu_t *cpu){alu(cpu, ALU_CP, REG_H); return ONE_CYCLE;}
uint8_t cp_l(cpu_t *cpu){alu(cpu, ALU_CP, REG_L); return ONE_CYCLE;}
uint8_t cp_d8(cpu_t *cpu){alu(cpu, ALU_CP, memoryRead(cpu, REG_PC++)); return TWO_CYCLE;}
uint8_t cp_ind_hl(cpu_t *cpu){alu(cpu, ALU_CP, memoryRead(cpu, REG_HL)); return TWO_CYCLE;}

/**
 * @brief 8bit load/store/move instructions
 */
uint8_t ld_a_a(cpu_t *cpu){(void)cpu; return ONE_CYCLE;}
uint8_t ld_a_b(cpu_t *cpu){REG_A = REG_B; return ONE_CYCLE;}
uint8_t ld_a_c(cpu_t *cpu){REG_A = REG_C; return ONE_CYCLE;}
uint8_t ld_a_d(cpu_t *cpu){REG_A = REG_D; return ONE_CYCLE;}
uint8_t ld_a_e(cpu_t *cpu){REG_A = REG_E; return ONE_CYCLE;}
uint8_t ld_a_h(cpu_t *cpu){REG_A = REG_H; return ONE_CYCLE;}
uint8_t ld_a_l(cpu_t *cpu){REG_A = REG_L; return ONE_CYCLE;}
uint8_t ld_a_d8(cpu_t *cpu){ REG_A = memoryRead(cpu, REG_PC++); return TWO_CYCLE; }
uint8_t ld_a_ind_bc(cpu_t *cpu){ REG_A = memoryRead(cpu, REG_BC); return TWO_CYCLE; }
uint8_t ld_a_ind_de(cpu_t *cpu){ REG_A = memoryRead(cpu, REG_DE); return TWO_CYCLE;}
uint8_t ld_a_ind_hl(cpu_t *cpu){REG_A = memoryRead(cpu, REG_HL); return TWO_CYCLE;}
uint8_t ld_a_ind_hli(cpu_t *cpu){REG_A = memoryRead(cpu, REG_HL++); return TWO_CYCLE;}
uint8_t ld_a_ind_hld(cpu_t *cpu){REG_A = memoryRead(cpu, REG_HL--); return TWO_CYCLE;}
uint8_t ld_a_ind_c(cpu_t *cpu){REG_A = memoryRead(cpu, 0xFF00 | REG_C); return TWO_CYCLE;}
uint8_t ld_a_ind_a16(cpu_t *cpu){REG_A = memoryRead(cpu, memoryRead16(cpu, REG_PC)); REG_PC +=2; return FOUR_CYCLE;}
uint8_t ld_a_ind_a8(cpu_t *cpu){REG_A = memoryRead(cpu, 0xFF00 | memoryRead(cpu, REG_PC++)); return THREE_CYCLE;}
uint8_t ld_b_a(cpu_t *cpu){REG_B = REG_A; return ONE_CYCLE;}
uint8_t ld_b_b(cpu_t *cpu){(void)cpu; return ONE_CYCLE;}
uint8_t ld_b_c(cpu_t *cpu){REG_B = REG_C; return ONE_CYCLE;}
uint8_t ld_b_d(cpu_t *cpu){REG_B = REG_D; return ONE_CYCLE;}
uint8_t ld_b_e(cpu_t *cpu){REG_B = REG_E; return ONE_CYCLE;}
uint8_t ld_b_h(cpu_t *cpu){REG_B = REG_H; return ONE_CYCLE;}
uint8_t ld_b_l(cpu_t *cpu){REG_B = REG_L; return ONE_CYCLE;}
uint8_t ld_b_d8(cpu_t *cpu){ REG_B = memoryRead(cpu, REG_PC++); return TWO_CYCLE; }
uint8_t ld_b_ind_hl(cpu_t *cpu){REG_B = memoryRead(cpu, REG_HL); return TWO_CYCLE;}
uint8_t ld_c_a(cpu_t *cpu){REG_C = REG_A; return ONE_CYCLE;}
uint8_t ld_c_b(cpu_t *cpu){REG_C = REG_B; return ONE_CYCLE;}
uint8_t ld_c_c(cpu_t *cpu){(void)cpu; return ONE_CYCLE;}
uint8_t ld_c_d(cpu_t *cpu){REG_C = REG_D; return ONE_CYCLE;}
uint8_t ld_c_e(cpu_t *cpu){REG_C = REG_E; return ONE_CYCLE;}
uint8_t ld_c_h(cpu_t *cpu){REG_C = REG_H; return ONE_CYCLE;}
uint8_t ld_c_l(cpu_t *cpu){REG_C = REG_L; return ONE_CYCLE;}
uint8_t ld_c_d8(cpu_t *cpu){ REG_C = memoryRead(cpu, REG_PC++); return TWO_CYCLE; }
uint8_t ld_c_ind_hl(cpu_t *cpu){REG_C = memoryRead(cpu, REG_HL); return TWO_CYCLE;}
uint8_t ld_d_a(cpu_t *cpu){REG_D = REG_A; return ONE_CYCLE;}
uint8_t ld_d_b(cpu_t *cpu){REG_D = REG_B; return ONE_CYCLE;}
uint8_t ld_d_c(cpu_t *cpu){REG_D = REG_C; return ONE_CYCLE;}
uint8_t ld_d_d(cpu_t *cpu){(void)cpu; return ONE_CYCLE;}
uint8_t ld_d_e(cpu_t *cpu){REG_D = REG_E; return ONE_CYCLE;}
uint8_t ld_d_h(cpu_t *cpu){REG_D = REG_H; return ONE_CYCLE;}
uint8_t ld_d_l(cpu_t *cpu){REG_D = REG_L; return ONE_CYCLE;}
uint8_t ld_d_d8(cpu_t *cpu){ REG_D = memoryRead(cpu, REG_PC++); return TWO_CYCLE; }
uint8_t ld_d_ind_hl(cpu_t *cpu){REG_D = memoryRead(cpu, REG_HL); return TWO_CYCLE;}
uint8_t ld_e_a(cpu_t *cpu){REG_E = REG_A; return ONE_CYCLE;}
uint8_t ld_e_b(cpu_t *cpu){REG_E = REG_B; return ONE_CYCLE;}
uint8_t ld_e_c(cpu_t *cpu){REG_E = REG_C; return ONE_CYCLE;}
uint8_t ld_e_d(cpu_t *cpu){REG_E = REG_D; return ONE_CYCLE;}
uint8_t ld_e_e(cpu_t *cpu){(void)cpu; return ONE_CYCLE;}
uint8_t ld_e_h(cpu_t *cpu){REG_E = REG_H; return ONE_CYCLE;}
uint8_t ld_e_l(cpu_t *cpu){REG_E = REG_L; return ONE_CYCLE;}
uint8_t ld_e_d8(cpu_t *cpu){ REG_E = memoryRead(cpu, REG_PC++); return TWO_CYCLE; }
uint8_t ld_e_ind_hl(cpu_t *cpu){REG_E = memoryRead(cpu, REG_HL); return TWO_CYCLE;}
uint8_t ld_h_a(cpu_t *cpu){REG_H = REG_A; return ONE_CYCLE;}
uint8_t ld_h_b(cpu_t *cpu){REG_H = REG_B; return ONE_CYCLE;}
uint8_t ld_h_c(cpu_t *cpu){REG_H = REG_C; return ONE_CYCLE;}
uint8_t ld_h_d(cpu_t *cpu){REG_H = REG_D; return ONE_CYCLE;}
uint8_t ld_h_e(cpu_t *cpu){REG_H = REG_E; return ONE_CYCLE;}
uint8_t ld_h_h(cpu_t *cpu){(void)cpu; return ONE_CYCLE;}
uint8_t ld_h_l(cpu_t *cpu){REG_H = REG_L; return ONE_CYCLE;}
uint8_t ld_h_d8(cpu_t *cpu){ REG_H = memoryRead(cpu, REG_PC++); return TWO_CYCLE; }
uint8_t ld_h_ind_hl(cpu_t *cpu){REG_H = memoryRead(cpu, REG_HL); return TWO_CYCLE;}
uint8_t ld_l_a(cpu_t *cpu){REG_L = REG_A; return ONE_CYCLE;}
uint8_t ld_l_b(cpu_t *cpu){REG_L = REG_B; return ONE_CYCLE;}
uint8_t ld_l_c(cpu_t *cpu){REG_L = REG_C; return ONE_CYCLE;}
uint8_t ld_l_d(cpu_t *cpu){REG_L = REG_D; return ONE_CYCLE;}
uint8_t ld_l_e(cpu_t *cpu){REG_L = REG_E; return ONE_CYCLE;}
uint8_t ld_l_h(cpu_t *cpu){REG_L = REG_H; return ONE_CYCLE;}
uint8_t ld_l_l(cpu_t *cpu){(void)cpu; return ONE_CYCLE;}
uint8_t ld_l_d8(cpu_t *cpu){REG_L = memoryRead(cpu, REG_PC++); return TWO_CYCLE;}
uint8_t ld_l_ind_hl(cpu_t *cpu){REG_L = memoryRead(cpu, REG_HL); return TWO_CYCLE;}
uint8_t ld_ind_hl_a(cpu_t *cpu){memoryWrite(cpu, REG_HL, REG_A); return TWO_CYCLE;}
uint8_t ld_ind_hl_b(cpu_t *cpu){memoryWrite(cpu, REG_HL, REG_B); return TWO_CYCLE;}
uint8_t ld_ind_hl_c(cpu_t *cpu){memoryWrite(cpu, REG_HL, REG_C); return TWO_CYCLE;}
uint8_t ld_ind_hl_d(cpu_t *cpu){memoryWrite(cpu, REG_HL, REG_D); return TWO_CYCLE;}
uint8_t ld_ind_hl_e(cpu_t *cpu){memoryWrite(cpu, REG_HL, REG_E); return TWO_CYCLE;}
uint8_t ld_ind_hl_h(cpu_t *cpu){memoryWrite(cpu, REG_HL, REG_H); return TWO_CYCLE;}
uint8_t ld_ind_hl_l(cpu_t *cpu){memoryWrite(cpu, REG_HL, REG_L); return TWO_CYCLE;}
uint8_t ld_ind_hl_d8(cpu_t *cpu){memoryWrite(cpu, REG_HL, memoryRead(cpu, REG_PC++)); return THREE_CYCLE;}
uint8_t ld_ind_hli_a(cpu_t *cpu){memoryWrite(cpu, REG_HL++, REG_A); return TWO_CYCLE;}
uint8_t ld_ind_hld_a(cpu_t *cpu){memoryWrite(cpu, REG_HL--, REG_A); return TWO_CYCLE;}
uint8_t ld_ind_bc_a(cpu_t *cpu){memoryWrite(cpu, REG_BC, REG_A); return TWO_CYCLE;}
uint8_t ld_ind_de_a(cpu_t *cpu){memoryWrite(cpu, REG_DE, REG_A); return TWO_CYCLE;}
uint8_t ld_ind_c_a(cpu_t *cpu){memoryWrite(cpu, 0xFF00 | REG_C, REG_A); return TWO_CYCLE;}
uint8_t ld_ind_a8_a(cpu_t *cpu){memoryWrite(cpu, 0xFF00 | memoryRead(cpu, REG_PC++), REG_A); return THREE_CYCLE;}
uint8_t ld_ind_a16_a(cpu_t *cpu){memoryWrite(cpu, memoryRead16(cpu, REG_PC), REG_A); REG_PC +=2; return FOUR_CYCLE;}
/**
 * @brief 8bit rotations/shifts and bit instructions
 */
uint8_t rlca(cpu_t *cpu){
	uint8_t aux = REG_A >> 7;		
	PSW = (aux == 0) ? 0 : FC;	
    REG_A = (REG_A << 1) | aux;		
	return ONE_CYCLE;
}
uint8_t rla(cpu_t *cpu){
	uint8_t aux = (PSW & FC) ? 1 : 0;
	PSW = (REG_A & (1<<7)) ? FC : 0;
	REG_A = (REG_A << 1) | aux;			
	return ONE_CYCLE;
}
uint8_t rrca(cpu_t *cpu){
	uint8_t aux = REG_A << 7;    	
	PSW = (aux & (1<<7)) ? FC : 0;
	REG_A = aux | (REG_A >> 1);    
	return ONE_CYCLE;
}
uint8_t rra(cpu_t *cpu){
	uint8_t aux = (PSW & FC) ? (1<<7) : 0;
	PSW = (REG_A & (1<<0)) ? FC : 0;
	REG_A = aux | (REG_A >> 1);
	return ONE_CYCLE;	
}

uint8_t rlc_a(cpu_t *cpu){REG_A = rlc(cpu, REG_A); return TWO_CYCLE;}
uint8_t rlc_b(cpu_t *cpu){REG_B = rlc(cpu, REG_B); return TWO_CYCLE;}
uint8_t rlc_c(cpu_t *cpu){REG_C = rlc(cpu, REG_C); return TWO_CYCLE;}
uint8_t rlc_d(cpu_t *cpu){REG_D = rlc(cpu, REG_D); return TWO_CYCLE;}
uint8_t rlc_e(cpu_t *cpu){REG_E = rlc(cpu, REG_E); return TWO_CYCLE;}
uint8_t rlc_h(cpu_t *cpu){REG_H = rlc(cpu, REG_H); return TWO_CYCLE;}
uint8_t rlc_l(cpu_t *cpu){REG_L = rlc(cpu, REG_L); return TWO_CYCLE;}
uint8_t rlc_ind_hl(cpu_t *cpu){memoryWrite(cpu, REG_HL, rlc(cpu, memoryRead(cpu, REG_HL))); return FOUR_CYCLE;}
uint8_t rrc_a(cpu_t *cpu){REG_A = rrc(cpu, REG_A); return TWO_CYCLE;}
uint8_t rrc_b(cpu_t *cpu){REG_B = rrc(cpu, REG_B); return TWO_CYCLE;}
uint8_t rrc_c(cpu_t *cpu){REG_C = rrc(cpu, REG_C); return TWO_CYCLE;}
uint8_t rrc_d(cpu_t *cpu){REG_D = rrc(cpu, REG_D); return TWO_CYCLE;}
uint8_t rrc_e(cpu_t *cpu){REG_E = rrc(cpu, REG_E); return TWO_CYCLE;}
uint8_t rrc_h(cpu_t *cpu){REG_H = rrc(cpu, REG_H); return TWO_CYCLE;}
uint8_t rrc_l(cpu_t *cpu){REG_L = rrc(cpu, REG_L); return TWO_CYCLE;}
uint8_t rrc_ind_hl(cpu_t *cpu){memoryWrite(cpu, REG_HL, rrc(cpu, memoryRead(cpu, REG_HL))); return FOUR_CYCLE;}
uint8_t rl_a(cpu_t *cpu){REG_A = rl(cpu, REG_A); return TWO_CYCLE;}
uint8_t rl_b(cpu_t *cpu){REG_B = rl(cpu, REG_B); return TWO_CYCLE;}
uint8_t rl_c(cpu_t *cpu){REG_C = rl(cpu, REG_C); return TWO_CYCLE;}
uint8_t rl_d(cpu_t *cpu){REG_D = rl(cpu, REG_D); return TWO_CYCLE;}
uint8_t rl_e(cpu_t *cpu){REG_E = rl(cpu, REG_E); return TWO_CYCLE;}
uint8_t rl_h(cpu_t *cpu){REG_H = rl(cpu, REG_H); return TWO_CYCLE;}
uint8_t rl_l(cpu_t *cpu){REG_L = rl(cpu, REG_L); return TWO_CYCLE;}
uint8_t rl_ind_hl(cpu_t *cpu){memoryWrite(cpu, REG_HL, rl(cpu, memoryRead(cpu, REG_HL))); return FOUR_CYCLE;}
uint8_t rr_a(cpu_t *cpu){REG_A = rr(cpu, REG_A); return TWO_CYCLE;}
uint8_t rr_b(cpu_t *cpu){REG_B = rr(cpu, REG_B); return TWO_CYCLE;}
uint8_t rr_c(cpu_t *cpu){REG_C = rr(cpu, REG_C); return TWO_CYCLE;}
uint8_t rr_d(cpu_t *cpu){REG_D = rr(cpu, REG_D); return TWO_CYCLE;}
uint8_t rr_e(cpu_t *cpu){REG_E = rr(cpu, REG_E); return TWO_CYCLE;}
uint8_t rr_h(cpu_t *cpu){REG_H = rr(cpu, REG_H); return TWO_CYCLE;}
uint8_t rr_l(cpu_t *cpu){REG_L = rr(cpu, REG_L); return TWO_CYCLE;}
uint8_t rr_ind_hl(cpu_t *cpu){memoryWrite(cpu, REG_HL, rr(cpu, memoryRead(cpu, REG_HL))); return FOUR_CYCLE;}
uint8_t sla_a(cpu_t *cpu){REG_A = sla(cpu, REG_A); return TWO_CYCLE;}
uint8_t sla_b(cpu_t *cpu){REG_B = sla(cpu, REG_B); return TWO_CYCLE;}
uint8_t sla_c(cpu_t *cpu){REG_C = sla(cpu, REG_C); return TWO_CYCLE;}
uint8_t sla_d(cpu_t *cpu){REG_D = sla(cpu, REG_D); return TWO_CYCLE;}
uint8_t sla_e(cpu_t *cpu){REG_E = sla(cpu, REG_E); return TWO_CYCLE;}
uint8_t sla_h(cpu_t *cpu){REG_H = sla(cpu, REG_H); return TWO_CYCLE;}
uint8_t sla_l(cpu_t *cpu){REG_L = sla(cpu, REG_L); return TWO_CYCLE;}
uint8_t sla_ind_hl(cpu_t *cpu){memoryWrite(cpu, REG_HL, sla(cpu, memoryRead(cpu, REG_HL))); return FOUR_CYCLE;}
uint8_t sra_a(cpu_t *cpu){REG_A = sra(cpu, REG_A); return TWO_CYCLE;}
uint8_t sra_b(cpu_t *cpu){REG_B = sra(cpu, REG_B); return TWO_CYCLE;}
uint8_t sra_c(cpu_t *cpu){REG_C = sra(cpu, REG_C); return TWO_CYCLE;}
uint8_t sra_d(cpu_t *cpu){REG_D = sra(cpu, REG_D); return TWO_CYCLE;}
uint8_t sra_e(cpu_t *cpu){REG_E = sra(cpu, REG_E); return TWO_CYCLE;}
uint8_t sra_h(cpu_t *cpu){REG_H = sra(cpu, REG_H); return TWO_CYCLE;}
uint8_t sra_l(cpu_t *cpu){REG_L = sra(cpu, REG_L); return TWO_CYCLE;}
uint8_t sra_ind_hl(cpu_t *cpu){memoryWrite(cpu, REG_HL, sra(cpu, memoryRead(cpu, REG_HL))); return FOUR_CYCLE;}
uint8_t swap_a(cpu_t *cpu){REG_A = swap(cpu, REG_A); return TWO_CYCLE;}
uint8_t swap_b(cpu_t *cpu){REG_B = swap(cpu, REG_B); return TWO_CYCLE;}
uint8_t swap_c(cpu_t *cpu){REG_C = swap(cpu, REG_C); return TWO_CYCLE;}
uint8_t swap_d(cpu_t *cpu){REG_D = swap(cpu, REG_D); return TWO_CYCLE;}
uint8_t swap_e(cpu_t *cpu){REG_E = swap(cpu, REG_E); return TWO_CYCLE;}
uint8_t swap_h(cpu_t *cpu){REG_H = swap(cpu, REG_H); return TWO_CYCLE;}
uint8_t swap_l(cpu_t *cpu){REG_L = swap(cpu, REG_L); return TWO_CYCLE;}
uint8_t swap_ind_hl(cpu_t *cpu){memoryWrite(cpu, REG_HL, swap(cpu, memoryRead(cpu, REG_HL))); return FOUR_CYCLE;}
uint8_t srl_a(cpu_t *cpu){REG_A = srl(cpu, REG_A); return TWO_CYCLE;}
uint8_t srl_b(cpu_t *cpu){REG_B = srl(cpu, REG_B); return TWO_CYCLE;}
uint8_t srl_c(cpu_t *cpu){REG_C = srl(cpu, REG_C); return TWO_CYCLE;}
uint8_t srl_d(cpu_t *cpu){REG_D = srl(cpu, REG_D); return TWO_CYCLE;}
uint8_t srl_e(cpu_t *cpu){REG_E = srl(cpu, REG_E); return TWO_CYCLE;}
uint8_t srl_h(cpu_t *cpu){REG_H = srl(cpu, REG_H); return TWO_CYCLE;}
uint8_t srl_l(cpu_t *cpu){REG_L = srl(cpu, REG_L); return TWO_CYCLE;}
uint8_t srl_ind_hl(cpu_t *cpu){memoryWrite(cpu, REG_HL, srl(cpu, memoryRead(cpu, REG_HL))); return FOUR_CYCLE;}
uint8_t bit_0_a(cpu_t *cpu) { bit(cpu, 0, REG_A); return TWO_CYCLE; }
uint8_t bit_0_b(cpu_t *cpu) { bit(cpu, 0, REG_B); return TWO_CYCLE; }
uint8_t bit_0_c(cpu_t *cpu) { bit(cpu, 0, REG_C); return TWO_CYCLE; }
uint8_t bit_0_d(cpu_t *cpu) { bit(cpu, 0, REG_D); return TWO_CYCLE; }
uint8_t bit_0_e(cpu_t *cpu) { bit(cpu, 0, REG_E); return TWO_CYCLE; }
uint8_t bit_0_h(cpu_t *cpu) { bit(cpu, 0, REG_H); return TWO_CYCLE; }
uint8_t bit_0_l(cpu_t *cpu) { bit(cpu, 0, REG_L); return TWO_CYCLE; }
uint8_t bit_0_ind_hl(cpu_t *cpu) { bit(cpu, 0, memoryRead(cpu, REG_HL)); return THREE_CYCLE; }
uint8_t bit_1_a(cpu_t *cpu) { bit(cpu, 1, REG_A); return TWO_CYCLE; }
uint8_t bit_1_b(cpu_t *cpu) { bit(cpu, 1, REG_B); return TWO_CYCLE; }
uint8_t bit_1_c(cpu_t *cpu) { bit(cpu, 1, REG_C); return TWO_CYCLE; }
uint8_t bit_1_d(cpu_t *cpu) { bit(cpu, 1, REG_D); return TWO_CYCLE; }
uint8_t bit_1_e(cpu_t *cpu) { bit(cpu, 1, REG_E); return TWO_CYCLE; }
uint8_t bit_1_h(cpu_t *cpu) { bit(cpu, 1, REG_H); return TWO_CYCLE; }
uint8_t bit_1_l(cpu_t *cpu) { bit(cpu, 1, REG_L); return TWO_CYCLE; }
uint8_t bit_1_ind_hl(cpu_t *cpu) { bit(cpu, 1, memoryRead(cpu, REG_HL)); return THREE_CYCLE; }
uint8_t bit_2_a(cpu_t *cpu) { bit(cpu, 2, REG_A); return TWO_CYCLE; }
uint8_t bit_2_b(cpu_t *cpu) { bit(cpu, 2, REG_B); return TWO_CYCLE; }
uint8_t bit_2_c(cpu_t *cpu) { bit(cpu, 2, REG_C); return TWO_CYCLE; }
uint8_t bit_2_d(cpu_t *cpu) { bit(cpu, 2, REG_D); return TWO_CYCLE; }
uint8_t bit_2_e(cpu_t *cpu) { bit(cpu, 2, REG_E); return TWO_CYCLE; }
uint8_t bit_2_h(cpu_t *cpu) { bit(cpu, 2, REG_H); return TWO_CYCLE; }
uint8_t bit_2_l(cpu_t *cpu) { bit(cpu, 2, REG_L); return TWO_CYCLE; }
uint8_t bit_2_ind_hl(cpu_t *cpu) { bit(cpu, 2, memoryRead(cpu, REG_HL)); return THREE_CYCLE; }
uint8_t bit_3_a(cpu_t *cpu) { bit(cpu, 3, REG_A); return TWO_CYCLE; }
uint8_t bit_3_b(cpu_t *cpu) { bit(cpu, 3, REG_B); return TWO_CYCLE; }
uint8_t bit_3_c(cpu_t *cpu) { bit(cpu, 3, REG_C); return TWO_CYCLE; }
uint8_t bit_3_d(cpu_t *cpu) { bit(cpu, 3, REG_D); return TWO_CYCLE; }
uint8_t bit_3_e(cpu_t *cpu) { bit(cpu, 3, REG_E); return TWO_CYCLE; }
uint8_t bit_3_h(cpu_t *cpu) { bit(cpu, 3, REG_H); return TWO_CYCLE; }
uint8_t bit_3_l(cpu_t *cpu) { bit(cpu, 3, REG_L); return TWO_CYCLE; }
uint8_t bit_3_ind_hl(cpu_t *cpu) { bit(cpu, 3, memoryRead(cpu, REG_HL)); return THREE_CYCLE; }
uint8_t bit_4_a(cpu_t *cpu) { bit(cpu, 4, REG_A); return TWO_CYCLE; }
uint8_t bit_4_b(cpu_t *cpu) { bit(cpu, 4, REG_B); return TWO_CYCLE; }
uint8_t bit_4_c(cpu_t *cpu) { bit(cpu, 4, REG_C); return TWO_CYCLE; }
uint8_t bit_4_d(cpu_t *cpu) { bit(cpu, 4, REG_D); return TWO_CYCLE; }
uint8_t bit_4_e(cpu_t *cpu) { bit(cpu, 4, REG_E); return TWO_CYCLE; }
uint8_t bit_4_h(cpu_t *cpu) { bit(cpu, 4, REG_H); return TWO_CYCLE; }
uint8_t bit_4_l(cpu_t *cpu) { bit(cpu, 4, REG_L); return TWO_CYCLE; }
uint8_t bit_4_ind_hl(cpu_t *cpu) { bit(cpu, 4, memoryRead(cpu, REG_HL)); return THREE_CYCLE; }
uint8_t bit_5_a(cpu_t *cpu) { bit(cpu, 5, REG_A); return TWO_CYCLE; }
uint8_t bit_5_b(cpu_t *cpu) { bit(cpu, 5, REG_B); return TWO_CYCLE; }
uint8_t bit_5_c(cpu_t *cpu) { bit(cpu, 5, REG_C); return TWO_CYCLE; }
uint8_t bit_5_d(cpu_t *cpu) { bit(cpu, 5, REG_D); return TWO_CYCLE; }
uint8_t bit_5_e(cpu_t *cpu) { bit(cpu, 5, REG_E); return TWO_CYCLE; }
uint8_t bit_5_h(cpu_t *cpu) { bit(cpu, 5, REG_H); return TWO_CYCLE; }
uint8_t bit_5_l(cpu_t *cpu) { bit(cpu, 5, REG_L); return TWO_CYCLE; }
uint8_t bit_5_ind_hl(cpu_t *cpu) { bit(cpu, 5, memoryRead(cpu, REG_HL)); return THREE_CYCLE; }
uint8_t bit_6_a(cpu_t *cpu) { bit(cpu, 6, REG_A); return TWO_CYCLE; }
uint8_t bit_6_b(cpu_t *cpu) { bit(cpu, 6, REG_B); return TWO_CYCLE; }
uint8_t bit_6_c(cpu_t *cpu) { bit(cpu, 6, REG_C); return TWO_CYCLE; }
uint8_t bit_6_d(cpu_t *cpu) { bit(cpu, 6, REG_D); return TWO_CYCLE; }
uint8_t bit_6_e(cpu_t *cpu) { bit(cpu, 6, REG_E); return TWO_CYCLE; }
uint8_t bit_6_h(cpu_t *cpu) { bit(cpu, 6, REG_H); return TWO_CYCLE; }
uint8_t bit_6_l(cpu_t *cpu) { bit(cpu, 6, REG_L); return TWO_CYCLE; }
uint8_t bit_6_ind_hl(cpu_t *cpu) { bit(cpu, 6, memoryRead(cpu, REG_HL)); return THREE_CYCLE; }
uint8_t bit_7_a(cpu_t *cpu) { bit(cpu, 7, REG_A); return TWO_CYCLE; }
uint8_t bit_7_b(cpu_t *cpu) { bit(cpu, 7, REG_B); return TWO_CYCLE; }
uint8_t bit_7_c(cpu_t *cpu) { bit(cpu, 7, REG_C); return TWO_CYCLE; }
uint8_t bit_7_d(cpu_t *cpu) { bit(cpu, 7, REG_D); return TWO_CYCLE; }
uint8_t bit_7_e(cpu_t *cpu) { bit(cpu, 7, REG_E); return TWO_CYCLE; }
uint8_t bit_7_h(cpu_t *cpu) { bit(cpu, 7, REG_H); return TWO_CYCLE; }
uint8_t bit_7_l(cpu_t *cpu) { bit(cpu, 7, REG_L); return TWO_CYCLE; }
uint8_t bit_7_ind_hl(cpu_t *cpu) { bit(cpu, 7, memoryRead(cpu, REG_HL)); return THREE_CYCLE; }
uint8_t res_0_a(cpu_t *cpu) { REG_A &= ~(1 << 0); return TWO_CYCLE; }
uint8_t res_0_b(cpu_t *cpu) { REG_B &= ~(1 << 0); return TWO_CYCLE; }
uint8_t res_0_c(cpu_t *cpu) { REG_C &= ~(1 << 0); return TWO_CYCLE; }
uint8_t res_0_d(cpu_t *cpu) { REG_D &= ~(1 << 0); return TWO_CYCLE; }
uint8_t res_0_e(cpu_t *cpu) { REG_E &= ~(1 << 0); return TWO_CYCLE; }
uint8_t res_0_h(cpu_t *cpu) { REG_H &= ~(1 << 0); return TWO_CYCLE; }
uint8_t res_0_l(cpu_t *cpu) { REG_L &= ~(1 << 0); return TWO_CYCLE; }
uint8_t res_0_ind_hl(cpu_t *cpu) { memoryWrite(cpu, REG_HL, memoryRead(cpu, REG_HL) & ~(1 << 0)); return FOUR_CYCLE; }
uint8_t res_1_a(cpu_t *cpu) { REG_A &= ~(1 << 1); return TWO_CYCLE; }
uint8_t res_1_b(cpu_t *cpu) { REG_B &= ~(1 << 1); return TWO_CYCLE; }
uint8_t res_1_c(cpu_t *cpu) { REG_C &= ~(1 << 1); return TWO_CYCLE; }
uint8_t res_1_d(cpu_t *cpu) { REG_D &= ~(1 << 1); return TWO_CYCLE; }
uint8_t res_1_e(cpu_t *cpu) { REG_E &= ~(1 << 1); return TWO_CYCLE; }
uint8_t res_1_h(cpu_t *cpu) { REG_H &= ~(1 << 1); return TWO_CYCLE; }
uint8_t res_1_l(cpu_t *cpu) { REG_L &= ~(1 << 1); return TWO_CYCLE; }
uint8_t res_1_ind_hl(cpu_t *cpu) { memoryWrite(cpu, REG_HL, memoryRead(cpu, REG_HL) & ~(1 << 1)); return FOUR_CYCLE; }
uint8_t res_2_a(cpu_t *cpu) { REG_A &= ~(1 << 2); return TWO_CYCLE; }
uint8_t res_2_b(cpu_t *cpu) { REG_B &= ~(1 << 2); return TWO_CYCLE; }
uint8_t res_2_c(cpu_t *cpu) { REG_C &= ~(1 << 2); return TWO_CYCLE; }
uint8_t res_2_d(cpu_t *cpu) { REG_D &= ~(1 << 2); return TWO_CYCLE; }
uint8_t res_2_e(cpu_t *cpu) { REG_E &= ~(1 << 2); return TWO_CYCLE; }
uint8_t res_2_h(cpu_t *cpu) { REG_H &= ~(1 << 2); return TWO_CYCLE; }
uint8_t res_2_l(cpu_t *cpu) { REG_L &= ~(1 << 2); return TWO_CYCLE; }
uint8_t res_2_ind_hl(cpu_t *cpu) { memoryWrite(cpu, REG_HL, memoryRead(cpu, REG_HL) & ~(1 << 2)); return FOUR_CYCLE; }
uint8_t res_3_a(cpu_t *cpu) { REG_A &= ~(1 << 3); return TWO_CYCLE; }
uint8_t res_3_b(cpu_t *cpu) { REG_B &= ~(1 << 3); return TWO_CYCLE; }
uint8_t res_3_c(cpu_t *cpu) { REG_C &= ~(1 << 3); return TWO_CYCLE; }
uint8_t res_3_d(cpu_t *cpu) { REG_D &= ~(1 << 3); return TWO_CYCLE; }
uint8_t res_3_e(cpu_t *cpu) { REG_E &= ~(1 << 3); return TWO_CYCLE; }
uint8_t res_3_h(cpu_t *cpu) { REG_H &= ~(1 << 3); return TWO_CYCLE; }
uint8_t res_3_l(cpu_t *cpu) { REG_L &= ~(1 << 3); return TWO_CYCLE; }
uint8_t res_3_ind_hl(cpu_t *cpu) { memoryWrite(cpu, REG_HL, memoryRead(cpu, REG_HL) & ~(1 << 3)); return FOUR_CYCLE; }
uint8_t res_4_a(cpu_t *cpu) { REG_A &= ~(1 << 4); return TWO_CYCLE; }
uint8_t res_4_b(cpu_t *cpu) { REG_B &= ~(1 << 4); return TWO_CYCLE; }
uint8_t res_4_c(cpu_t *cpu) { REG_C &= ~(1 << 4); return TWO_CYCLE; }
uint8_t res_4_d(cpu_t *cpu) { REG_D &= ~(1 << 4); return TWO_CYCLE; }
uint8_t res_4_e(cpu_t *cpu) { REG_E &= ~(1 << 4); return TWO_CYCLE; }
uint8_t res_4_h(cpu_t *cpu) { REG_H &= ~(1 << 4); return TWO_CYCLE; }
uint8_t res_4_l(cpu_t *cpu) { REG_L &= ~(1 << 4); return TWO_CYCLE; }
uint8_t res_4_ind_hl(cpu_t *cpu) { memoryWrite(cpu, REG_HL, memoryRead(cpu, REG_HL) & ~(1 << 4)); return FOUR_CYCLE; }
uint8_t res_5_a(cpu_t *cpu) { REG_A &= ~(1 << 5); return TWO_CYCLE; }
uint8_t res_5_b(cpu_t *cpu) { REG_B &= ~(1 << 5); return TWO_CYCLE; }
uint8_t res_5_c(cpu_t *cpu) { REG_C &= ~(1 << 5); return TWO_CYCLE; }
uint8_t res_5_d(cpu_t *cpu) { REG_D &= ~(1 << 5); return TWO_CYCLE; }
uint8_t res_5_e(cpu_t *cpu) { REG_E &= ~(1 << 5); return TWO_CYCLE; }
uint8_t res_5_h(cpu_t *cpu) { REG_H &= ~(1 << 5); return TWO_CYCLE; }
uint8_t res_5_l(cpu_t *cpu) { REG_L &= ~(1 << 5); return TWO_CYCLE; }
uint8_t res_5_ind_hl(cpu_t *cpu) { memoryWrite(cpu, REG_HL, memoryRead(cpu, REG_HL) & ~(1 << 5)); return FOUR_CYCLE; }
uint8_t res_6_a(cpu_t *cpu) { REG_A &= ~(1 << 6); return TWO_CYCLE; }
uint8_t res_6_b(cpu_t *cpu) { REG_B &= ~(1 << 6); return TWO_CYCLE; }
uint8_t res_6_c(cpu_t *cpu) { REG_C &= ~(1 << 6); return TWO_CYCLE; }
uint8_t res_6_d(cpu_t *cpu) { REG_D &= ~(1 << 6); return TWO_CYCLE; }
uint8_t res_6_e(cpu_t *cpu) { REG_E &= ~(1 << 6); return TWO_CYCLE; }
uint8_t res_6_h(cpu_t *cpu) { REG_H &= ~(1 << 6); return TWO_CYCLE; }
uint8_t res_6_l(cpu_t *cpu) { REG_L &= ~(1 << 6); return TWO_CYCLE; }
uint8_t res_6_ind_hl(cpu_t *cpu) { memoryWrite(cpu, REG_HL, memoryRead(cpu, REG_HL) & ~(1 << 6)); return FOUR_CYCLE; }
uint8_t res_7_a(cpu_t *cpu) { REG_A &= ~(1 << 7); return TWO_CYCLE; }
uint8_t res_7_b(cpu_t *cpu) { REG_B &= ~(1 << 7); return TWO_CYCLE; }
uint8_t res_7_c(cpu_t *cpu) { REG_C &= ~(1 << 7); return TWO_CYCLE; }
uint8_t res_7_d(cpu_t *cpu) { REG_D &= ~(1 << 7); return TWO_CYCLE; }
uint8_t res_7_e(cpu_t *cpu) { REG_E &= ~(1 << 7); return TWO_CYCLE; }
uint8_t res_7_h(cpu_t *cpu) { REG_H &= ~(1 << 7); return TWO_CYCLE; }
uint8_t res_7_l(cpu_t *cpu) { REG_L &= ~(1 << 7); return TWO_CYCLE; }
uint8_t res_7_ind_hl(cpu_t *cpu) { memoryWrite(cpu, REG_HL, memoryRead(cpu, REG_HL) & ~(1 << 7)); return FOUR_CYCLE; }
uint8_t set_0_a(cpu_t *cpu) { REG_A |= (1 << 0); return TWO_CYCLE; }
uint8_t set_0_b(cpu_t *cpu) { REG_B |= (1 << 0); return TWO_CYCLE; }
uint8_t set_0_c(cpu_t *cpu) { REG_C |= (1 << 0); return TWO_CYCLE; }
uint8_t set_0_d(cpu_t *cpu) { REG_D |= (1 << 0); return TWO_CYCLE; }
uint8_t set_0_e(cpu_t *cpu) { REG_E |= (1 << 0); return TWO_CYCLE; }
uint8_t set_0_h(cpu_t *cpu) { REG_H |= (1 << 0); return TWO_CYCLE; }
uint8_t set_0_l(cpu_t *cpu) { REG_L |= (1 << 0); return TWO_CYCLE; }
uint8_t set_0_ind_hl(cpu_t *cpu) { memoryWrite(cpu, REG_HL, memoryRead(cpu, REG_HL) | (1 << 0)); return FOUR_CYCLE; }
uint8_t set_1_a(cpu_t *cpu) { REG_A |= (1 << 1); return TWO_CYCLE; }
uint8_t set_1_b(cpu_t *cpu) { REG_B |= (1 << 1); return TWO_CYCLE; }
uint8_t set_1_c(cpu_t *cpu) { REG_C |= (1 << 1); return TWO_CYCLE; }
uint8_t set_1_d(cpu_t *cpu) { REG_D |= (1 << 1); return TWO_CYCLE; }
uint8_t set_1_e(cpu_t *cpu) { REG_E |= (1 << 1); return TWO_CYCLE; }
uint8_t set_1_h(cpu_t *cpu) { REG_H |= (1 << 1); return TWO_CYCLE; }
uint8_t set_1_l(cpu_t *cpu) { REG_L |= (1 << 1); return TWO_CYCLE; }
uint8_t set_1_ind_hl(cpu_t *cpu) { memoryWrite(cpu, REG_HL, memoryRead(cpu, REG_HL) | (1 << 1)); return FOUR_CYCLE; }
uint8_t set_2_a(cpu_t *cpu) { REG_A |= (1 << 2); return TWO_CYCLE; }
uint8_t set_2_b(cpu_t *cpu) { REG_B |= (1 << 2); return TWO_CYCLE; }
uint8_t set_2_c(cpu_t *cpu) { REG_C |= (1 << 2); return TWO_CYCLE; }
uint8_t set_2_d(cpu_t *cpu) { REG_D |= (1 << 2); return TWO_CYCLE; }
uint8_t set_2_e(cpu_t *cpu) { REG_E |= (1 << 2); return TWO_CYCLE; }
uint8_t set_2_h(cpu_t *cpu) { REG_H |= (1 << 2); return TWO_CYCLE; }
uint8_t set_2_l(cpu_t *cpu) { REG_L |= (1 << 2); return TWO_CYCLE; }
uint8_t set_2_ind_hl(cpu_t *cpu) { memoryWrite(cpu, REG_HL, memoryRead(cpu, REG_HL) | (1 << 2)); return FOUR_CYCLE; }
uint8_t set_3_a(cpu_t *cpu) { REG_A |= (1 << 3); return TWO_CYCLE; }
uint8_t set_3_b(cpu_t *cpu) { REG_B |= (1 << 3); return TWO_CYCLE; }
uint8_t set_3_c(cpu_t *cpu) { REG_C |= (1 << 3); return TWO_CYCLE; }
uint8_t set_3_d(cpu_t *cpu) { REG_D |= (1 << 3); return TWO_CYCLE; }
uint8_t set_3_e(cpu_t *cpu) { REG_E |= (1 << 3); return TWO_CYCLE; }
uint8_t set_3_h(cpu_t *cpu) { REG_H |= (1 << 3); return TWO_CYCLE; }
uint8_t set_3_l(cpu_t *cpu) { REG_L |= (1 << 3); return TWO_CYCLE; }
uint8_t set_3_ind_hl(cpu_t *cpu) { memoryWrite(cpu, REG_HL, memoryRead(cpu, REG_HL) | (1 << 3)); return FOUR_CYCLE; }
uint8_t set_4_a(cpu_t *cpu) { REG_A |= (1 << 4); return TWO_CYCLE; }
uint8_t set_4_b(cpu_t *cpu) { REG_B |= (1 << 4); return TWO_CYCLE; }
uint8_t set_4_c(cpu_t *cpu) { REG_C |= (1 << 4); return TWO_CYCLE; }
uint8_t set_4_d(cpu_t *cpu) { REG_D |= (1 << 4); return TWO_CYCLE; }
uint8_t set_4_e(cpu_t *cpu) { REG_E |= (1 << 4); return TWO_CYCLE; }
uint8_t set_4_h(cpu_t *cpu) { REG_H |= (1 << 4); return TWO_CYCLE; }
uint8_t set_4_l(cpu_t *cpu) { REG_L |= (1 << 4); return TWO_CYCLE; }
uint8_t set_4_ind_hl(cpu_t *cpu) { memoryWrite(cpu, REG_HL, memoryRead(cpu, REG_HL) | (1 << 4)); return FOUR_CYCLE; }
uint8_t set_5_a(cpu_t *cpu) { REG_A |= (1 << 5); return TWO_CYCLE; }
uint8_t set_5_b(cpu_t *cpu) { REG_B |= (1 << 5); return TWO_CYCLE; }
uint8_t set_5_c(cpu_t *cpu) { REG_C |= (1 << 5); return TWO_CYCLE; }
uint8_t set_5_d(cpu_t *cpu) { REG_D |= (1 << 5); return TWO_CYCLE; }
uint8_t set_5_e(cpu_t *cpu) { REG_E |= (1 << 5); return TWO_CYCLE; }
uint8_t set_5_h(cpu_t *cpu) { REG_H |= (1 << 5); return TWO_CYCLE; }
uint8_t set_5_l(cpu_t *cpu) { REG_L |= (1 << 5); return TWO_CYCLE; }
uint8_t set_5_ind_hl(cpu_t *cpu) { memoryWrite(cpu, REG_HL, memoryRead(cpu, REG_HL) | (1 << 5)); return FOUR_CYCLE; }
uint8_t set_6_a(cpu_t *cpu) { REG_A |= (1 << 6); return TWO_CYCLE; }
uint8_t set_6_b(cpu_t *cpu) { REG_B |= (1 << 6); return TWO_CYCLE; }
uint8_t set_6_c(cpu_t *cpu) { REG_C |= (1 << 6); return TWO_CYCLE; }
uint8_t set_6_d(cpu_t *cpu) { REG_D |= (1 << 6); return TWO_CYCLE; }
uint8_t set_6_e(cpu_t *cpu) { REG_E |= (1 << 6); return TWO_CYCLE; }
uint8_t set_6_h(cpu_t *cpu) { REG_H |= (1 << 6); return TWO_CYCLE; }
uint8_t set_6_l(cpu_t *cpu) { REG_L |= (1 << 6); return TWO_CYCLE; }
uint8_t set_6_ind_hl(cpu_t *cpu) { memoryWrite(cpu, REG_HL, memoryRead(cpu, REG_HL) | (1 << 6)); return FOUR_CYCLE; }
uint8_t set_7_a(cpu_t *cpu) { REG_A |= (1 << 7); return TWO_CYCLE; }
uint8_t set_7_b(cpu_t *cpu) { REG_B |= (1 << 7); return TWO_CYCLE; }
uint8_t set_7_c(cpu_t *cpu) { REG_C |= (1 << 7); return TWO_CYCLE; }
uint8_t set_7_d(cpu_t *cpu) { REG_D |= (1 << 7); return TWO_CYCLE; }
uint8_t set_7_e(cpu_t *cpu) { REG_E |= (1 << 7); return TWO_CYCLE; }
uint8_t set_7_h(cpu_t *cpu) { REG_H |= (1 << 7); return TWO_CYCLE; }
uint8_t set_7_l(cpu_t *cpu) { REG_L |= (1 << 7); return TWO_CYCLE; }
uint8_t set_7_ind_hl(cpu_t *cpu) { memoryWrite(cpu, REG_HL, memoryRead(cpu, REG_HL) | (1 << 7)); return FOUR_CYCLE; }
/**
 * @brief 	16bit load/store/move instructions
 */
uint8_t ld_bc_d16(cpu_t *cpu){ 
	REG_C = memoryRead(cpu, REG_PC++); // LSB
	REG_B = memoryRead(cpu, REG_PC++); // MSB
	return THREE_CYCLE;
}
uint8_t ld_de_d16(cpu_t *cpu){	
	REG_E = memoryRead(cpu, REG_PC++);
	REG_D = memoryRead(cpu, REG_PC++);
	return THREE_CYCLE;
}
uint8_t ld_hl_d16(cpu_t *cpu){	
	REG_L = memoryRead(cpu, REG_PC++);
	REG_H = memoryRead(cpu, REG_PC++);
	return THREE_CYCLE;
}
uint8_t ld_sp_d16(cpu_t *cpu){	
	REG_SP = memoryRead16(cpu, REG_PC);
	REG_PC += 2;
	return THREE_CYCLE;
}
uint8_t ld_ind_a16_sp(cpu_t *cpu){
	memoryWrite16(cpu, memoryRead16(cpu, REG_PC), REG_SP);
	REG_PC += 2;
	return FIVE_CYCLE;
}
uint8_t pop_bc(cpu_t *cpu){
	REG_C = memoryRead(cpu, REG_SP++); // LSB
	REG_B = memoryRead(cpu, REG_SP++); // MSB
	return THREE_CYCLE;
}
uint8_t pop_de(cpu_t *cpu){
	REG_E = memoryRead(cpu, REG_SP++);
	REG_D = memoryRead(cpu, REG_SP++);
	return THREE_CYCLE;
}
uint8_t pop_hl(cpu_t *cpu){
	REG_L = memoryRead(cpu, REG_SP++);
	REG_H = memoryRead(cpu, REG_SP++);
	return THREE_CYCLE;
}
uint8_t pop_af(cpu_t *cpu){
	PSW = memoryRead(cpu, REG_SP++)&0xF0;
	REG_A = memoryRead(cpu, REG_SP++);
	return THREE_CYCLE;
}
uint8_t push_bc(cpu_t *cpu){
	memoryWrite(cpu, --REG_SP, REG_B); // MSB
	memoryWrite(cpu, --REG_SP, REG_C); // LSB
	return FOUR_CYCLE;
}
uint8_t push_de(cpu_t *cpu){
	memoryWrite(cpu, --REG_SP, REG_D);
	memoryWrite(cpu, --REG_SP, REG_E);
	return FOUR_CYCLE;
}
uint8_t push_hl(cpu_t *cpu){
	memoryWrite(cpu, --REG_SP, REG_H);
	memoryWrite(cpu, --REG_SP, REG_L);
	return FOUR_CYCLE;
}
uint8_t push_af(cpu_t *cpu){
	memoryWrite(cpu, --REG_SP, REG_A);
	memoryWrite(cpu, --REG_SP, PSW & 0xf0);
	return FOUR_CYCLE;
}
uint8_t ld_hl_sp_s8(cpu_t *cpu){
	uint8_t aux = memoryRead(cpu, REG_PC++);
	PSW = 0;
	if( ((REG_SP & 0xff) + aux) > 0xFF) PSW = FC;
	if( ((REG_SP & 0x0f) + (aux & 0x0f)) > 0x0f) PSW |= FH;				
	REG_HL = REG_SP + (signed char)aux;			
	return THREE_CYCLE;
}
uint8_t ld_sp_hl(cpu_t *cpu){
	REG_SP = REG_HL;
	return TWO_CYCLE;
}
//-----------------------------------------------------
#else
//-----------------------------------------
// add to HL 16bit value
// flags: -,0,H,C
//-----------------------------------------
void add_hl_d16(cpu_t *cpu, uint16_t v)
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
// increments 8 bit register
// flags affected Z,0,H,-
//-----------------------------------------
void inc(cpu_t *cpu, uint8_t *r)
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
void dec(cpu_t *cpu, uint8_t *r)
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
void rlc(cpu_t *cpu, uint8_t *r)
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
void rrc(cpu_t *cpu, uint8_t *r)
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
void rl(cpu_t *cpu, uint8_t *r)
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
void rr(cpu_t *cpu, uint8_t *r)
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
void sla(cpu_t *cpu, uint8_t *r)
{
	PSW = (*r & (1<<7)) ? FC : 0;		
	*r = *r<<1;
    if(*r == 0) PSW |= FZ;
}
//-----------------------------------------
// shift right into carry with signal extension
// flags: Z,0,0,C  b0 to carry
//-----------------------------------------
void sra(cpu_t *cpu, uint8_t *r)
{
	PSW = (*r & (1<<0)) ? FC : 0;
	*r = (*r & 0x80) | (*r>>1);
	if(*r == 0) PSW |= FZ;	
}
//-----------------------------------------
// shift right into carry MSb = 0
// flags: Z,0,0,C   b0 to carry
//-----------------------------------------
void srl(cpu_t *cpu, uint8_t *r)
{
	PSW = (*r & (1<<0)) ? FC : 0;
	*r = (*r>>1);
	if(*r == 0) PSW |= FZ;
}
//-----------------------------------------
// swap nibles
// fags: Z,0,0,0
//-----------------------------------------
void swap(cpu_t *cpu, uint8_t *r)
{
	PSW = 0;
	*r = (*r<<4)|(*r>>4);
	if(*r == 0) PSW |= FZ;
}
//-----------------------------------------
// bit test
// flags: Z,0,1,-
//-----------------------------------------
void bit(cpu_t *cpu, uint8_t b, uint8_t *r)
{
	uint8_t aux;
	PSW &= ~(FN | FZ);
	PSW |=  FH;
	
	if(r == &PSW)
	{
		aux = memoryRead(cpu, REG_HL);
		SET_INSTR_CYCLES(ONE_CYCLE);
	}
	else
		aux = *r;
	
	if(!(aux & (1<<b)))	PSW |= FZ;	
	SET_INSTR_CYCLES(TWO_CYCLE);
}
//-----------------------------------------
// bit reset
// flags: no flags affected
//-----------------------------------------
void res(cpu_t *cpu, uint8_t b, uint8_t *r)
{
	uint8_t aux;
	
	if(r == &PSW)
	{
		aux = memoryRead(cpu, REG_HL);
		aux &= ~(1<<b);
		memoryWrite(cpu, REG_HL,aux);
		SET_INSTR_CYCLES(TWO_CYCLE);
	}
	else
		*r &= ~(1<<b);
		
	SET_INSTR_CYCLES(TWO_CYCLE);
}
//-----------------------------------------
// bit set
// flags: no flags affected
//-----------------------------------------
void set(cpu_t *cpu, uint8_t b, uint8_t *r)
{
	uint8_t aux;
	
	if(r == &PSW)
	{
		aux = memoryRead(cpu, REG_HL);
		aux |= (1<<b);
		memoryWrite(cpu, REG_HL,aux);
		SET_INSTR_CYCLES(TWO_CYCLE);
	}
	else	
		*r |= (1<<b);
		
	SET_INSTR_CYCLES(TWO_CYCLE);
}
//-----------------------------------------------------
#endif
