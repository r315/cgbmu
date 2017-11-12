#include "debug.h"
#include "dmgcpu.h"
#include "alu.h"

//-----------------------------------------
//-----------------------------------------  
void decode(void)
{
uint8_t src;
uint8_t dst;
uint8_t aux;
uint8_t opcode;
uint16_t aux16;

	if(halted)
	 opcode = 0; // NOP
    else
     opcode = memoryRead(REG_PC++);
	
	src = (opcode & 0x07);	
	dst = ((opcode >> 3) & 0x07);
	
	cycles = 0;

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
			cycles +=4;
			break;		
			
// 8bit loads			
		case 0x06: // LD Rn,n
		case 0x0E:
		case 0x16:
		case 0x1E:
		case 0x26:
		case 0x2E:
		case 0x3E: // LD A,#		
			REG_INDEX(dst) = memoryRead(REG_PC++);
			cycles += 8;
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
            REG_INDEX(dst) = REG_INDEX(src);
            cycles += 4;
            break;
            
        case 0x46: // LD Rn,(HL)
		case 0x4E:
		case 0x56:
		case 0x5e:
		case 0x66:
		case 0x6E:
		case 0x7E: // LD A,(HL)			
			REG_INDEX(dst) = memoryRead(REG_HL);
			cycles += 8;
			break;			
			
		case 0x70: // LD (HL),Rn
		case 0x71:
		case 0x72:
		case 0x73:
		case 0x74:
		case 0x75:
		case 0x77: // LD (HL),A			
			memoryWrite(REG_HL, REG_INDEX(src));
			cycles += 8;
			break;	
                
        case 0x36: // LD (HL),n
			memoryWrite(REG_HL, memoryRead(REG_PC++));
			cycles += 12;
			break;
			
		case 0x0A: // LD A,(BC)
			REG_A = memoryRead(REG_BC);
			cycles += 8;
			break;
			
		case 0x1A: // LD A,(DE)
			REG_A = memoryRead(REG_DE);
			cycles += 8;
			break;		
						
		case 0xFA: // LD A,(nn)
			aux16 = memoryRead16(REG_PC);
			REG_A = memoryRead(aux16);
			REG_PC += 2;
			cycles += 16;
			break;		
			
		case 0x02: // LD (BC),A
			memoryWrite(REG_BC,REG_A);
			cycles += 8;
			break;
			
		case 0x12: // LD (DE),A
			memoryWrite(REG_DE,REG_A);
			cycles += 8;
			break;		
		
		case 0xEA: // LD (nn),A
			memoryWrite(memoryRead16(REG_PC),REG_A);
			REG_PC += 2;
			cycles +=16;
			break;	
			
		case 0xF2: // LD A,($FF00+C)
			aux = REG_C;
			REG_A = memoryRead(0xFF00 | aux);
			cycles += 8;
			break;	
		
		case 0xE2: // LD ($FF00+C),A
			aux = REG_C;
			memoryWrite(0xFF00 | aux, REG_A);
			cycles += 8;
			break;		
		
		case 0xE0: // LD ($FF00+n),A
			aux = memoryRead(REG_PC++);
			memoryWrite(0xFF00 | aux, REG_A);
			cycles += 12;
			break;	
			
		case 0xF0: // LD A,($FF00+n)
			aux = memoryRead(REG_PC++);				
			REG_A = memoryRead(0xFF00 | aux);
			cycles += 12;
			break;			
			
		case 0x22: // LD (HLI),A
		case 0x32: // LD (HLD),A
			memoryWrite(REG_HL, REG_A);
			if(opcode & 0x10) dec16(&REG_HL);
			else inc16(&REG_HL);
			cycles += 8;
			break;			
			
		case 0x2A: // LD A,(HLI)
		case 0x3A: // LD A,(HLD)
			REG_A = memoryRead(REG_HL);
			if(opcode & 0x10) dec16(&REG_HL);
			else inc16(&REG_HL);
			cycles += 8;
			break;
      		
// 16bit loads	
		case 0x01: // LD n,nn
		case 0x11:
		case 0x21:			
			REG_INDEX(dst+1) = memoryRead(REG_PC++); // LSB
		    REG_INDEX(dst)   = memoryRead(REG_PC++); // MSB
			cycles +=12;
			break;
			
		case 0x31: // LD SP,nn
			REG_SP = memoryRead16(REG_PC);
			REG_PC +=2;
			cycles += 12;
			break;				
			
		case 0xF9: // LD SP,HL
			REG_SP = REG_HL;
			cycles += 8;
			break;	
			
		case 0xF8: // LD HL,SP+n
			aux = memoryRead(REG_PC++);
			REG_F = 0;
			if( ((REG_SP & 0xff) + aux) > 0xFF) PSW = FC;
			if( ((REG_SP & 0x0f) + (aux & 0x0f)) > 0x0f) PSW |= FH;				
			REG_HL = REG_SP + (signed char)aux;			
			cycles += 12;
			break;				
				
		case 0x08: // LD (nn),SP		
			memoryWrite16(memoryRead16(REG_PC), REG_SP);
			REG_PC += 2;
			cycles += 20;
			break;			
			
		case 0xC5: // PUSH Rnn
		case 0xD5:
		case 0xE5:
			memoryWrite(--REG_SP, REG_INDEX(dst));   // MSB
			memoryWrite(--REG_SP, REG_INDEX(dst+1)); // LSB
			cycles += 16;
			break;
			
		case 0xF5: // PUSH AF
			memoryWrite(--REG_SP, REG_A); // MSB			
			memoryWrite(--REG_SP, REG_F & 0xf0); // LSB			
			cycles += 16;
			break;		
		
		case 0xC1: // POP Rnn
		case 0xD1:
		case 0xE1:
			REG_INDEX(dst+1) = memoryRead(REG_SP++); // LSB
			REG_INDEX(dst) = memoryRead(REG_SP++); // MSB
			cycles += 12;
			break;
			
		case 0xF1: // POP AF
			PSW = memoryRead(REG_SP++)&0xF0; // LSB
			REG_A = memoryRead(REG_SP++); // MSB
			cycles += 12;
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
			alu(dst, REG_INDEX(src));
			cycles += 4;
			break;
			
		case 0x86: // ADD A,(HL)
		case 0x8E: // ADC A,(HL)
		case 0x96: // SUB (HL)
		case 0x9E: // SBC A,(HL)
		case 0xA6: // AND (HL)
		case 0xAE: // XOR (HL)
		case 0xB6: // OR (HL)
		case 0xBE: // CP (HL)
			alu(dst, memoryRead(REG_HL));
			cycles += 8;
			break;		
		
		case 0xC6: // ADD A,#
		case 0xCE: // ADC #
		case 0xD6: // SUB #
		case 0xDE: // SBC A,#
		case 0xE6: // AND #		
		case 0xEE: // XOR #
		case 0xF6: // OR #
		case 0xFE: // CP #
			alu(dst,memoryRead(REG_PC++));
			cycles += 8;
			break;		
			
		case 0x04: // INC rn
		case 0x0C:
		case 0x14:
		case 0x1C:	
		case 0x24:
		case 0x2C:
		case 0x3C:
			inc(REG_ADDR(dst));
			cycles += 4;
			break;	
			
		case 0x34: // INC (HL)
		case 0x35: // DEC (HL)
			aux = memoryRead(REG_HL);
			if(opcode&1)
				dec(&aux);
			else
				inc(&aux);
			memoryWrite(REG_HL, aux);
			cycles += 12;
			break;
			
		case 0x05: // DEC rn
		case 0x0D:
		case 0x15:
		case 0x1D:
		case 0x25:
		case 0x2D:
		case 0x3D:
			dec(REG_ADDR(dst));
			cycles += 4;
			break;		 		
				
// 16bit Arithmetic
		// ADD HL,rnn - flags: -,0,H,C
		case 0x09: addHL(REG_BC); cycles += 8; break;
		case 0x19: addHL(REG_DE); cycles += 8; break;
		case 0x29: addHL(REG_HL); cycles += 8; break;
		case 0x39: addHL(REG_SP); cycles += 8; break;
			
		case 0xE8: // ADD SP,#				
			aux = memoryRead(REG_PC++);
			PSW = 0;
			if( ((REG_SP & 0xff) + aux) > 0xFF) PSW = FC;
			if( ((REG_SP & 0x0f) + (aux & 0x0f)) > 0x0f) PSW |= FH;	
			REG_SP += (signed char)aux;
			cycles += 16;
			break;	
			
		// INC Rnn - no flags affected
		case 0x03: inc16(&REG_BC); cycles += 8; break;
		case 0x13: inc16(&REG_DE); cycles += 8; break;
		case 0x23: inc16(&REG_HL); cycles += 8; break;
		case 0x33: inc16(&REG_SP); cycles += 8; break;
			
		// DEC Rnn
		case 0x0B: dec16(&REG_BC); cycles += 8; break;
		case 0x1B: dec16(&REG_DE); cycles += 8; break;
		case 0x2B: dec16(&REG_HL); cycles += 8; break;
		case 0x3B: dec16(&REG_SP); cycles += 8; break;
			
// Miscellaneous
		// SWAP, RLC, RL, RRC, RR, SLA, SRA, SRL, BIT, RST, SET
		case 0xCB: CBcodes(memoryRead(REG_PC++));break;
			
		// DAA
		case 0x27: daa(); cycles += 4; break;
		
		case 0x2F: // CPL
			REG_A = ~REG_A;
			PSW |= FN | FH;
			cycles += 4;
			break;	
			
		case 0x3F: // CCF
			PSW &= ~( FN | FH);
			PSW ^= FC;
			cycles  += 4;
			break;
			
		case 0x37: // SCF			
			PSW &= ~(FN | FH);
			PSW |= FC;
			cycles += 4;
			break;
			
		case 0x76: // HALT
			halted = 1;			
			cycles += 4;         
			break;	
			
		case 0x10: // STOP
			stopped	= 1;
			cycles += 4;
			break;
			
		case 0xF3: // DI
		case 0xFB: // EI
			IME = (opcode>>3) & 1 ;
			cycles += 4;
			break;	
			
// Rotates & shifts		//TODO SET Flag zero??
		case 0x07: // RLCA
			aux = REG_A >> 7;		
			PSW = (aux & (1<<0)) ? FC : 0;	
    		REG_A = (REG_A << 1) | aux;		
			cycles += 4;
			break;
				
		case 0x17: // RLA
			aux = (PSW & FC) ? 1 : 0;
			PSW = (REG_A & (1<<7)) ? FC : 0;
			REG_A = (REG_A << 1) | aux;
			cycles += 4;
			break;	
			
		case 0x0F: // RRCA
			aux = REG_A << 7;    	
			PSW = (aux & (1<<7)) ? FC : 0;
    		REG_A = aux | (REG_A >> 1);    
			cycles += 4;
			break;	
			
		case 0x1F: // RRA
			aux = (PSW & FC) ? (1<<7) : 0;
			PSW = (REG_A & (1<<0)) ? FC : 0;
			REG_A = aux | (REG_A >> 1);
			cycles += 4;
			break;	
			
// Jumps			
		case 0xC3: // JP nn
			REG_PC = memoryRead16(REG_PC);
			cycles += 16;
			break;	
			
		case 0xC2: // JP NZ,nn
		case 0xCA: // JP z,nn
			aux = (opcode<<4) & FZ;			
			if ((PSW & FZ) == aux) {
				REG_PC = memoryRead16(REG_PC);
				cycles += 4;
			}
			else	
				REG_PC += 2;
			cycles += 12;
			break;
			
		case 0xD2: // JP NC,nn
		case 0xDA: // JP C,nn
			aux = (opcode<<1) & FC;			
			if ((PSW & FC) == aux) {
				REG_PC = memoryRead16(REG_PC);
				cycles += 4;
			}
			else	
				REG_PC += 2;
			cycles += 12;
			break;
			
		case 0xE9: // JP (HL)
			REG_PC = REG_HL;
			cycles += 4;
			break;
			
		case 0x18: // JR n
			REG_PC = REG_PC + (signed char)memoryRead(REG_PC) + 1;
			cycles += 12;
			break;	
			
		case 0x20: // JR NZ,n
		case 0x28: // JR Z,n
			aux = (opcode<<4) & FZ;			
			if ((PSW & FZ) == aux) {
				REG_PC = REG_PC + (signed char)memoryRead(REG_PC) + 1;
				cycles += 4;
			}else
				REG_PC++;				
			cycles += 8;
			break;		
			
		case 0x30: // JR NC,n
		case 0x38: // JR C,n
			aux = (opcode<<1) & FC;
			if ((PSW & FC) == aux) {
				REG_PC = REG_PC + (signed char)memoryRead(REG_PC) + 1;
				cycles += 4;
			}
			else	
				REG_PC++;				
			cycles += 8;
			break;
			
// Calls			
			
		case 0xCD: // CALL nn
			push(REG_PC + 2);
			REG_PC = memoryRead16(REG_PC);
			cycles += 24;
			break;	
			
		case 0xC4: // CALL NZ,nn
		case 0xCC: // CALL z,nn
			aux = (opcode<<4) & FZ;			
			if( (PSW & FZ) == aux )	{
				push(REG_PC+2);
				REG_PC = memoryRead16(REG_PC);
				cycles += 12;
			}
			else
				REG_PC += 2;
			cycles += 12;
			break;
			
		case 0xD4: // CALL NC,nn
		case 0xDC: // CALL C,nn
			aux = (opcode<<1) & FC;			
			if( (PSW & FC) == aux ) {
				push(REG_PC+2);
				REG_PC = memoryRead16(REG_PC);
				cycles += 12;
			}
			else
				REG_PC += 2;
			cycles += 12;
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
			push(REG_PC);
			REG_PC = opcode & 0x38;
			cycles += 16;
			break;
			
// Returns			
		
		case 0xD9: // RETI
			 IME = 1;				
		case 0xC9: // RET
			REG_PC = pop();
			cycles += 16;
			break;	
			
		case 0xC0: // RET NZ
		case 0xC8: // RET Z
			aux = (opcode<<4) & FZ;			
			if ((PSW & FZ) == aux) {
				REG_PC = pop();
				cycles += 12;
			}
			cycles += 8;
			break;
			
		case 0xD0: // RET NC
		case 0xD8: // RET C
			aux = (opcode<<1) & FC;			
			if ((PSW & FC) == aux) {
				REG_PC = pop();
				cycles += 12;
			}
			cycles += 8;		
			break;			
		
			
		default:
		//	printf("\nbad instruction 0x%.2X at 0x%.4X\n",opcode,PC);
			break;
		
	}
}
//-----------------------------------------
// 2 byte instructions
//-----------------------------------------
void CBcodes(uint8_t op)
{
uint8_t reg = op & 7, aux;
uint8_t bit = (op >> 3) & 7;
 
	switch(op)
	{
		case 0x00: // RLC Rn
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
		case 0x05:
		case 0x07:
			rlc(REG_ADDR(reg));
			cycles += 8;
			break;		
		
		case 0x06: // RLC (HL)	
			aux = memoryRead(REG_HL);
			rlc(&aux);
			memoryWrite(REG_HL, aux);
			cycles += 16;	
			break;
						
		case 0x08: // RRC Rn
		case 0x09:
		case 0x0A:
		case 0x0B:
		case 0x0C:
		case 0x0D:
		case 0x0F:
			rrc(REG_ADDR(reg));
			cycles += 8;
			break;		
		
		case 0x0E: // RRC (HL)	
			aux = memoryRead(REG_HL);
			rrc(&aux);
			memoryWrite(REG_HL, aux);
			cycles += 16;	
			break;			
			
		case 0x10: // RL Rn
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x17:
			rl(REG_ADDR(reg));
			cycles += 8;
			break;		
		
		case 0x16: // RL (HL)	
			aux = memoryRead(REG_HL);
			rl(&aux);
			memoryWrite(REG_HL, aux);
			cycles += 16;	
			break;	
		
		case 0x18: // RR Rn
		case 0x19:
		case 0x1A:
		case 0x1B:
		case 0x1C:
		case 0x1D:
		case 0x1F:
			rr(REG_ADDR(reg));
			cycles += 8;
			break;		
		
		case 0x1E: // RR (HL)	
			aux = memoryRead(REG_HL);
			rr(&aux);
			memoryWrite(REG_HL,aux);
			cycles += 16;	
			break;
			
		case 0x20: // SLA Rn
		case 0x21:
		case 0x22:
		case 0x23:
		case 0x24:
		case 0x25:
		case 0x27:
			sla(REG_ADDR(reg));
			cycles += 8;
			break;		
		
		case 0x26: // SLA (HL)	
			aux = memoryRead(REG_HL);
			sla(&aux);
			memoryWrite(REG_HL, aux);
			cycles += 16;
			break;
			
		case 0x28: // SRA Rn
		case 0x29:
		case 0x2A:
		case 0x2B:
		case 0x2C:
		case 0x2D:
		case 0x2F:
			sra(REG_ADDR(reg));
			cycles += 8;
			break;		
		
		case 0x2E: // SRA (HL)	
			aux = memoryRead(REG_HL);
			sra(&aux);
			memoryWrite(REG_HL, aux);
			cycles += 16;	
			break;			
		
		case 0x30: // SWAP Rn		
		case 0x31:
		case 0x32:
		case 0x33:
		case 0x34:
		case 0x35:
		case 0x37:
			swap(REG_ADDR(reg));
			cycles += 8;
			break;	
			
		case 0x36: // SWAP (HL)
			aux = memoryRead(REG_HL);
			swap(&aux);
			memoryWrite(REG_HL, aux);
			cycles += 16;
			break;
		
		case 0x38: // SRL Rn
		case 0x39:
		case 0x3A:
		case 0x3B:
		case 0x3C:
		case 0x3D:
		case 0x3F:
			srl(REG_ADDR(reg));
			cycles += 8;
			break;
			
		case 0x3E: // SRL (HL)
			aux = memoryRead(REG_HL);
			srl(&aux);
			memoryWrite(REG_HL, aux);
			cycles += 16;
			break;	
			
		default:
			switch(op & 0xC0)
			{
				case 0x40:
					BIT(bit, REG_ADDR(reg));
					break;
					
				case 0x80:
					res(bit, REG_ADDR(reg));
					break;
					
				case 0xC0:
					set(bit, REG_ADDR(reg));
					break;
			}
			break;
	}	
}
