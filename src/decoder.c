#include "dmgcpu.h"
#include "decoder.h"
#include "instrs.h"

static uint8_t cb_decode(cpu_t*);

#if TABLE_DECODER
#define NOP nop

static const uint8_t (*cb_opcodes[])(cpu_t*) = {
	rlc_b, rlc_c, rlc_d, rlc_e, rlc_h, rlc_l, rlc_ind_hl, rlc_a, rrc_b, rrc_c, rrc_d, rrc_e, rrc_h, rrc_l, rrc_ind_hl, rrc_a,
	rl_b, rl_c, rl_d, rl_e, rl_h, rl_l, rl_ind_hl, rl_a, rr_b, rr_c, rr_d, rr_e, rr_h, rr_l, rr_ind_hl, rr_a,
	sla_b, sla_c, sla_d, sla_e, sla_h, sla_l, sla_ind_hl, sla_a, sra_b, sra_c, sra_d, sra_e, sra_h, sra_l, sra_ind_hl, sra_a,
	swap_b, swap_c, swap_d, swap_e, swap_h, swap_l, swap_ind_hl, swap_a, srl_b, srl_c, srl_d, srl_e, srl_h, srl_l, srl_ind_hl, srl_a,
	bit_0_b, bit_0_c, bit_0_d, bit_0_e, bit_0_h, bit_0_l, bit_0_ind_hl, bit_0_a, bit_1_b, bit_1_c,bit_1_d, bit_1_e, bit_1_h, bit_1_l, bit_1_ind_hl, bit_1_a,
	bit_2_b, bit_2_c, bit_2_d, bit_2_e, bit_2_h, bit_2_l, bit_2_ind_hl, bit_2_a, bit_3_b, bit_3_c,bit_3_d, bit_3_e, bit_3_h, bit_3_l, bit_3_ind_hl, bit_3_a,
	bit_4_b, bit_4_c, bit_4_d, bit_4_e, bit_4_h, bit_4_l, bit_4_ind_hl, bit_4_a, bit_5_b, bit_5_c,bit_5_d, bit_5_e, bit_5_h, bit_5_l, bit_5_ind_hl, bit_5_a,
	bit_6_b, bit_6_c, bit_6_d, bit_6_e, bit_6_h, bit_6_l, bit_6_ind_hl, bit_6_a, bit_7_b, bit_7_c,bit_7_d, bit_7_e, bit_7_h, bit_7_l, bit_7_ind_hl, bit_7_a,
	res_0_b, res_0_c, res_0_d, res_0_e, res_0_h, res_0_l, res_0_ind_hl, res_0_a, res_1_b, res_1_c,res_1_d, res_1_e, res_1_h, res_1_l, res_1_ind_hl, res_1_a,
	res_2_b, res_2_c, res_2_d, res_2_e, res_2_h, res_2_l, res_2_ind_hl, res_2_a, res_3_b, res_3_c,res_3_d, res_3_e, res_3_h, res_3_l, res_3_ind_hl, res_3_a,
	res_4_b, res_4_c, res_4_d, res_4_e, res_4_h, res_4_l, res_4_ind_hl, res_4_a, res_5_b, res_5_c,res_5_d, res_5_e, res_5_h, res_5_l, res_5_ind_hl, res_5_a,
	res_6_b, res_6_c, res_6_d, res_6_e, res_6_h, res_6_l, res_6_ind_hl, res_6_a, res_7_b, res_7_c,res_7_d, res_7_e, res_7_h, res_7_l, res_7_ind_hl, res_7_a,
	set_0_b, set_0_c, set_0_d, set_0_e, set_0_h, set_0_l, set_0_ind_hl, set_0_a, set_1_b, set_1_c,set_1_d, set_1_e, set_1_h, set_1_l, set_1_ind_hl, set_1_a,
	set_2_b, set_2_c, set_2_d, set_2_e, set_2_h, set_2_l, set_2_ind_hl, set_2_a, set_3_b, set_3_c,set_3_d, set_3_e, set_3_h, set_3_l, set_3_ind_hl, set_3_a,
	set_4_b, set_4_c, set_4_d, set_4_e, set_4_h, set_4_l, set_4_ind_hl, set_4_a, set_5_b, set_5_c,set_5_d, set_5_e, set_5_h, set_5_l, set_5_ind_hl, set_5_a,
	set_6_b, set_6_c, set_6_d, set_6_e, set_6_h, set_6_l, set_6_ind_hl, set_6_a, set_7_b, set_7_c,set_7_d, set_7_e, set_7_h, set_7_l, set_7_ind_hl, set_7_a,
};


