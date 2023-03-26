#include "dmgcpu.h"
#include "instrs.h"

enum {
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
void alu(uint8_t op, uint8_t opb)
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
uint8_t daa(void){
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
static uint8_t rlc(uint8_t r)
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
static uint8_t rrc(uint8_t r)
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
static uint8_t rl(uint8_t r)
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
static uint8_t rr(uint8_t r)
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
static uint8_t sla(uint8_t r)
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
static uint8_t sra(uint8_t r)
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
static uint8_t srl(uint8_t r)
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
static uint8_t swap(uint8_t r)
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
static void bit(uint8_t b, uint8_t r){
	PSW = (PSW & FC) | FH;
	if(!(r & (1 << b)))	PSW |= FZ;
}

/**
 * @brief Misc/control instructions
 */
uint8_t nop(void){ return ONE_CYCLE;}
uint8_t stop(void){ stopped = 1; return ONE_CYCLE; }
uint8_t scf(void){ PSW &= ~(FN | FH); PSW |= FC; return ONE_CYCLE; }
uint8_t cpl(void){ REG_A = ~REG_A; PSW |= FN | FH; return ONE_CYCLE; }
uint8_t ccf(void){ PSW &= ~( FN | FH); PSW ^= FC; return ONE_CYCLE; }
uint8_t di(void){ IME = 0; return ONE_CYCLE; }
uint8_t ei(void){ IME = 1; return ONE_CYCLE; }
uint8_t halt(void){ halt_state = HALT_ACTIVE; return ONE_CYCLE; }

/**
 * @brief 16bit arithmetic/logical instructions
 */
uint8_t inc_bc(void){ REG_BC++; return TWO_CYCLE;}
uint8_t inc_de(void){ REG_DE++; return TWO_CYCLE;}
uint8_t inc_hl(void){ REG_HL++; return TWO_CYCLE;}
uint8_t inc_sp(void){ REG_SP++; return TWO_CYCLE;}
uint8_t add_hl_bc(void){
	PSW &= ~(FN | FH| FC);
	if(((REG_HL & 0x0FFF) + (REG_BC & 0x0FFF) ) > 0x0FFF) PSW |= FH;
	if((REG_HL + REG_BC) > 0xFFFF) PSW |= FC;
	REG_HL += REG_BC;
	return TWO_CYCLE;
}
uint8_t add_hl_de(void){
	PSW &= ~(FN | FH| FC);
	if(((REG_HL & 0x0FFF) + (REG_DE & 0x0FFF) ) > 0x0FFF) PSW |= FH;
	if((REG_HL + REG_DE) > 0xFFFF) PSW |= FC;
	REG_HL += REG_DE;
	return TWO_CYCLE;
}
uint8_t add_hl_hl(void){
	PSW &= ~(FN | FH| FC);
	if(((REG_HL & 0x0FFF) + (REG_HL & 0x0FFF) ) > 0x0FFF) PSW |= FH;
	if((REG_HL + REG_HL) > 0xFFFF) PSW |= FC;
	REG_HL += REG_HL;
	return TWO_CYCLE;
}
uint8_t add_hl_sp(void){
	PSW &= ~(FN | FH| FC);
	if(((REG_HL & 0x0FFF) + (REG_SP & 0x0FFF) ) > 0x0FFF) PSW |= FH;
	if((REG_HL + REG_SP) > 0xFFFF) PSW |= FC;
	REG_HL += REG_SP;
	return TWO_CYCLE;
}
uint8_t dec_bc(void){ REG_BC--; return TWO_CYCLE; }
uint8_t dec_de(void){ REG_DE--; return TWO_CYCLE; }
uint8_t dec_hl(void){ REG_HL--; return TWO_CYCLE; }
uint8_t dec_sp(void){ REG_SP--; return TWO_CYCLE; }
uint8_t add_sp_s8(void){
	uint8_t aux = memoryRead(REG_PC++);
	PSW = 0;
	if( ((REG_SP & 0xff) + aux) > 0xFF) PSW = FC;
	if( ((REG_SP & 0x0f) + (aux & 0x0f)) > 0x0f) PSW |= FH;	
	REG_SP += (signed char)aux;
	return FOUR_CYCLE;
}
/**
 * @brief Jumps/calls
 */
uint8_t jr_s8(void){
	REG_PC = REG_PC + (signed char)memoryRead(REG_PC) + 1;
	return THREE_CYCLE;
}
uint8_t jr_nz_s8(void){	
	if (!(PSW & FZ)) {
		REG_PC = REG_PC + (signed char)memoryRead(REG_PC) + 1;
		return THREE_CYCLE;
	}else
		REG_PC++;				
	return TWO_CYCLE;
}
uint8_t jr_nc_s8(void){	
	if (!(PSW & FC)) {
		REG_PC = REG_PC + (signed char)memoryRead(REG_PC) + 1;
		return THREE_CYCLE;
	}else
		REG_PC++;				
	return TWO_CYCLE;
}
uint8_t jr_z_s8(void){	
	if (PSW & FZ) {
		REG_PC = REG_PC + (signed char)memoryRead(REG_PC) + 1;
		return THREE_CYCLE;
	}else
		REG_PC++;				
	return TWO_CYCLE;
}
uint8_t jr_c_s8(void){
	if (PSW & FC) {
		REG_PC = REG_PC + (signed char)memoryRead(REG_PC) + 1;
		return THREE_CYCLE;
	}else
		REG_PC++;				
	return TWO_CYCLE;
}
uint8_t ret_nz(void){
	if (!(PSW & FZ)) {
		POP(REG_PC);
		return FIVE_CYCLE;
	}
	return TWO_CYCLE;
}
uint8_t ret_nc(void){
	if (!(PSW & FC)) {
		POP(REG_PC);
		return FIVE_CYCLE;
	}
	return TWO_CYCLE;
}
uint8_t jp_nz_a16(void){
	if (!(PSW & FZ)) {
		REG_PC = memoryRead16(REG_PC);
		return FOUR_CYCLE;
	}
	else	
		REG_PC += 2;
	return THREE_CYCLE;
}
uint8_t jp_nc_a16(void){
	if (!(PSW & FC)) {
		REG_PC = memoryRead16(REG_PC);
		return FOUR_CYCLE;
	}
	else	
		REG_PC += 2;
	return THREE_CYCLE;
}
uint8_t jp_a16(void){REG_PC = memoryRead16(REG_PC); return FOUR_CYCLE;}
uint8_t call_nz_a16(void){
	if(!(PSW & FZ))	{
		PUSH(REG_PC + 2);
		REG_PC = memoryRead16(REG_PC);
		return SIX_CYCLE;
	}
	else
		REG_PC += 2;
	return THREE_CYCLE;
}
uint8_t call_nc_a16(void){
	if(!(PSW & FC))	{
		PUSH(REG_PC + 2);
		REG_PC = memoryRead16(REG_PC);
		return SIX_CYCLE;
	}
	else
		REG_PC += 2;
	return THREE_CYCLE;
}
uint8_t rst_0(void){PUSH(REG_PC); REG_PC = 0x00; return FOUR_CYCLE;}
uint8_t rst_1(void){PUSH(REG_PC); REG_PC = 0x08; return FOUR_CYCLE;}
uint8_t rst_2(void){PUSH(REG_PC); REG_PC = 0x10; return FOUR_CYCLE;}
uint8_t rst_3(void){PUSH(REG_PC); REG_PC = 0x18; return FOUR_CYCLE;}
uint8_t rst_4(void){PUSH(REG_PC); REG_PC = 0x20; return FOUR_CYCLE;}
uint8_t rst_5(void){PUSH(REG_PC); REG_PC = 0x28; return FOUR_CYCLE;}
uint8_t rst_6(void){PUSH(REG_PC); REG_PC = 0x30; return FOUR_CYCLE;}
uint8_t rst_7(void){PUSH(REG_PC); REG_PC = 0x38; return FOUR_CYCLE;}
uint8_t ret_z(void){
	if (PSW & FZ) {
		POP(REG_PC);
		return FIVE_CYCLE;
	}
	return TWO_CYCLE;
}
uint8_t ret_c(void){
	if (PSW & FC) {
		POP(REG_PC);
		return FIVE_CYCLE;
	}
	return TWO_CYCLE;
}
uint8_t ret(void){ POP(REG_PC); return FOUR_CYCLE;}
uint8_t reti(void){ IME = 1; POP(REG_PC); return FOUR_CYCLE;}
uint8_t jp_hl(void){REG_PC = REG_HL; return ONE_CYCLE;}
uint8_t jp_z_a16(void){
	if (PSW & FZ) {
		REG_PC = memoryRead16(REG_PC);
		return FOUR_CYCLE;
	}
	else	
		REG_PC += 2;
	return THREE_CYCLE;
}
uint8_t jp_c_a16(void){
	if (PSW & FC) {
		REG_PC = memoryRead16(REG_PC);
		return FOUR_CYCLE;
	}
	else	
		REG_PC += 2;
	return THREE_CYCLE;
}
uint8_t call_z_a16(void){
	if (PSW & FZ) {
		PUSH(REG_PC+2);
		REG_PC = memoryRead16(REG_PC);
		return SIX_CYCLE;
	}
	else	
		REG_PC += 2;
	return THREE_CYCLE;
}
uint8_t call_c_a16(void){
	if (PSW & FC) {
		PUSH(REG_PC+2);
		REG_PC = memoryRead16(REG_PC);
		return SIX_CYCLE;
	}
	else	
		REG_PC += 2;
	return THREE_CYCLE;
}
uint8_t call_a16(void){
	PUSH(REG_PC+2);
	REG_PC = memoryRead16(REG_PC);
	return SIX_CYCLE;
}
/**
 * @brief 8bit arithmetic/logical instructions
 */	
uint8_t inc_a(void){
	PSW &= ~(FZ | FN | FH);	
	if((REG_A & 0x0F) == 0x0F) PSW |= FH;
	if(++REG_A == 0) PSW |= FZ;
	return ONE_CYCLE;
}
uint8_t inc_b(void){
	PSW &= ~(FZ | FN | FH);	
	if((REG_B & 0x0F) == 0x0F) PSW |= FH;
	if(++REG_B == 0) PSW |= FZ;
	return ONE_CYCLE;
}
uint8_t inc_c(void){
	PSW &= ~(FZ | FN | FH);	
	if((REG_C & 0x0F) == 0x0F)	PSW |= FH;
	if(++REG_C == 0) PSW |= FZ;
	return ONE_CYCLE;
}
uint8_t inc_d(void){
	PSW &= ~(FZ | FN | FH);	
	if((REG_D & 0x0F) == 0x0F) PSW |= FH;
	if(++REG_D == 0) PSW |= FZ;
	return ONE_CYCLE;
}
uint8_t inc_e(void){
	PSW &= ~(FZ | FN | FH);	
	if((REG_E & 0x0F) == 0x0F) PSW |= FH;
	if(++REG_E == 0) PSW |= FZ;
	return ONE_CYCLE;
}
uint8_t inc_h(void){
	PSW &= ~(FZ | FN | FH);	
	if((REG_H & 0x0F) == 0x0F) PSW |= FH;		
	if(++REG_H == 0) PSW |= FZ;
	return ONE_CYCLE;
}
uint8_t inc_l(void){
	PSW &= ~(FZ | FN | FH);
	if((REG_L & 0x0F) == 0x0F) PSW |= FH;
	if(++REG_L == 0) PSW |= FZ;
	return ONE_CYCLE;
}
uint8_t inc_ind_hl(void){
	PSW &= ~(FZ | FN | FH);
	uint8_t aux = memoryRead(REG_HL);
	if((aux & 0x0F) == 0x0F) PSW |= FH;
	if(++aux == 0) PSW |= FZ;
	memoryWrite(REG_HL, aux);
	return THREE_CYCLE;
}
uint8_t dec_a(void) {
	PSW &= ~(FZ | FH);
	PSW |= FN;
	if(!(REG_A & 0x0f)) PSW |= FH;
    if(--REG_A == 0) PSW |= FZ;
	return ONE_CYCLE;
}
uint8_t dec_b(void) {
	PSW &= ~(FZ | FH);
	PSW |= FN;
	if(!(REG_B & 0x0f)) PSW |= FH;		
    if(--REG_B == 0) PSW |= FZ;
	return ONE_CYCLE;
}
uint8_t dec_c(void){
	PSW &= ~(FZ | FH);
	PSW |= FN;	
	if(!(REG_C & 0x0f)) PSW |= FH;		
    if(--REG_C == 0) PSW |= FZ;
	return ONE_CYCLE;
}
uint8_t dec_d(void) {
	PSW &= ~(FZ | FH);
	PSW |= FN;
	if(!(REG_D & 0x0f)) PSW |= FH;		
    if(--REG_D == 0) PSW |= FZ;
	return ONE_CYCLE;
}
uint8_t dec_e(void) {
	PSW &= ~(FZ | FH);
	PSW |= FN;
	if(!(REG_E & 0x0f)) PSW |= FH;
    if(--REG_E == 0) PSW |= FZ;
	return ONE_CYCLE;
}
uint8_t dec_h(void) {
	PSW &= ~(FZ | FH);
	PSW |= FN;
	if(!(REG_H & 0x0f)) PSW |= FH;
    if(--REG_H == 0) PSW |= FZ;
	return ONE_CYCLE;
}
uint8_t dec_l(void) {
	PSW &= ~(FZ | FH);
	PSW |= FN;
	if(!(REG_L & 0x0f)) PSW |= FH;		
    if(--REG_L == 0) PSW |= FZ;
	return ONE_CYCLE;
}
uint8_t dec_ind_hl(void){
	uint8_t aux = memoryRead(REG_HL);
	PSW &= ~(FZ | FH);
	PSW |= FN;
	if(!(aux & 0x0F)) PSW |= FH;
	if(--aux == 0) PSW |= FZ;
	memoryWrite(REG_HL, aux);
	return THREE_CYCLE;
}

uint8_t add_a_a(void){alu(ALU_ADD, REG_A); return ONE_CYCLE;}
uint8_t add_a_b(void){alu(ALU_ADD, REG_B); return ONE_CYCLE;}
uint8_t add_a_c(void){alu(ALU_ADD, REG_C); return ONE_CYCLE;}
uint8_t add_a_d(void){alu(ALU_ADD, REG_D); return ONE_CYCLE;}
uint8_t add_a_e(void){alu(ALU_ADD, REG_E); return ONE_CYCLE;}
uint8_t add_a_h(void){alu(ALU_ADD, REG_H); return ONE_CYCLE;}
uint8_t add_a_l(void){alu(ALU_ADD, REG_L); return ONE_CYCLE;}
uint8_t add_a_d8(void){alu(ALU_ADD, memoryRead(REG_PC++)); return TWO_CYCLE;}
uint8_t add_a_ind_hl(void){alu(ALU_ADD, memoryRead(REG_HL)); return TWO_CYCLE;}
uint8_t adc_a_a(void){alu(ALU_ADC, REG_A); return ONE_CYCLE;}
uint8_t adc_a_b(void){alu(ALU_ADC, REG_B); return ONE_CYCLE;}
uint8_t adc_a_c(void){alu(ALU_ADC, REG_C); return ONE_CYCLE;}
uint8_t adc_a_d(void){alu(ALU_ADC, REG_D); return ONE_CYCLE;}
uint8_t adc_a_e(void){alu(ALU_ADC, REG_E); return ONE_CYCLE;}
uint8_t adc_a_h(void){alu(ALU_ADC, REG_H); return ONE_CYCLE;}
uint8_t adc_a_l(void){alu(ALU_ADC, REG_L); return ONE_CYCLE;}
uint8_t adc_a_d8(void){alu(ALU_ADC, memoryRead(REG_PC++)); return TWO_CYCLE;}
uint8_t adc_a_ind_hl(void){alu(ALU_ADC, memoryRead(REG_HL)); return TWO_CYCLE;}
uint8_t sub_a(void){alu(ALU_SUB, REG_A); return ONE_CYCLE;}
uint8_t sub_b(void){alu(ALU_SUB, REG_B); return ONE_CYCLE;}
uint8_t sub_c(void){alu(ALU_SUB, REG_C); return ONE_CYCLE;}
uint8_t sub_d(void){alu(ALU_SUB, REG_D); return ONE_CYCLE;}
uint8_t sub_e(void){alu(ALU_SUB, REG_E); return ONE_CYCLE;}
uint8_t sub_h(void){alu(ALU_SUB, REG_H); return ONE_CYCLE;}
uint8_t sub_l(void){alu(ALU_SUB, REG_L); return ONE_CYCLE;}
uint8_t sub_d8(void){alu(ALU_SUB, memoryRead(REG_PC++)); return TWO_CYCLE;}
uint8_t sub_ind_hl(void){alu(ALU_SUB, memoryRead(REG_HL)); return TWO_CYCLE;}
uint8_t sbc_a_a(void){alu(ALU_SBC, REG_A); return ONE_CYCLE;}
uint8_t sbc_a_b(void){alu(ALU_SBC, REG_B); return ONE_CYCLE;}
uint8_t sbc_a_c(void){alu(ALU_SBC, REG_C); return ONE_CYCLE;}
uint8_t sbc_a_d(void){alu(ALU_SBC, REG_D); return ONE_CYCLE;}
uint8_t sbc_a_e(void){alu(ALU_SBC, REG_E); return ONE_CYCLE;}
uint8_t sbc_a_h(void){alu(ALU_SBC, REG_H); return ONE_CYCLE;}
uint8_t sbc_a_l(void){alu(ALU_SBC, REG_L); return ONE_CYCLE;}
uint8_t sbc_a_d8(void){alu(ALU_SBC, memoryRead(REG_PC++)); return TWO_CYCLE;}
uint8_t sbc_a_ind_hl(void){alu(ALU_SBC, memoryRead(REG_HL)); return TWO_CYCLE;}
uint8_t and_a(void){alu(ALU_AND, REG_A); return ONE_CYCLE;}
uint8_t and_b(void){alu(ALU_AND, REG_B); return ONE_CYCLE;}
uint8_t and_c(void){alu(ALU_AND, REG_C); return ONE_CYCLE;}
uint8_t and_d(void){alu(ALU_AND, REG_D); return ONE_CYCLE;}
uint8_t and_e(void){alu(ALU_AND, REG_E); return ONE_CYCLE;}
uint8_t and_h(void){alu(ALU_AND, REG_H); return ONE_CYCLE;}
uint8_t and_l(void){alu(ALU_AND, REG_L); return ONE_CYCLE;}
uint8_t and_d8(void){alu(ALU_AND, memoryRead(REG_PC++)); return TWO_CYCLE;}
uint8_t and_ind_hl(void){alu(ALU_AND, memoryRead(REG_HL)); return TWO_CYCLE;}
uint8_t xor_a(void){alu(ALU_XOR, REG_A); return ONE_CYCLE;}
uint8_t xor_b(void){alu(ALU_XOR, REG_B); return ONE_CYCLE;}
uint8_t xor_c(void){alu(ALU_XOR, REG_C); return ONE_CYCLE;}
uint8_t xor_d(void){alu(ALU_XOR, REG_D); return ONE_CYCLE;}
uint8_t xor_e(void){alu(ALU_XOR, REG_E); return ONE_CYCLE;}
uint8_t xor_h(void){alu(ALU_XOR, REG_H); return ONE_CYCLE;}
uint8_t xor_l(void){alu(ALU_XOR, REG_L); return ONE_CYCLE;}
uint8_t xor_d8(void){alu(ALU_XOR, memoryRead(REG_PC++)); return TWO_CYCLE;}
uint8_t xor_ind_hl(void){alu(ALU_XOR, memoryRead(REG_HL)); return TWO_CYCLE;}
uint8_t or_a(void){alu(ALU_OR, REG_A); return ONE_CYCLE;}
uint8_t or_b(void){alu(ALU_OR, REG_B); return ONE_CYCLE;}
uint8_t or_c(void){alu(ALU_OR, REG_C); return ONE_CYCLE;}
uint8_t or_d(void){alu(ALU_OR, REG_D); return ONE_CYCLE;}
uint8_t or_e(void){alu(ALU_OR, REG_E); return ONE_CYCLE;}
uint8_t or_h(void){alu(ALU_OR, REG_H); return ONE_CYCLE;}
uint8_t or_l(void){alu(ALU_OR, REG_L); return ONE_CYCLE;}
uint8_t or_d8(void){alu(ALU_OR, memoryRead(REG_PC++)); return TWO_CYCLE;}
uint8_t or_ind_hl(void){alu(ALU_OR, memoryRead(REG_HL)); return TWO_CYCLE;}
uint8_t cp_a(void){alu(ALU_CP, REG_A); return ONE_CYCLE;}
uint8_t cp_b(void){alu(ALU_CP, REG_B); return ONE_CYCLE;}
uint8_t cp_c(void){alu(ALU_CP, REG_C); return ONE_CYCLE;}
uint8_t cp_d(void){alu(ALU_CP, REG_D); return ONE_CYCLE;}
uint8_t cp_e(void){alu(ALU_CP, REG_E); return ONE_CYCLE;}
uint8_t cp_h(void){alu(ALU_CP, REG_H); return ONE_CYCLE;}
uint8_t cp_l(void){alu(ALU_CP, REG_L); return ONE_CYCLE;}
uint8_t cp_d8(void){alu(ALU_CP, memoryRead(REG_PC++)); return TWO_CYCLE;}
uint8_t cp_ind_hl(void){alu(ALU_CP, memoryRead(REG_HL)); return TWO_CYCLE;}

/**
 * @brief 8bit load/store/move instructions
 */
uint8_t ld_a_a(void){return ONE_CYCLE;}
uint8_t ld_a_b(void){REG_A = REG_B; return ONE_CYCLE;}
uint8_t ld_a_c(void){REG_A = REG_C; return ONE_CYCLE;}
uint8_t ld_a_d(void){REG_A = REG_D; return ONE_CYCLE;}
uint8_t ld_a_e(void){REG_A = REG_E; return ONE_CYCLE;}
uint8_t ld_a_h(void){REG_A = REG_H; return ONE_CYCLE;}
uint8_t ld_a_l(void){REG_A = REG_L; return ONE_CYCLE;}
uint8_t ld_a_d8(void){ REG_A = memoryRead(REG_PC++); return TWO_CYCLE; }
uint8_t ld_a_ind_bc(void){ REG_A = memoryRead(REG_BC); return TWO_CYCLE; }
uint8_t ld_a_ind_de(void){ REG_A = memoryRead(REG_DE); return TWO_CYCLE;}
uint8_t ld_a_ind_hl(void){REG_A = memoryRead(REG_HL); return TWO_CYCLE;}
uint8_t ld_a_ind_hli(void){REG_A = memoryRead(REG_HL++); return TWO_CYCLE;}
uint8_t ld_a_ind_hld(void){REG_A = memoryRead(REG_HL--); return TWO_CYCLE;}
uint8_t ld_a_ind_c(void){REG_A = memoryRead(0xFF00 | REG_C); return TWO_CYCLE;}
uint8_t ld_a_ind_a16(void){REG_A = memoryRead(memoryRead16(REG_PC)); REG_PC +=2; return FOUR_CYCLE;}
uint8_t ld_a_ind_a8(void){REG_A = memoryRead(0xFF00 | memoryRead(REG_PC++)); return THREE_CYCLE;}
uint8_t ld_b_a(void){REG_B = REG_A; return ONE_CYCLE;}
uint8_t ld_b_b(void){return ONE_CYCLE;}
uint8_t ld_b_c(void){REG_B = REG_C; return ONE_CYCLE;}
uint8_t ld_b_d(void){REG_B = REG_D; return ONE_CYCLE;}
uint8_t ld_b_e(void){REG_B = REG_E; return ONE_CYCLE;}
uint8_t ld_b_h(void){REG_B = REG_H; return ONE_CYCLE;}
uint8_t ld_b_l(void){REG_B = REG_L; return ONE_CYCLE;}
uint8_t ld_b_d8(void){ REG_B = memoryRead(REG_PC++); return TWO_CYCLE; }
uint8_t ld_b_ind_hl(void){REG_B = memoryRead(REG_HL); return TWO_CYCLE;}
uint8_t ld_c_a(void){REG_C = REG_A; return ONE_CYCLE;}
uint8_t ld_c_b(void){REG_C = REG_B; return ONE_CYCLE;}
uint8_t ld_c_c(void){return ONE_CYCLE;}
uint8_t ld_c_d(void){REG_C = REG_D; return ONE_CYCLE;}
uint8_t ld_c_e(void){REG_C = REG_E; return ONE_CYCLE;}
uint8_t ld_c_h(void){REG_C = REG_H; return ONE_CYCLE;}
uint8_t ld_c_l(void){REG_C = REG_L; return ONE_CYCLE;}
uint8_t ld_c_d8(void){ REG_C = memoryRead(REG_PC++); return TWO_CYCLE; }
uint8_t ld_c_ind_hl(void){REG_C = memoryRead(REG_HL); return TWO_CYCLE;}
uint8_t ld_d_a(void){REG_D = REG_A; return ONE_CYCLE;}
uint8_t ld_d_b(void){REG_D = REG_B; return ONE_CYCLE;}
uint8_t ld_d_c(void){REG_D = REG_C; return ONE_CYCLE;}
uint8_t ld_d_d(void){return ONE_CYCLE;}
uint8_t ld_d_e(void){REG_D = REG_E; return ONE_CYCLE;}
uint8_t ld_d_h(void){REG_D = REG_H; return ONE_CYCLE;}
uint8_t ld_d_l(void){REG_D = REG_L; return ONE_CYCLE;}
uint8_t ld_d_d8(void){ REG_D = memoryRead(REG_PC++); return TWO_CYCLE; }
uint8_t ld_d_ind_hl(void){REG_D = memoryRead(REG_HL); return TWO_CYCLE;}
uint8_t ld_e_a(void){REG_E = REG_A; return ONE_CYCLE;}
uint8_t ld_e_b(void){REG_E = REG_B; return ONE_CYCLE;}
uint8_t ld_e_c(void){REG_E = REG_C; return ONE_CYCLE;}
uint8_t ld_e_d(void){REG_E = REG_D; return ONE_CYCLE;}
uint8_t ld_e_e(void){return ONE_CYCLE;}
uint8_t ld_e_h(void){REG_E = REG_H; return ONE_CYCLE;}
uint8_t ld_e_l(void){REG_E = REG_L; return ONE_CYCLE;}
uint8_t ld_e_d8(void){ REG_E = memoryRead(REG_PC++); return TWO_CYCLE; }
uint8_t ld_e_ind_hl(void){REG_E = memoryRead(REG_HL); return TWO_CYCLE;}
uint8_t ld_h_a(void){REG_H = REG_A; return ONE_CYCLE;}
uint8_t ld_h_b(void){REG_H = REG_B; return ONE_CYCLE;}
uint8_t ld_h_c(void){REG_H = REG_C; return ONE_CYCLE;}
uint8_t ld_h_d(void){REG_H = REG_D; return ONE_CYCLE;}
uint8_t ld_h_e(void){REG_H = REG_E; return ONE_CYCLE;}
uint8_t ld_h_h(void){return ONE_CYCLE;}
uint8_t ld_h_l(void){REG_H = REG_L; return ONE_CYCLE;}
uint8_t ld_h_d8(void){ REG_H = memoryRead(REG_PC++); return TWO_CYCLE; }
uint8_t ld_h_ind_hl(void){REG_H = memoryRead(REG_HL); return TWO_CYCLE;}
uint8_t ld_l_a(void){REG_L = REG_A; return ONE_CYCLE;}
uint8_t ld_l_b(void){REG_L = REG_B; return ONE_CYCLE;}
uint8_t ld_l_c(void){REG_L = REG_C; return ONE_CYCLE;}
uint8_t ld_l_d(void){REG_L = REG_D; return ONE_CYCLE;}
uint8_t ld_l_e(void){REG_L = REG_E; return ONE_CYCLE;}
uint8_t ld_l_h(void){REG_L = REG_H; return ONE_CYCLE;}
uint8_t ld_l_l(void){return ONE_CYCLE;}
uint8_t ld_l_d8(void){REG_L = memoryRead(REG_PC++); return TWO_CYCLE;}
uint8_t ld_l_ind_hl(void){REG_L = memoryRead(REG_HL); return TWO_CYCLE;}
uint8_t ld_ind_hl_a(void){memoryWrite(REG_HL, REG_A); return TWO_CYCLE;}
uint8_t ld_ind_hl_b(void){memoryWrite(REG_HL, REG_B); return TWO_CYCLE;}
uint8_t ld_ind_hl_c(void){memoryWrite(REG_HL, REG_C); return TWO_CYCLE;}
uint8_t ld_ind_hl_d(void){memoryWrite(REG_HL, REG_D); return TWO_CYCLE;}
uint8_t ld_ind_hl_e(void){memoryWrite(REG_HL, REG_E); return TWO_CYCLE;}
uint8_t ld_ind_hl_h(void){memoryWrite(REG_HL, REG_H); return TWO_CYCLE;}
uint8_t ld_ind_hl_l(void){memoryWrite(REG_HL, REG_L); return TWO_CYCLE;}
uint8_t ld_ind_hl_d8(void){memoryWrite(REG_HL, memoryRead(REG_PC++)); return THREE_CYCLE;}
uint8_t ld_ind_hli_a(void){memoryWrite(REG_HL++, REG_A); return TWO_CYCLE;}
uint8_t ld_ind_hld_a(void){memoryWrite(REG_HL--, REG_A); return TWO_CYCLE;}
uint8_t ld_ind_bc_a(void){memoryWrite(REG_BC, REG_A); return TWO_CYCLE;}
uint8_t ld_ind_de_a(void){memoryWrite(REG_DE, REG_A); return TWO_CYCLE;}
uint8_t ld_ind_c_a(void){memoryWrite(0xFF00 | REG_C, REG_A); return TWO_CYCLE;}
uint8_t ld_ind_a8_a(void){memoryWrite(0xFF00 | memoryRead(REG_PC++), REG_A); return THREE_CYCLE;}
uint8_t ld_ind_a16_a(void){memoryWrite(memoryRead16(REG_PC), REG_A); REG_PC +=2; return FOUR_CYCLE;}
/**
 * @brief 8bit rotations/shifts and bit instructions
 */
uint8_t rlca(void){
	uint8_t aux = REG_A >> 7;		
	PSW = (aux == 0) ? 0 : FC;	
    REG_A = (REG_A << 1) | aux;		
	return ONE_CYCLE;
}
uint8_t rla(void){
	uint8_t aux = (PSW & FC) ? 1 : 0;
	PSW = (REG_A & (1<<7)) ? FC : 0;
	REG_A = (REG_A << 1) | aux;			
	return ONE_CYCLE;
}
uint8_t rrca(void){
	uint8_t aux = REG_A << 7;    	
	PSW = (aux & (1<<7)) ? FC : 0;
	REG_A = aux | (REG_A >> 1);    
	return ONE_CYCLE;
}
uint8_t rra(void){
	uint8_t aux = (PSW & FC) ? (1<<7) : 0;
	PSW = (REG_A & (1<<0)) ? FC : 0;
	REG_A = aux | (REG_A >> 1);
	return ONE_CYCLE;	
}

uint8_t rlc_a(void){REG_A = rlc(REG_A); return TWO_CYCLE;}
uint8_t rlc_b(void){REG_B = rlc(REG_B); return TWO_CYCLE;}
uint8_t rlc_c(void){REG_C = rlc(REG_C); return TWO_CYCLE;}
uint8_t rlc_d(void){REG_D = rlc(REG_D); return TWO_CYCLE;}
uint8_t rlc_e(void){REG_E = rlc(REG_E); return TWO_CYCLE;}
uint8_t rlc_h(void){REG_H = rlc(REG_H); return TWO_CYCLE;}
uint8_t rlc_l(void){REG_L = rlc(REG_L); return TWO_CYCLE;}
uint8_t rlc_ind_hl(void){memoryWrite(REG_HL, rlc(memoryRead(REG_HL))); return FOUR_CYCLE;}
uint8_t rrc_a(void){REG_A = rrc(REG_A); return TWO_CYCLE;}
uint8_t rrc_b(void){REG_B = rrc(REG_B); return TWO_CYCLE;}
uint8_t rrc_c(void){REG_C = rrc(REG_C); return TWO_CYCLE;}
uint8_t rrc_d(void){REG_D = rrc(REG_D); return TWO_CYCLE;}
uint8_t rrc_e(void){REG_E = rrc(REG_E); return TWO_CYCLE;}
uint8_t rrc_h(void){REG_H = rrc(REG_H); return TWO_CYCLE;}
uint8_t rrc_l(void){REG_L = rrc(REG_L); return TWO_CYCLE;}
uint8_t rrc_ind_hl(void){memoryWrite(REG_HL, rrc(memoryRead(REG_HL))); return FOUR_CYCLE;}
uint8_t rl_a(void){REG_A = rl(REG_A); return TWO_CYCLE;}
uint8_t rl_b(void){REG_B = rl(REG_B); return TWO_CYCLE;}
uint8_t rl_c(void){REG_C = rl(REG_C); return TWO_CYCLE;}
uint8_t rl_d(void){REG_D = rl(REG_D); return TWO_CYCLE;}
uint8_t rl_e(void){REG_E = rl(REG_E); return TWO_CYCLE;}
uint8_t rl_h(void){REG_H = rl(REG_H); return TWO_CYCLE;}
uint8_t rl_l(void){REG_L = rl(REG_L); return TWO_CYCLE;}
uint8_t rl_ind_hl(void){memoryWrite(REG_HL, rl(memoryRead(REG_HL))); return FOUR_CYCLE;}
uint8_t rr_a(void){REG_A = rr(REG_A); return TWO_CYCLE;}
uint8_t rr_b(void){REG_B = rr(REG_B); return TWO_CYCLE;}
uint8_t rr_c(void){REG_C = rr(REG_C); return TWO_CYCLE;}
uint8_t rr_d(void){REG_D = rr(REG_D); return TWO_CYCLE;}
uint8_t rr_e(void){REG_E = rr(REG_E); return TWO_CYCLE;}
uint8_t rr_h(void){REG_H = rr(REG_H); return TWO_CYCLE;}
uint8_t rr_l(void){REG_L = rr(REG_L); return TWO_CYCLE;}
uint8_t rr_ind_hl(void){memoryWrite(REG_HL, rr(memoryRead(REG_HL))); return FOUR_CYCLE;}
uint8_t sla_a(void){REG_A = sla(REG_A); return TWO_CYCLE;}
uint8_t sla_b(void){REG_B = sla(REG_B); return TWO_CYCLE;}
uint8_t sla_c(void){REG_C = sla(REG_C); return TWO_CYCLE;}
uint8_t sla_d(void){REG_D = sla(REG_D); return TWO_CYCLE;}
uint8_t sla_e(void){REG_E = sla(REG_E); return TWO_CYCLE;}
uint8_t sla_h(void){REG_H = sla(REG_H); return TWO_CYCLE;}
uint8_t sla_l(void){REG_L = sla(REG_L); return TWO_CYCLE;}
uint8_t sla_ind_hl(void){memoryWrite(REG_HL, sla(memoryRead(REG_HL))); return FOUR_CYCLE;}
uint8_t sra_a(void){REG_A = sra(REG_A); return TWO_CYCLE;}
uint8_t sra_b(void){REG_B = sra(REG_B); return TWO_CYCLE;}
uint8_t sra_c(void){REG_C = sra(REG_C); return TWO_CYCLE;}
uint8_t sra_d(void){REG_D = sra(REG_D); return TWO_CYCLE;}
uint8_t sra_e(void){REG_E = sra(REG_E); return TWO_CYCLE;}
uint8_t sra_h(void){REG_H = sra(REG_H); return TWO_CYCLE;}
uint8_t sra_l(void){REG_L = sra(REG_L); return TWO_CYCLE;}
uint8_t sra_ind_hl(void){memoryWrite(REG_HL, sra(memoryRead(REG_HL))); return FOUR_CYCLE;}
uint8_t swap_a(void){REG_A = swap(REG_A); return TWO_CYCLE;}
uint8_t swap_b(void){REG_B = swap(REG_B); return TWO_CYCLE;}
uint8_t swap_c(void){REG_C = swap(REG_C); return TWO_CYCLE;}
uint8_t swap_d(void){REG_D = swap(REG_D); return TWO_CYCLE;}
uint8_t swap_e(void){REG_E = swap(REG_E); return TWO_CYCLE;}
uint8_t swap_h(void){REG_H = swap(REG_H); return TWO_CYCLE;}
uint8_t swap_l(void){REG_L = swap(REG_L); return TWO_CYCLE;}
uint8_t swap_ind_hl(void){memoryWrite(REG_HL, swap(memoryRead(REG_HL))); return FOUR_CYCLE;}
uint8_t srl_a(void){REG_A = srl(REG_A); return TWO_CYCLE;}
uint8_t srl_b(void){REG_B = srl(REG_B); return TWO_CYCLE;}
uint8_t srl_c(void){REG_C = srl(REG_C); return TWO_CYCLE;}
uint8_t srl_d(void){REG_D = srl(REG_D); return TWO_CYCLE;}
uint8_t srl_e(void){REG_E = srl(REG_E); return TWO_CYCLE;}
uint8_t srl_h(void){REG_H = srl(REG_H); return TWO_CYCLE;}
uint8_t srl_l(void){REG_L = srl(REG_L); return TWO_CYCLE;}
uint8_t srl_ind_hl(void){memoryWrite(REG_HL, srl(memoryRead(REG_HL))); return FOUR_CYCLE;}
uint8_t bit_0_a(void) { bit(0, REG_A); return TWO_CYCLE; }
uint8_t bit_0_b(void) { bit(0, REG_B); return TWO_CYCLE; }
uint8_t bit_0_c(void) { bit(0, REG_C); return TWO_CYCLE; }
uint8_t bit_0_d(void) { bit(0, REG_D); return TWO_CYCLE; }
uint8_t bit_0_e(void) { bit(0, REG_E); return TWO_CYCLE; }
uint8_t bit_0_h(void) { bit(0, REG_H); return TWO_CYCLE; }
uint8_t bit_0_l(void) { bit(0, REG_L); return TWO_CYCLE; }
uint8_t bit_0_ind_hl(void) { bit(0, memoryRead(REG_HL)); return THREE_CYCLE; }
uint8_t bit_1_a(void) { bit(1, REG_A); return TWO_CYCLE; }
uint8_t bit_1_b(void) { bit(1, REG_B); return TWO_CYCLE; }
uint8_t bit_1_c(void) { bit(1, REG_C); return TWO_CYCLE; }
uint8_t bit_1_d(void) { bit(1, REG_D); return TWO_CYCLE; }
uint8_t bit_1_e(void) { bit(1, REG_E); return TWO_CYCLE; }
uint8_t bit_1_h(void) { bit(1, REG_H); return TWO_CYCLE; }
uint8_t bit_1_l(void) { bit(1, REG_L); return TWO_CYCLE; }
uint8_t bit_1_ind_hl(void) { bit(1, memoryRead(REG_HL)); return THREE_CYCLE; }
uint8_t bit_2_a(void) { bit(2, REG_A); return TWO_CYCLE; }
uint8_t bit_2_b(void) { bit(2, REG_B); return TWO_CYCLE; }
uint8_t bit_2_c(void) { bit(2, REG_C); return TWO_CYCLE; }
uint8_t bit_2_d(void) { bit(2, REG_D); return TWO_CYCLE; }
uint8_t bit_2_e(void) { bit(2, REG_E); return TWO_CYCLE; }
uint8_t bit_2_h(void) { bit(2, REG_H); return TWO_CYCLE; }
uint8_t bit_2_l(void) { bit(2, REG_L); return TWO_CYCLE; }
uint8_t bit_2_ind_hl(void) { bit(2, memoryRead(REG_HL)); return THREE_CYCLE; }
uint8_t bit_3_a(void) { bit(3, REG_A); return TWO_CYCLE; }
uint8_t bit_3_b(void) { bit(3, REG_B); return TWO_CYCLE; }
uint8_t bit_3_c(void) { bit(3, REG_C); return TWO_CYCLE; }
uint8_t bit_3_d(void) { bit(3, REG_D); return TWO_CYCLE; }
uint8_t bit_3_e(void) { bit(3, REG_E); return TWO_CYCLE; }
uint8_t bit_3_h(void) { bit(3, REG_H); return TWO_CYCLE; }
uint8_t bit_3_l(void) { bit(3, REG_L); return TWO_CYCLE; }
uint8_t bit_3_ind_hl(void) { bit(3, memoryRead(REG_HL)); return THREE_CYCLE; }
uint8_t bit_4_a(void) { bit(4, REG_A); return TWO_CYCLE; }
uint8_t bit_4_b(void) { bit(4, REG_B); return TWO_CYCLE; }
uint8_t bit_4_c(void) { bit(4, REG_C); return TWO_CYCLE; }
uint8_t bit_4_d(void) { bit(4, REG_D); return TWO_CYCLE; }
uint8_t bit_4_e(void) { bit(4, REG_E); return TWO_CYCLE; }
uint8_t bit_4_h(void) { bit(4, REG_H); return TWO_CYCLE; }
uint8_t bit_4_l(void) { bit(4, REG_L); return TWO_CYCLE; }
uint8_t bit_4_ind_hl(void) { bit(4, memoryRead(REG_HL)); return THREE_CYCLE; }
uint8_t bit_5_a(void) { bit(5, REG_A); return TWO_CYCLE; }
uint8_t bit_5_b(void) { bit(5, REG_B); return TWO_CYCLE; }
uint8_t bit_5_c(void) { bit(5, REG_C); return TWO_CYCLE; }
uint8_t bit_5_d(void) { bit(5, REG_D); return TWO_CYCLE; }
uint8_t bit_5_e(void) { bit(5, REG_E); return TWO_CYCLE; }
uint8_t bit_5_h(void) { bit(5, REG_H); return TWO_CYCLE; }
uint8_t bit_5_l(void) { bit(5, REG_L); return TWO_CYCLE; }
uint8_t bit_5_ind_hl(void) { bit(5, memoryRead(REG_HL)); return THREE_CYCLE; }
uint8_t bit_6_a(void) { bit(6, REG_A); return TWO_CYCLE; }
uint8_t bit_6_b(void) { bit(6, REG_B); return TWO_CYCLE; }
uint8_t bit_6_c(void) { bit(6, REG_C); return TWO_CYCLE; }
uint8_t bit_6_d(void) { bit(6, REG_D); return TWO_CYCLE; }
uint8_t bit_6_e(void) { bit(6, REG_E); return TWO_CYCLE; }
uint8_t bit_6_h(void) { bit(6, REG_H); return TWO_CYCLE; }
uint8_t bit_6_l(void) { bit(6, REG_L); return TWO_CYCLE; }
uint8_t bit_6_ind_hl(void) { bit(6, memoryRead(REG_HL)); return THREE_CYCLE; }
uint8_t bit_7_a(void) { bit(7, REG_A); return TWO_CYCLE; }
uint8_t bit_7_b(void) { bit(7, REG_B); return TWO_CYCLE; }
uint8_t bit_7_c(void) { bit(7, REG_C); return TWO_CYCLE; }
uint8_t bit_7_d(void) { bit(7, REG_D); return TWO_CYCLE; }
uint8_t bit_7_e(void) { bit(7, REG_E); return TWO_CYCLE; }
uint8_t bit_7_h(void) { bit(7, REG_H); return TWO_CYCLE; }
uint8_t bit_7_l(void) { bit(7, REG_L); return TWO_CYCLE; }
uint8_t bit_7_ind_hl(void) { bit(7, memoryRead(REG_HL)); return THREE_CYCLE; }
uint8_t res_0_a(void) { REG_A &= ~(1 << 0); return TWO_CYCLE; }
uint8_t res_0_b(void) { REG_B &= ~(1 << 0); return TWO_CYCLE; }
uint8_t res_0_c(void) { REG_C &= ~(1 << 0); return TWO_CYCLE; }
uint8_t res_0_d(void) { REG_D &= ~(1 << 0); return TWO_CYCLE; }
uint8_t res_0_e(void) { REG_E &= ~(1 << 0); return TWO_CYCLE; }
uint8_t res_0_h(void) { REG_H &= ~(1 << 0); return TWO_CYCLE; }
uint8_t res_0_l(void) { REG_L &= ~(1 << 0); return TWO_CYCLE; }
uint8_t res_0_ind_hl(void) { memoryWrite(REG_HL, memoryRead(REG_HL) & ~(1 << 0)); return FOUR_CYCLE; }
uint8_t res_1_a(void) { REG_A &= ~(1 << 1); return TWO_CYCLE; }
uint8_t res_1_b(void) { REG_B &= ~(1 << 1); return TWO_CYCLE; }
uint8_t res_1_c(void) { REG_C &= ~(1 << 1); return TWO_CYCLE; }
uint8_t res_1_d(void) { REG_D &= ~(1 << 1); return TWO_CYCLE; }
uint8_t res_1_e(void) { REG_E &= ~(1 << 1); return TWO_CYCLE; }
uint8_t res_1_h(void) { REG_H &= ~(1 << 1); return TWO_CYCLE; }
uint8_t res_1_l(void) { REG_L &= ~(1 << 1); return TWO_CYCLE; }
uint8_t res_1_ind_hl(void) { memoryWrite(REG_HL, memoryRead(REG_HL) & ~(1 << 1)); return FOUR_CYCLE; }
uint8_t res_2_a(void) { REG_A &= ~(1 << 2); return TWO_CYCLE; }
uint8_t res_2_b(void) { REG_B &= ~(1 << 2); return TWO_CYCLE; }
uint8_t res_2_c(void) { REG_C &= ~(1 << 2); return TWO_CYCLE; }
uint8_t res_2_d(void) { REG_D &= ~(1 << 2); return TWO_CYCLE; }
uint8_t res_2_e(void) { REG_E &= ~(1 << 2); return TWO_CYCLE; }
uint8_t res_2_h(void) { REG_H &= ~(1 << 2); return TWO_CYCLE; }
uint8_t res_2_l(void) { REG_L &= ~(1 << 2); return TWO_CYCLE; }
uint8_t res_2_ind_hl(void) { memoryWrite(REG_HL, memoryRead(REG_HL) & ~(1 << 2)); return FOUR_CYCLE; }
uint8_t res_3_a(void) { REG_A &= ~(1 << 3); return TWO_CYCLE; }
uint8_t res_3_b(void) { REG_B &= ~(1 << 3); return TWO_CYCLE; }
uint8_t res_3_c(void) { REG_C &= ~(1 << 3); return TWO_CYCLE; }
uint8_t res_3_d(void) { REG_D &= ~(1 << 3); return TWO_CYCLE; }
uint8_t res_3_e(void) { REG_E &= ~(1 << 3); return TWO_CYCLE; }
uint8_t res_3_h(void) { REG_H &= ~(1 << 3); return TWO_CYCLE; }
uint8_t res_3_l(void) { REG_L &= ~(1 << 3); return TWO_CYCLE; }
uint8_t res_3_ind_hl(void) { memoryWrite(REG_HL, memoryRead(REG_HL) & ~(1 << 3)); return FOUR_CYCLE; }
uint8_t res_4_a(void) { REG_A &= ~(1 << 4); return TWO_CYCLE; }
uint8_t res_4_b(void) { REG_B &= ~(1 << 4); return TWO_CYCLE; }
uint8_t res_4_c(void) { REG_C &= ~(1 << 4); return TWO_CYCLE; }
uint8_t res_4_d(void) { REG_D &= ~(1 << 4); return TWO_CYCLE; }
uint8_t res_4_e(void) { REG_E &= ~(1 << 4); return TWO_CYCLE; }
uint8_t res_4_h(void) { REG_H &= ~(1 << 4); return TWO_CYCLE; }
uint8_t res_4_l(void) { REG_L &= ~(1 << 4); return TWO_CYCLE; }
uint8_t res_4_ind_hl(void) { memoryWrite(REG_HL, memoryRead(REG_HL) & ~(1 << 4)); return FOUR_CYCLE; }
uint8_t res_5_a(void) { REG_A &= ~(1 << 5); return TWO_CYCLE; }
uint8_t res_5_b(void) { REG_B &= ~(1 << 5); return TWO_CYCLE; }
uint8_t res_5_c(void) { REG_C &= ~(1 << 5); return TWO_CYCLE; }
uint8_t res_5_d(void) { REG_D &= ~(1 << 5); return TWO_CYCLE; }
uint8_t res_5_e(void) { REG_E &= ~(1 << 5); return TWO_CYCLE; }
uint8_t res_5_h(void) { REG_H &= ~(1 << 5); return TWO_CYCLE; }
uint8_t res_5_l(void) { REG_L &= ~(1 << 5); return TWO_CYCLE; }
uint8_t res_5_ind_hl(void) { memoryWrite(REG_HL, memoryRead(REG_HL) & ~(1 << 5)); return FOUR_CYCLE; }
uint8_t res_6_a(void) { REG_A &= ~(1 << 6); return TWO_CYCLE; }
uint8_t res_6_b(void) { REG_B &= ~(1 << 6); return TWO_CYCLE; }
uint8_t res_6_c(void) { REG_C &= ~(1 << 6); return TWO_CYCLE; }
uint8_t res_6_d(void) { REG_D &= ~(1 << 6); return TWO_CYCLE; }
uint8_t res_6_e(void) { REG_E &= ~(1 << 6); return TWO_CYCLE; }
uint8_t res_6_h(void) { REG_H &= ~(1 << 6); return TWO_CYCLE; }
uint8_t res_6_l(void) { REG_L &= ~(1 << 6); return TWO_CYCLE; }
uint8_t res_6_ind_hl(void) { memoryWrite(REG_HL, memoryRead(REG_HL) & ~(1 << 6)); return FOUR_CYCLE; }
uint8_t res_7_a(void) { REG_A &= ~(1 << 7); return TWO_CYCLE; }
uint8_t res_7_b(void) { REG_B &= ~(1 << 7); return TWO_CYCLE; }
uint8_t res_7_c(void) { REG_C &= ~(1 << 7); return TWO_CYCLE; }
uint8_t res_7_d(void) { REG_D &= ~(1 << 7); return TWO_CYCLE; }
uint8_t res_7_e(void) { REG_E &= ~(1 << 7); return TWO_CYCLE; }
uint8_t res_7_h(void) { REG_H &= ~(1 << 7); return TWO_CYCLE; }
uint8_t res_7_l(void) { REG_L &= ~(1 << 7); return TWO_CYCLE; }
uint8_t res_7_ind_hl(void) { memoryWrite(REG_HL, memoryRead(REG_HL) & ~(1 << 7)); return FOUR_CYCLE; }
uint8_t set_0_a(void) { REG_A |= (1 << 0); return TWO_CYCLE; }
uint8_t set_0_b(void) { REG_B |= (1 << 0); return TWO_CYCLE; }
uint8_t set_0_c(void) { REG_C |= (1 << 0); return TWO_CYCLE; }
uint8_t set_0_d(void) { REG_D |= (1 << 0); return TWO_CYCLE; }
uint8_t set_0_e(void) { REG_E |= (1 << 0); return TWO_CYCLE; }
uint8_t set_0_h(void) { REG_H |= (1 << 0); return TWO_CYCLE; }
uint8_t set_0_l(void) { REG_L |= (1 << 0); return TWO_CYCLE; }
uint8_t set_0_ind_hl(void) { memoryWrite(REG_HL, memoryRead(REG_HL) | (1 << 0)); return FOUR_CYCLE; }
uint8_t set_1_a(void) { REG_A |= (1 << 1); return TWO_CYCLE; }
uint8_t set_1_b(void) { REG_B |= (1 << 1); return TWO_CYCLE; }
uint8_t set_1_c(void) { REG_C |= (1 << 1); return TWO_CYCLE; }
uint8_t set_1_d(void) { REG_D |= (1 << 1); return TWO_CYCLE; }
uint8_t set_1_e(void) { REG_E |= (1 << 1); return TWO_CYCLE; }
uint8_t set_1_h(void) { REG_H |= (1 << 1); return TWO_CYCLE; }
uint8_t set_1_l(void) { REG_L |= (1 << 1); return TWO_CYCLE; }
uint8_t set_1_ind_hl(void) { memoryWrite(REG_HL, memoryRead(REG_HL) | (1 << 1)); return FOUR_CYCLE; }
uint8_t set_2_a(void) { REG_A |= (1 << 2); return TWO_CYCLE; }
uint8_t set_2_b(void) { REG_B |= (1 << 2); return TWO_CYCLE; }
uint8_t set_2_c(void) { REG_C |= (1 << 2); return TWO_CYCLE; }
uint8_t set_2_d(void) { REG_D |= (1 << 2); return TWO_CYCLE; }
uint8_t set_2_e(void) { REG_E |= (1 << 2); return TWO_CYCLE; }
uint8_t set_2_h(void) { REG_H |= (1 << 2); return TWO_CYCLE; }
uint8_t set_2_l(void) { REG_L |= (1 << 2); return TWO_CYCLE; }
uint8_t set_2_ind_hl(void) { memoryWrite(REG_HL, memoryRead(REG_HL) | (1 << 2)); return FOUR_CYCLE; }
uint8_t set_3_a(void) { REG_A |= (1 << 3); return TWO_CYCLE; }
uint8_t set_3_b(void) { REG_B |= (1 << 3); return TWO_CYCLE; }
uint8_t set_3_c(void) { REG_C |= (1 << 3); return TWO_CYCLE; }
uint8_t set_3_d(void) { REG_D |= (1 << 3); return TWO_CYCLE; }
uint8_t set_3_e(void) { REG_E |= (1 << 3); return TWO_CYCLE; }
uint8_t set_3_h(void) { REG_H |= (1 << 3); return TWO_CYCLE; }
uint8_t set_3_l(void) { REG_L |= (1 << 3); return TWO_CYCLE; }
uint8_t set_3_ind_hl(void) { memoryWrite(REG_HL, memoryRead(REG_HL) | (1 << 3)); return FOUR_CYCLE; }
uint8_t set_4_a(void) { REG_A |= (1 << 4); return TWO_CYCLE; }
uint8_t set_4_b(void) { REG_B |= (1 << 4); return TWO_CYCLE; }
uint8_t set_4_c(void) { REG_C |= (1 << 4); return TWO_CYCLE; }
uint8_t set_4_d(void) { REG_D |= (1 << 4); return TWO_CYCLE; }
uint8_t set_4_e(void) { REG_E |= (1 << 4); return TWO_CYCLE; }
uint8_t set_4_h(void) { REG_H |= (1 << 4); return TWO_CYCLE; }
uint8_t set_4_l(void) { REG_L |= (1 << 4); return TWO_CYCLE; }
uint8_t set_4_ind_hl(void) { memoryWrite(REG_HL, memoryRead(REG_HL) | (1 << 4)); return FOUR_CYCLE; }
uint8_t set_5_a(void) { REG_A |= (1 << 5); return TWO_CYCLE; }
uint8_t set_5_b(void) { REG_B |= (1 << 5); return TWO_CYCLE; }
uint8_t set_5_c(void) { REG_C |= (1 << 5); return TWO_CYCLE; }
uint8_t set_5_d(void) { REG_D |= (1 << 5); return TWO_CYCLE; }
uint8_t set_5_e(void) { REG_E |= (1 << 5); return TWO_CYCLE; }
uint8_t set_5_h(void) { REG_H |= (1 << 5); return TWO_CYCLE; }
uint8_t set_5_l(void) { REG_L |= (1 << 5); return TWO_CYCLE; }
uint8_t set_5_ind_hl(void) { memoryWrite(REG_HL, memoryRead(REG_HL) | (1 << 5)); return FOUR_CYCLE; }
uint8_t set_6_a(void) { REG_A |= (1 << 6); return TWO_CYCLE; }
uint8_t set_6_b(void) { REG_B |= (1 << 6); return TWO_CYCLE; }
uint8_t set_6_c(void) { REG_C |= (1 << 6); return TWO_CYCLE; }
uint8_t set_6_d(void) { REG_D |= (1 << 6); return TWO_CYCLE; }
uint8_t set_6_e(void) { REG_E |= (1 << 6); return TWO_CYCLE; }
uint8_t set_6_h(void) { REG_H |= (1 << 6); return TWO_CYCLE; }
uint8_t set_6_l(void) { REG_L |= (1 << 6); return TWO_CYCLE; }
uint8_t set_6_ind_hl(void) { memoryWrite(REG_HL, memoryRead(REG_HL) | (1 << 6)); return FOUR_CYCLE; }
uint8_t set_7_a(void) { REG_A |= (1 << 7); return TWO_CYCLE; }
uint8_t set_7_b(void) { REG_B |= (1 << 7); return TWO_CYCLE; }
uint8_t set_7_c(void) { REG_C |= (1 << 7); return TWO_CYCLE; }
uint8_t set_7_d(void) { REG_D |= (1 << 7); return TWO_CYCLE; }
uint8_t set_7_e(void) { REG_E |= (1 << 7); return TWO_CYCLE; }
uint8_t set_7_h(void) { REG_H |= (1 << 7); return TWO_CYCLE; }
uint8_t set_7_l(void) { REG_L |= (1 << 7); return TWO_CYCLE; }
uint8_t set_7_ind_hl(void) { memoryWrite(REG_HL, memoryRead(REG_HL) | (1 << 7)); return FOUR_CYCLE; }
/**
 * @brief 	16bit load/store/move instructions
 */
uint8_t ld_bc_d16(void){ 
	REG_C = memoryRead(REG_PC++); // LSB
	REG_B = memoryRead(REG_PC++); // MSB
	return THREE_CYCLE;
}
uint8_t ld_de_d16(void){	
	REG_E = memoryRead(REG_PC++);
	REG_D = memoryRead(REG_PC++);
	return THREE_CYCLE;
}
uint8_t ld_hl_d16(void){	
	REG_L = memoryRead(REG_PC++);
	REG_H = memoryRead(REG_PC++);
	return THREE_CYCLE;
}
uint8_t ld_sp_d16(void){	
	REG_SP = memoryRead16(REG_PC);
	REG_PC += 2;
	return THREE_CYCLE;
}
uint8_t ld_ind_a16_sp(void){
	memoryWrite16(memoryRead16(REG_PC), REG_SP);
	REG_PC += 2;
	return FIVE_CYCLE;
}
uint8_t pop_bc(void){
	REG_C = memoryRead(REG_SP++); // LSB
	REG_B = memoryRead(REG_SP++); // MSB
	return THREE_CYCLE;
}
uint8_t pop_de(void){
	REG_E = memoryRead(REG_SP++);
	REG_D = memoryRead(REG_SP++);
	return THREE_CYCLE;
}
uint8_t pop_hl(void){
	REG_L = memoryRead(REG_SP++);
	REG_H = memoryRead(REG_SP++);
	return THREE_CYCLE;
}
uint8_t pop_af(void){
	PSW = memoryRead(REG_SP++)&0xF0;
	REG_A = memoryRead(REG_SP++);
	return THREE_CYCLE;
}
uint8_t push_bc(void){
	memoryWrite(--REG_SP, REG_B); // MSB
	memoryWrite(--REG_SP, REG_C); // LSB
	return FOUR_CYCLE;
}
uint8_t push_de(void){
	memoryWrite(--REG_SP, REG_D);
	memoryWrite(--REG_SP, REG_E);
	return FOUR_CYCLE;
}
uint8_t push_hl(void){
	memoryWrite(--REG_SP, REG_H);
	memoryWrite(--REG_SP, REG_L);
	return FOUR_CYCLE;
}
uint8_t push_af(void){
	memoryWrite(--REG_SP, REG_A);
	memoryWrite(--REG_SP, REG_F & 0xf0);
	return FOUR_CYCLE;
}
uint8_t ld_hl_sp_s8(void){
	uint8_t aux = memoryRead(REG_PC++);
	REG_F = 0;
	if( ((REG_SP & 0xff) + aux) > 0xFF) PSW = FC;
	if( ((REG_SP & 0x0f) + (aux & 0x0f)) > 0x0f) PSW |= FH;				
	REG_HL = REG_SP + (signed char)aux;			
	return THREE_CYCLE;
}
uint8_t ld_sp_hl(void){
	REG_SP = REG_HL;
	return TWO_CYCLE;
}
//-----------------------------------------------------
#else
//-----------------------------------------
// add to HL 16bit value
// flags: -,0,H,C
//-----------------------------------------
void add_hl_d16(uint16_t v)
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
void bit(uint8_t b, uint8_t *r)
{
uint8_t aux;
	PSW &= ~(FN | FZ);
	PSW |=  FH;
	
	if(r == &PSW)
	{
		aux = memoryRead(REG_HL);
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
void res(uint8_t b, uint8_t *r)
{
uint8_t aux;
	
	if(r == &PSW)
	{
		aux = memoryRead(REG_HL);
		aux &= ~(1<<b);
		memoryWrite(REG_HL,aux);
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
void set(uint8_t b, uint8_t *r)
{
uint8_t aux;
	
	if(r == &PSW)
	{
		aux = memoryRead(REG_HL);
		aux |= (1<<b);
		memoryWrite(REG_HL,aux);
		SET_INSTR_CYCLES(TWO_CYCLE);
	}
	else	
		*r |= (1<<b);
		
	SET_INSTR_CYCLES(TWO_CYCLE);
}
//-----------------------------------------------------
#endif