static const uint8_t (*opcodes[])(cpu_t*) = {
	nop, ld_bc_d16, ld_ind_bc_a, inc_bc, inc_b, dec_b, ld_b_d8, rlca, ld_ind_a16_sp, add_hl_bc, ld_a_ind_bc, dec_bc, inc_c, dec_c, ld_c_d8, rrca,						  // 0x00
	stop, ld_de_d16, ld_ind_de_a, inc_de, inc_d, dec_d, ld_d_d8, rla, jr_s8, add_hl_de, ld_a_ind_de, dec_de, inc_e, dec_e, ld_e_d8, rra,								  // 0x10
	jr_nz_s8, ld_hl_d16, ld_ind_hli_a, inc_hl, inc_h, dec_h, ld_h_d8, daa, jr_z_s8, add_hl_hl, ld_a_ind_hli, dec_hl, inc_l, dec_l, ld_l_d8, cpl,						  // 0x20
	jr_nc_s8, ld_sp_d16, ld_ind_hld_a, inc_sp, inc_ind_hl, dec_ind_hl, ld_ind_hl_d8, scf, jr_c_s8, add_hl_sp, ld_a_ind_hld, dec_sp, inc_a, dec_a, ld_a_d8, ccf,			  // 0x30
	ld_b_b, ld_b_c, ld_b_d, ld_b_e, ld_b_h, ld_b_l, ld_b_ind_hl, ld_b_a, ld_c_b, ld_c_c, ld_c_d, ld_c_e, ld_c_h, ld_c_l, ld_c_ind_hl, ld_c_a,							  // 0x40
	ld_d_b, ld_d_c, ld_d_d, ld_d_e, ld_d_h, ld_d_l, ld_d_ind_hl, ld_d_a, ld_e_b, ld_e_c, ld_e_d, ld_e_e, ld_e_h, ld_e_l, ld_e_ind_hl, ld_e_a,							  // 0x50
	ld_h_b, ld_h_c, ld_h_d, ld_h_e, ld_h_h, ld_h_l, ld_h_ind_hl, ld_h_a, ld_l_b, ld_l_c, ld_l_d, ld_l_e, ld_l_h, ld_l_l, ld_l_ind_hl, ld_l_a,							  // 0x60
	ld_ind_hl_b, ld_ind_hl_c, ld_ind_hl_d, ld_ind_hl_e, ld_ind_hl_h, ld_ind_hl_l, halt, ld_ind_hl_a, ld_a_b, ld_a_c, ld_a_d, ld_a_e, ld_a_h, ld_a_l, ld_a_ind_hl, ld_a_a, // 0x70
	add_a_b, add_a_c, add_a_d, add_a_e, add_a_h, add_a_l, add_a_ind_hl, add_a_a, adc_a_b, adc_a_c, adc_a_d, adc_a_e, adc_a_h, adc_a_l, adc_a_ind_hl, adc_a_a,			  // 0x80
	sub_b, sub_c, sub_d, sub_e, sub_h, sub_l, sub_ind_hl, sub_a, sbc_a_b, sbc_a_c, sbc_a_d, sbc_a_e, sbc_a_h, sbc_a_l, sbc_a_ind_hl, sbc_a_a,							  // 0x90
	and_b, and_c, and_d, and_e, and_h, and_l, and_ind_hl, and_a, xor_b, xor_c, xor_d, xor_e, xor_h, xor_l, xor_ind_hl, xor_a,											  // 0xa0
	or_b, or_c, or_d, or_e, or_h, or_l, or_ind_hl, or_a, cp_b, cp_c, cp_d, cp_e, cp_h, cp_l, cp_ind_hl, cp_a,															  // 0xb0
	ret_nz, pop_bc, jp_nz_a16, jp_a16, call_nz_a16, push_bc, add_a_d8, rst_0, ret_z, ret, jp_z_a16, cb_decode, call_z_a16, call_a16, adc_a_d8, rst_1,					  // 0xc0
	ret_nc, pop_de, jp_nc_a16, NOP, call_nc_a16, push_de, sub_d8, rst_2, ret_c, reti, jp_c_a16, NOP, call_c_a16, NOP, sbc_a_d8, rst_3,									  // 0xd0
	ld_ind_a8_a, pop_hl, ld_ind_c_a, NOP, NOP, push_hl, and_d8, rst_4, add_sp_s8, jp_hl, ld_ind_a16_a, NOP, NOP, NOP, xor_d8, rst_5,									  // 0xe0
	ld_a_ind_a8, pop_af, ld_a_ind_c, di, NOP, push_af, or_d8, rst_6, ld_hl_sp_s8, ld_sp_hl, ld_a_ind_a16, ei, NOP, NOP, cp_d8, rst_7									  // 0xf0
};

void decode(cpu_t *cpu)
{
	if (cpu->halt == 1) {	
		cpu->instr_cycles = ONE_CYCLE;
		return;			
	}

	uint8_t opcode = memoryRead(cpu, cpu->PC++);
	cpu->instr_cycles = opcodes[opcode](cpu);
}

static uint8_t cb_decode(cpu_t *cpu)
{ 
	return cb_opcodes[memoryRead(cpu, cpu->PC++)](cpu);
}
#else

//-----------------------------------------
// Decoder that uses opcode bits to decode 
// instruction to be executed.
// The only gain is code size
//-----------------------------------------
void decode(cpu_t *cpu)
{
uint8_t dst;
uint8_t aux;
uint8_t opcode;
uint16_t aux16;

	cpu->instr_cycles = 0;

	if(cpu->halt == 1){		
		SET_INSTR_CYCLES(ONE_CYCLE);
		return;
	}
    
    opcode = memoryRead(cpu, REG_PC++);

	switch(opcode)
	{
		case 0x00: // NOP
        case 0xD3:
        case 0xDB:
        case 0xDD:
        case 0xE3:
        case 0xE4:
        case 0xEB:
        case 0xEC:
        case 0xF4:
        case 0xFC:
		case 0xFD:		
			SET_INSTR_CYCLES(ONE_CYCLE);
			break;		
			
// 8bit loads			
		case 0x06: // LD Rn,n
		case 0x0E:
		case 0x16:
		case 0x1E:
		case 0x26:
		case 0x2E:
		case 0x3E: // LD A,#		
			REG_INDEX(DST_REG(opcode)) = memoryRead(cpu, REG_PC++);
			SET_INSTR_CYCLES(TWO_CYCLE);
			break;	
			
		case 0x40: // LD Rn,Rm
        case 0x41:
        case 0x42:
        case 0x43:
        case 0x44:
        case 0x45:                
        case 0x47:
        case 0x48:
        case 0x49:
        case 0x4A:
        case 0x4B:
        case 0x4C:
        case 0x4D:
        case 0x4F:
        case 0x50:
        case 0x51:
        case 0x52:
        case 0x53:
        case 0x54:
        case 0x55:
        case 0x57:
        case 0x58:
        case 0x59:
        case 0x5A:
        case 0x5B:
        case 0x5C:
        case 0x5D:
        case 0x5F:
        case 0x60:
        case 0x61:
        case 0x62:
        case 0x63:
        case 0x64:
        case 0x65:
        case 0x67:
        case 0x68:
        case 0x69:
        case 0x6A:
        case 0x6B:
        case 0x6C:
        case 0x6D:
        case 0x6F:                
        case 0x78:
        case 0x79:
        case 0x7A:
        case 0x7B:
        case 0x7C:
        case 0x7D:
		case 0x7F:		
            REG_INDEX(DST_REG(opcode)) = REG_INDEX(SRC_REG(opcode));
            SET_INSTR_CYCLES(ONE_CYCLE);
            break;
            
        case 0x46: // LD Rn,(HL)
		case 0x4E:
		case 0x56:
		case 0x5e:
		case 0x66:
		case 0x6E:
		case 0x7E: // LD A,(HL)			
			REG_INDEX(DST_REG(opcode)) = memoryRead(cpu, REG_HL);
			SET_INSTR_CYCLES(TWO_CYCLE);
			break;			
			
		case 0x70: // LD (HL),Rn
		case 0x71:
		case 0x72:
		case 0x73:
		case 0x74:
		case 0x75:
		case 0x77: // LD (HL),A			
			memoryWrite(cpu, REG_HL, REG_INDEX(SRC_REG(opcode)));
			SET_INSTR_CYCLES(TWO_CYCLE);
			break;	
                
        case 0x36: // LD (HL),n
			memoryWrite(cpu, REG_HL, memoryRead(cpu, REG_PC++));
			SET_INSTR_CYCLES(THREE_CYCLE);
			break;
			
		case 0x0A: // LD A,(BC)
			REG_A = memoryRead(cpu, REG_BC);
			SET_INSTR_CYCLES(TWO_CYCLE);
			break;
			
		case 0x1A: // LD A,(DE)
			REG_A = memoryRead(cpu, REG_DE);
			SET_INSTR_CYCLES(TWO_CYCLE);
			break;		
						
		case 0xFA: // LD A,(nn)
			aux16 = memoryRead16(cpu, REG_PC);
			REG_A = memoryRead(cpu, aux16);
			REG_PC += 2;
			SET_INSTR_CYCLES(FOUR_CYCLE);
			break;		
			
		case 0x02: // LD (BC),A
			memoryWrite(cpu, REG_BC,REG_A);
			SET_INSTR_CYCLES(TWO_CYCLE);
			break;
			
		case 0x12: // LD (DE),A
			memoryWrite(cpu, REG_DE,REG_A);
			SET_INSTR_CYCLES(TWO_CYCLE);
			break;		
		
		case 0xEA: // LD (nn),A
			memoryWrite(cpu, memoryRead16(cpu, REG_PC),REG_A);
			REG_PC += 2;
			SET_INSTR_CYCLES(FOUR_CYCLE);
			break;	
			
		case 0xF2: // LD A,($FF00+C)
			aux = REG_C;
			REG_A = memoryRead(cpu, 0xFF00 | aux);
			SET_INSTR_CYCLES(TWO_CYCLE);
			break;	
		
		case 0xE2: // LD ($FF00+C),A
			aux = REG_C;
			memoryWrite(cpu, 0xFF00 | aux, REG_A);
			SET_INSTR_CYCLES(TWO_CYCLE);
			break;		
		
		case 0xE0: // LD ($FF00+n),A
			aux = memoryRead(cpu, REG_PC++);
			memoryWrite(cpu, 0xFF00 | aux, REG_A);
			SET_INSTR_CYCLES(THREE_CYCLE);
			break;	
			
		case 0xF0: // LD A,($FF00+n)
			REG_A = memoryRead(cpu, 0xFF00 | memoryRead(cpu, REG_PC++));
			SET_INSTR_CYCLES(THREE_CYCLE);
			break;			
			
		case 0x22: // LD (HLI),A
		case 0x32: // LD (HLD),A
			memoryWrite(cpu, REG_HL, REG_A);
			if(opcode & 0x10) REG_HL--;
			else REG_HL++;
			SET_INSTR_CYCLES(TWO_CYCLE);
			break;			
			
		case 0x2A: // LD A,(HLI)
		case 0x3A: // LD A,(HLD)
			REG_A = memoryRead(cpu, REG_HL);
			if(opcode & 0x10) REG_HL--;
			else REG_HL++;
			SET_INSTR_CYCLES(TWO_CYCLE);
			break;
      		
// 16bit loads	
		case 0x01: // LD n,nn
		case 0x11:
		case 0x21:
			dst = DST_REG(opcode);
			REG_INDEX(dst+1) = memoryRead(cpu, REG_PC++); // LSB
		    REG_INDEX(dst)   = memoryRead(cpu, REG_PC++); // MSB
			SET_INSTR_CYCLES(THREE_CYCLE);
			break;
			
		case 0x31: // LD SP,nn
			REG_SP = memoryRead16(cpu, REG_PC);
			REG_PC +=2;
			SET_INSTR_CYCLES(THREE_CYCLE);
			break;				
			
		case 0xF9: // LD SP,HL
			REG_SP = REG_HL;
			SET_INSTR_CYCLES(TWO_CYCLE);
			break;	
			
		case 0xF8: // LD HL,SP+n
			aux = memoryRead(cpu, REG_PC++);
			PSW = 0;
			if( ((REG_SP & 0xff) + aux) > 0xFF) PSW = FC;
			if( ((REG_SP & 0x0f) + (aux & 0x0f)) > 0x0f) PSW |= FH;				
			REG_HL = REG_SP + (signed char)aux;			
			SET_INSTR_CYCLES(THREE_CYCLE);
			break;				
				
		case 0x08: // LD (nn),SP		
			memoryWrite16(cpu, memoryRead16(cpu, REG_PC), REG_SP);
			REG_PC += 2;
			SET_INSTR_CYCLES(FIVE_CYCLE);
			break;			
			
		case 0xC5: // PUSH Rnn
		case 0xD5:
		case 0xE5:
			dst = DST_REG(opcode);
			memoryWrite(cpu, --REG_SP, REG_INDEX(dst));   // MSB
			memoryWrite(cpu, --REG_SP, REG_INDEX(dst+1)); // LSB
			SET_INSTR_CYCLES(FOUR_CYCLE);
			break;
			
		case 0xF5: // PUSH AF
			memoryWrite(cpu, --REG_SP, REG_A); // MSB			
			memoryWrite(cpu, --REG_SP, PSW & 0xf0); // LSB			
			SET_INSTR_CYCLES(FOUR_CYCLE);
			break;		
		
		case 0xC1: // POP Rnn
		case 0xD1:
		case 0xE1:
			dst = DST_REG(opcode);
			REG_INDEX(dst+1) = memoryRead(cpu, REG_SP++); // LSB
			REG_INDEX(dst) = memoryRead(cpu, REG_SP++); // MSB
			SET_INSTR_CYCLES(THREE_CYCLE);
			break;
			
		case 0xF1: // POP AF
			PSW = memoryRead(cpu, REG_SP++)&0xF0; // LSB
			REG_A = memoryRead(cpu, REG_SP++); // MSB
			SET_INSTR_CYCLES(THREE_CYCLE);
			break;
			
// 8bit ALU
		case 0x80: // ADD A,Rn
		case 0x81:
		case 0x82:
		case 0x83:
		case 0x84:
		case 0x85:			
		case 0x87:						
		case 0x88: // ADC A,Rn
		case 0x89:
		case 0x8A:
		case 0x8B:
		case 0x8C:
		case 0x8D:	
		case 0x8F:	
		case 0x90: // SUB Rn
		case 0x91:
		case 0x92:
		case 0x93:
		case 0x94:
		case 0x95:			
		case 0x97:	
		case 0x98: // SBC A,Rn
		case 0x99:
		case 0x9A:
		case 0x9B:
		case 0x9C:
		case 0x9D:		
		case 0x9F:	
		case 0xA0: // AND Rn
		case 0xA1:
		case 0xA2:
		case 0xA3:
		case 0xA4:
		case 0xA5:			
		case 0xA7:	
		case 0xA8: // XOR Rn
		case 0xA9:
		case 0xAA:
		case 0xAB:
		case 0xAC:
		case 0xAD:		
		case 0xAF:	
		case 0xB0: // OR Rn
		case 0xB1:
		case 0xB2:
		case 0xB3:
		case 0xB4:
		case 0xB5:		
		case 0xB7:	
		case 0xB8: // CP Rn
		case 0xB9:
		case 0xBA:
		case 0xBB:
		case 0xBC:
		case 0xBD:		
		case 0xBF:	
			alu(cpu, DST_REG(opcode), REG_INDEX(SRC_REG(opcode)));
			SET_INSTR_CYCLES(ONE_CYCLE);
			break;
			
		case 0x86: // ADD A,(HL)
		case 0x8E: // ADC A,(HL)
		case 0x96: // SUB (HL)
		case 0x9E: // SBC A,(HL)
		case 0xA6: // AND (HL)
		case 0xAE: // XOR (HL)
		case 0xB6: // OR (HL)
		case 0xBE: // CP (HL)
			alu(cpu, DST_REG(opcode), memoryRead(cpu, REG_HL));
			SET_INSTR_CYCLES(TWO_CYCLE);
			break;		
		
		case 0xC6: // ADD A,#
		case 0xCE: // ADC #
		case 0xD6: // SUB #
		case 0xDE: // SBC A,#
		case 0xE6: // AND #		
		case 0xEE: // XOR #
		case 0xF6: // OR #
		case 0xFE: // CP #
			alu(cpu, DST_REG(opcode),memoryRead(cpu, REG_PC++));
			SET_INSTR_CYCLES(TWO_CYCLE);
			break;		
			
		case 0x04: // INC rn
		case 0x0C:
		case 0x14:
		case 0x1C:	
		case 0x24:
		case 0x2C:
		case 0x3C:
			inc(cpu, REG_ADDR(DST_REG(opcode)));
			SET_INSTR_CYCLES(ONE_CYCLE);
			break;	
			
		case 0x34: // INC (HL)
		case 0x35: // DEC (HL)
			aux = memoryRead(cpu, REG_HL);
			if(opcode&1)
				dec(cpu, &aux);
			else
				inc(cpu, &aux);
			memoryWrite(cpu, REG_HL, aux);
			SET_INSTR_CYCLES(THREE_CYCLE);
			break;
			
		case 0x05: // DEC rn
		case 0x0D:
		case 0x15:
		case 0x1D:
		case 0x25:
		case 0x2D:
		case 0x3D:
			dec(cpu, REG_ADDR(DST_REG(opcode)));
			SET_INSTR_CYCLES(ONE_CYCLE);
			break;		 		
				
// 16bit Arithmetic
		// ADD HL,rnn - flags: -,0,H,C
		case 0x09: add_hl_d16(cpu, REG_BC); SET_INSTR_CYCLES(TWO_CYCLE); break;
		case 0x19: add_hl_d16(cpu, REG_DE); SET_INSTR_CYCLES(TWO_CYCLE); break;
		case 0x29: add_hl_d16(cpu, REG_HL); SET_INSTR_CYCLES(TWO_CYCLE); break;
		case 0x39: add_hl_d16(cpu, REG_SP); SET_INSTR_CYCLES(TWO_CYCLE); break;
			
		case 0xE8: // ADD SP,#				
			aux = memoryRead(cpu, REG_PC++);
			PSW = 0;
			if( ((REG_SP & 0xff) + aux) > 0xFF) PSW = FC;
			if( ((REG_SP & 0x0f) + (aux & 0x0f)) > 0x0f) PSW |= FH;	
			REG_SP += (signed char)aux;
			SET_INSTR_CYCLES(FOUR_CYCLE);
			break;	
			
		// INC Rnn - no flags affected
		case 0x03: REG_BC++; SET_INSTR_CYCLES(TWO_CYCLE); break;
		case 0x13: REG_DE++; SET_INSTR_CYCLES(TWO_CYCLE); break;
		case 0x23: REG_HL++; SET_INSTR_CYCLES(TWO_CYCLE); break;
		case 0x33: REG_SP++; SET_INSTR_CYCLES(TWO_CYCLE); break;
			
		// DEC Rnn
		case 0x0B: REG_BC--; SET_INSTR_CYCLES(TWO_CYCLE); break;
		case 0x1B: REG_DE--; SET_INSTR_CYCLES(TWO_CYCLE); break;
		case 0x2B: REG_HL--; SET_INSTR_CYCLES(TWO_CYCLE); break;
		case 0x3B: REG_SP--; SET_INSTR_CYCLES(TWO_CYCLE); break;
			
// Miscellaneous
		// SWAP, RLC, RL, RRC, RR, SLA, SRA, SRL, BIT, RST, SET
		case 0xCB: cb_decode(cpu); break;
			
		// DAA
		case 0x27: daa(cpu); SET_INSTR_CYCLES(ONE_CYCLE); break;
		
		case 0x2F: // CPL
			REG_A = ~REG_A;
			PSW |= FN | FH;
			SET_INSTR_CYCLES(ONE_CYCLE);
			break;	
			
		case 0x3F: // CCF
			PSW &= ~( FN | FH);
			PSW ^= FC;
			SET_INSTR_CYCLES(ONE_CYCLE);
			break;
			
		case 0x37: // SCF			
			PSW &= ~(FN | FH);
			PSW |= FC;
			SET_INSTR_CYCLES(ONE_CYCLE);
			break;
			
		case 0x76: // HALT
			cpu->halt = 1;
			SET_INSTR_CYCLES(ONE_CYCLE);         
			break;	
			
		case 0x10: // STOP
			cpu->stopped	= 1;
			SET_INSTR_CYCLES(ONE_CYCLE);
			break;
			
		case 0xF3: // DI
		case 0xFB: // EI
			cpu->IME = (opcode>>3) & 1 ;
			SET_INSTR_CYCLES(ONE_CYCLE);
			break;	
			
// Rotates & shifts		//TODO SET Flag zero??
		case 0x07: // RLCA
			aux = REG_A >> 7;		
			PSW = (aux & (1<<0)) ? FC : 0;	
    		REG_A = (REG_A << 1) | aux;		
			SET_INSTR_CYCLES(ONE_CYCLE);
			break;
				
		case 0x17: // RLA
			aux = (PSW & FC) ? 1 : 0;
			PSW = (REG_A & (1<<7)) ? FC : 0;
			REG_A = (REG_A << 1) | aux;
			SET_INSTR_CYCLES(ONE_CYCLE);
			break;	
			
		case 0x0F: // RRCA
			aux = REG_A << 7;    	
			PSW = (aux & (1<<7)) ? FC : 0;
    		REG_A = aux | (REG_A >> 1);    
			SET_INSTR_CYCLES(ONE_CYCLE);
			break;	
			
		case 0x1F: // RRA
			aux = (PSW & FC) ? (1<<7) : 0;
			PSW = (REG_A & (1<<0)) ? FC : 0;
			REG_A = aux | (REG_A >> 1);
			SET_INSTR_CYCLES(ONE_CYCLE);
			break;	
			
// Jumps			
		case 0xC3: // JP nn
			REG_PC = memoryRead16(cpu, REG_PC);
			SET_INSTR_CYCLES(FOUR_CYCLE);
			break;	
			
		case 0xC2: // JP NZ,nn
		case 0xCA: // JP z,nn
			aux = (opcode<<4) & FZ;			
			if ((PSW & FZ) == aux) {
				REG_PC = memoryRead16(cpu, REG_PC);
				SET_INSTR_CYCLES(ONE_CYCLE);
			}
			else	
				REG_PC += 2;
			SET_INSTR_CYCLES(THREE_CYCLE);
			break;
			
		case 0xD2: // JP NC,nn
		case 0xDA: // JP C,nn
			aux = (opcode<<1) & FC;			
			if ((PSW & FC) == aux) {
				REG_PC = memoryRead16(cpu, REG_PC);
				SET_INSTR_CYCLES(ONE_CYCLE);
			}
			else	
				REG_PC += 2;
			SET_INSTR_CYCLES(THREE_CYCLE);
			break;
			
		case 0xE9: // JP (HL)
			REG_PC = REG_HL;
			SET_INSTR_CYCLES(ONE_CYCLE);
			break;
			
		case 0x18: // JR n
			REG_PC = REG_PC + (signed char)memoryRead(cpu, REG_PC) + 1;
			SET_INSTR_CYCLES(THREE_CYCLE);
			break;	
			
		case 0x20: // JR NZ,n
		case 0x28: // JR Z,n
			aux = (opcode<<4) & FZ;			
			if ((PSW & FZ) == aux) {
				REG_PC = REG_PC + (signed char)memoryRead(cpu, REG_PC) + 1;
				SET_INSTR_CYCLES(ONE_CYCLE);
			}else
				REG_PC++;				
			SET_INSTR_CYCLES(TWO_CYCLE);
			break;		
			
		case 0x30: // JR NC,n
		case 0x38: // JR C,n
			aux = (opcode<<1) & FC;
			if ((PSW & FC) == aux) {
				REG_PC = REG_PC + (signed char)memoryRead(cpu, REG_PC) + 1;
				SET_INSTR_CYCLES(ONE_CYCLE);
			}
			else	
				REG_PC++;				
			SET_INSTR_CYCLES(TWO_CYCLE);
			break;
			
// Calls			
			
		case 0xCD: // CALL nn
			PUSH(cpu, REG_PC + 2);
			REG_PC = memoryRead16(cpu, REG_PC);
			SET_INSTR_CYCLES(SIX_CYCLE);
			break;	
			
		case 0xC4: // CALL NZ,nn
		case 0xCC: // CALL z,nn
			aux = (opcode<<4) & FZ;			
			if( (PSW & FZ) == aux )	{
				PUSH(cpu, REG_PC+2);
				REG_PC = memoryRead16(cpu, REG_PC);
				SET_INSTR_CYCLES(THREE_CYCLE);
			}
			else
				REG_PC += 2;
			SET_INSTR_CYCLES(THREE_CYCLE);
			break;
			
		case 0xD4: // CALL NC,nn
		case 0xDC: // CALL C,nn
			aux = (opcode<<1) & FC;			
			if( (PSW & FC) == aux ) {
				PUSH(cpu, REG_PC+2);
				REG_PC = memoryRead16(cpu, REG_PC);
				SET_INSTR_CYCLES(THREE_CYCLE);
			}
			else
				REG_PC += 2;
			SET_INSTR_CYCLES(THREE_CYCLE);
			break;
			
// Restarts

		case 0xC7: // RST n
		case 0xCF:
		case 0xD7:
		case 0xDF:
		case 0xE7:
		case 0xEF:
		case 0xF7:
		case 0xFF:
			PUSH(cpu, REG_PC);
			REG_PC = opcode & 0x38;
			SET_INSTR_CYCLES(FOUR_CYCLE);
			break;
			
// Returns			
		
		case 0xD9: // RETI
			 cpu->IME = 1;				
		case 0xC9: // RET
			POP(cpu, REG_PC);
			SET_INSTR_CYCLES(FOUR_CYCLE);
			break;	
			
		case 0xC0: // RET NZ
		case 0xC8: // RET Z
			aux = (opcode<<4) & FZ;			
			if ((PSW & FZ) == aux) {
				POP(cpu, REG_PC);
				SET_INSTR_CYCLES(THREE_CYCLE);
			}
			SET_INSTR_CYCLES(TWO_CYCLE);
			break;
			
		case 0xD0: // RET NC
		case 0xD8: // RET C
			aux = (opcode<<1) & FC;			
			if ((PSW & FC) == aux) {
				POP(cpu, REG_PC);
				SET_INSTR_CYCLES(THREE_CYCLE);
			}
			SET_INSTR_CYCLES(TWO_CYCLE);		
			break;			
		
			
		default:
		//	printf("\nbad instruction 0x%.2X at 0x%.4X\n",opcode,PC);
			break;
		
	}
}
//-----------------------------------------
// 2 byte instructions
//-----------------------------------------
static uint8_t cb_decode(cpu_t *cpu)
{
	uint8_t op, reg, aux;
	uint8_t bitnum;

	op = memoryRead(cpu, REG_PC++);
	reg = op & 7;
	bitnum = (op >> 3) & 7;
 
	switch(op)
	{
		case 0x00: // RLC Rn
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
		case 0x05:
		case 0x07:
			rlc(cpu, REG_ADDR(reg));
			SET_INSTR_CYCLES(TWO_CYCLE);
			break;		
		
		case 0x06: // RLC (HL)	
			aux = memoryRead(cpu, REG_HL);
			rlc(cpu, &aux);
			memoryWrite(cpu, REG_HL, aux);
			SET_INSTR_CYCLES(FOUR_CYCLE);	
			break;
						
		case 0x08: // RRC Rn
		case 0x09:
		case 0x0A:
		case 0x0B:
		case 0x0C:
		case 0x0D:
		case 0x0F:
			rrc(cpu, REG_ADDR(reg));
			SET_INSTR_CYCLES(TWO_CYCLE);
			break;		
		
		case 0x0E: // RRC (HL)	
			aux = memoryRead(cpu, REG_HL);
			rrc(cpu, &aux);
			memoryWrite(cpu, REG_HL, aux);
			SET_INSTR_CYCLES(FOUR_CYCLE);	
			break;			
			
		case 0x10: // RL Rn
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x17:
			rl(cpu, REG_ADDR(reg));
			SET_INSTR_CYCLES(TWO_CYCLE);
			break;		
		
		case 0x16: // RL (HL)	
			aux = memoryRead(cpu, REG_HL);
			rl(cpu, &aux);
			memoryWrite(cpu, REG_HL, aux);
			SET_INSTR_CYCLES(FOUR_CYCLE);	
			break;	
		
		case 0x18: // RR Rn
		case 0x19:
		case 0x1A:
		case 0x1B:
		case 0x1C:
		case 0x1D:
		case 0x1F:
			rr(cpu, REG_ADDR(reg));
			SET_INSTR_CYCLES(TWO_CYCLE);
			break;		
		
		case 0x1E: // RR (HL)	
			aux = memoryRead(cpu, REG_HL);
			rr(cpu, &aux);
			memoryWrite(cpu, REG_HL,aux);
			SET_INSTR_CYCLES(FOUR_CYCLE);	
			break;
			
		case 0x20: // SLA Rn
		case 0x21:
		case 0x22:
		case 0x23:
		case 0x24:
		case 0x25:
		case 0x27:
			sla(cpu, REG_ADDR(reg));
			SET_INSTR_CYCLES(TWO_CYCLE);
			break;		
		
		case 0x26: // SLA (HL)	
			aux = memoryRead(cpu, REG_HL);
			sla(cpu, &aux);
			memoryWrite(cpu, REG_HL, aux);
			SET_INSTR_CYCLES(FOUR_CYCLE);
			break;
			
		case 0x28: // SRA Rn
		case 0x29:
		case 0x2A:
		case 0x2B:
		case 0x2C:
		case 0x2D:
		case 0x2F:
			sra(cpu, REG_ADDR(reg));
			SET_INSTR_CYCLES(TWO_CYCLE);
			break;		
		
		case 0x2E: // SRA (HL)	
			aux = memoryRead(cpu, REG_HL);
			sra(cpu, &aux);
			memoryWrite(cpu, REG_HL, aux);
			SET_INSTR_CYCLES(FOUR_CYCLE);	
			break;			
		
		case 0x30: // SWAP Rn		
		case 0x31:
		case 0x32:
		case 0x33:
		case 0x34:
		case 0x35:
		case 0x37:
			swap(cpu, REG_ADDR(reg));
			SET_INSTR_CYCLES(TWO_CYCLE);
			break;	
			
		case 0x36: // SWAP (HL)
			aux = memoryRead(cpu, REG_HL);
			swap(cpu, &aux);
			memoryWrite(cpu, REG_HL, aux);
			SET_INSTR_CYCLES(FOUR_CYCLE);
			break;
		
		case 0x38: // SRL Rn
		case 0x39:
		case 0x3A:
		case 0x3B:
		case 0x3C:
		case 0x3D:
		case 0x3F:
			srl(cpu, REG_ADDR(reg));
			SET_INSTR_CYCLES(TWO_CYCLE);
			break;
			
		case 0x3E: // SRL (HL)
			aux = memoryRead(cpu, REG_HL);
			srl(cpu, &aux);
			memoryWrite(cpu, REG_HL, aux);
			SET_INSTR_CYCLES(FOUR_CYCLE);
			break;	
			
		default:
			switch(op & 0xC0)
			{
				case 0x40:
					bit(cpu, bitnum, REG_ADDR(reg));
					break;
					
				case 0x80:
					res(cpu, bitnum, REG_ADDR(reg));
					break;
					
				case 0xC0:
					set(cpu, bitnum, REG_ADDR(reg));
					break;
			}
			break;
	}

	return 0;
}
#endif

