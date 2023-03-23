#include <stdio.h>
#include "dmgcpu.h"
#include "cartridge.h"

uint32_t clockcounter = 0;
char rname[] = {"bcdehlfa"};

void disassembleHeader(void) {
	printf("\n\t\tInstruction\t\tcycles\n");
}

void disassemble(void){
uint8_t src;
uint8_t dst;
uint8_t opcode;

    clockcounter += instr_cycles;
    printf("PC: %04X\t", REG_PC);

    opcode = memoryRead(REG_PC);	
	src = (opcode & 0x07);	
    dst = ((opcode >> 3) & 0x07);

	switch(opcode)
	{
        default: printf("\nbad instruction 0x%.2X at 0x%.4X\n",opcode,REG_PC); break;	
            
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
			printf("nop\t\t");
			break;		
			
// 8bit loads			
		case 0x06: // LD Rn,n
		case 0x0E:
		case 0x16:
		case 0x1E:
		case 0x26:
		case 0x2E:
        case 0x3E:
            printf("ld %c,$%02x\t",rname[dst],memoryRead(REG_PC+1));		
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
            printf("ld %c,%c\t\t",rname[dst],rname[src]);           
            break;
            
        case 0x46: // LD Rn,(HL)
		case 0x4E:
		case 0x56:
		case 0x5e:
		case 0x66:
		case 0x6E:
        case 0x7E:
            printf("ld %c,(HL)\t",rname[dst]);				
			break;			
			
		case 0x70: // LD (HL),Rn
		case 0x71:
		case 0x72:
		case 0x73:
		case 0x74:
		case 0x75:
        case 0x77:
            printf("ld (HL),%c\t",rname[src]);break;	
                
        case 0x36: // LD (HL),n
            printf("ld (HL),%02x\t",memoryRead(REG_PC+1));break;
			
        case 0x0A: // LD A,(BC)
            printf("ld a,(BC),%02x\t",memoryRead(REG_BC+1));break;
			
        case 0x1A: // LD A,(DE)
            printf("ld a,(DE),%02x\t",memoryRead(REG_DE+1));break;		
						
        case 0xFA: // LD A,(nn)
            printf("ld a,($%04x)\t",memoryRead16(REG_PC+1));break;		
			
        case 0x02: // LD (BC),A
            printf("ld (BC),a\t"); break;		
		case 0x12: // LD (DE),A
			printf("ld (DE),a\t"); break;		
        case 0xEA: // LD (nn),A
            printf("ld ($%04x),a\t",memoryRead16(REG_PC+1)); break;			
        case 0xF2: // LD A,($FF00+C)
            printf("ld a,($%04x)\t",0xFF00 + REG_C); break;		
		case 0xE2: // LD ($FF00+C),A
			printf("ld ($%04x),a\t",0xFF00 + REG_C); break;		
        case 0xE0: // LD ($FF00+n),A
            printf("ld ($%04x),a\t",0xFF00 + memoryRead(REG_PC+1)); break;			
		case 0xF0: // LD A,($FF00+n)
			printf("ld a,($%04x)\t",0xFF00 + memoryRead(REG_PC+1)); break;			
        case 0x22: // LD (HLI),A 
            printf("ld (hl+),a\t"); break;
		case 0x32: // LD (HLD),A
			printf("ld (hl-),a\t"); break;			
        case 0x2A: // LD A,(HLI)
            printf("ld a,(hl+)\t"); break;
		case 0x3A: // LD A,(HLD)
			printf("ld a,(hl-)\t"); break;      		
// 16bit loads	
        case 0x01: // LD n,nn
            printf("ld bc,$%04x\t", memoryRead16(REG_PC+1)); break;
        case 0x11:
            printf("ld de,$%04x\t", memoryRead16(REG_PC+1)); break;
        case 0x21:
            printf("ld hl,$%04x\t", memoryRead16(REG_PC+1)); break;			
		case 0x31: 
			printf("ld sp,$%04x\t", memoryRead16(REG_PC+1)); break;			
		case 0xF9: // LD SP,HL
			printf("ld sp,hl\t"); break;			
        case 0xF8: // LD HL,SP+n
            printf("ld hl,sp+n\t"); break;				
        case 0x08: // LD (nn),SP
             printf("ld ($%04x),sp\t", memoryRead16(REG_PC+1)); break;			
        case 0xC5: // PUSH Rnn
            printf("push bc\t\t"); break;		
        case 0xD5:
            printf("push de\t\t"); break;	
		case 0xE5:
			printf("push hl\t\t"); break;			
		case 0xF5:
			printf("push af\t\t"); break;		
        case 0xC1: // POP Rnn
            printf("pop bc\t\t"); break;	
        case 0xD1:
            printf("pop de\t\t"); break;
		case 0xE1:
			printf("pop hl\t\t"); break;		
		case 0xF1: 
			printf("pop af\t\t"); break;			
// 8bit ALU
		case 0x80: // ADD A,Rn
		case 0x81:
		case 0x82:
		case 0x83:
		case 0x84:
		case 0x85:			
		case 0x87:	printf("add a,%c\t\t", rname[src]); break;					
		case 0x88: // ADC A,Rn
		case 0x89:
		case 0x8A:
		case 0x8B:
		case 0x8C:
		case 0x8D:	
		case 0x8F: printf("adc a,%c\t\t", rname[src]); break;	
		case 0x90: // SUB Rn
		case 0x91:
		case 0x92:
		case 0x93:
		case 0x94:
		case 0x95:			
		case 0x97: printf("sub a,%c\t\t", rname[src]); break;
		case 0x98: // SBC A,Rn
		case 0x99:
		case 0x9A:
		case 0x9B:
		case 0x9C:
		case 0x9D:		
		case 0x9F: printf("sbc a,%c\t\t", rname[src]); break;	
		case 0xA0: // AND Rn
		case 0xA1:
		case 0xA2:
		case 0xA3:
		case 0xA4:
		case 0xA5:			
		case 0xA7: printf("and a,%c\t\t", rname[src]); break;	
		case 0xA8: // XOR Rn
		case 0xA9:
		case 0xAA:
		case 0xAB:
		case 0xAC:
		case 0xAD:		
		case 0xAF: printf("xor a,%c\t\t", rname[src]); break;	
		case 0xB0: // OR Rn
		case 0xB1:
		case 0xB2:
		case 0xB3:
		case 0xB4:
		case 0xB5:		
		case 0xB7: printf("or a,%c\t\t", rname[src]); break;	
		case 0xB8: // CP Rn
		case 0xB9:
		case 0xBA:
		case 0xBB:
		case 0xBC:
		case 0xBD:		
		case 0xBF: printf("cp %c\t", rname[src]); break;			
			
        case 0x86: // ADD A,(HL)
            printf("add a,(hl)\t"); break;			
        case 0x8E: // ADC A,(HL)
            printf("adc a,(hl)\t"); break;
        case 0x96: // SUB (HL)
            printf("sub a,(hl)\t"); break;
        case 0x9E: // SBC A,(HL)
            printf("sbc a,(hl)\t"); break;
        case 0xA6: // AND (HL)
            printf("and a,(hl)\t"); break;
        case 0xAE: // XOR (HL)
            printf("xor a,(hl)\t"); break;
        case 0xB6: // OR (HL)
            printf("or a,(hl)\t"); break;
		case 0xBE: // CP (HL)
            printf("cp (hl)\t\t"); break;
            		
        case 0xC6: // ADD A,#
            printf("add a,%02x\t", memoryRead(REG_PC+1)); break;
        case 0xCE: // ADC #
            printf("adc a,%02x\t", memoryRead(REG_PC+1)); break;
        case 0xD6: // SUB #
            printf("sub a,%02x\t", memoryRead(REG_PC+1)); break;
        case 0xDE: // SBC A,#
            printf("sbc a,%02x\t", memoryRead(REG_PC+1)); break;
        case 0xE6: // AND #		
            printf("and a,%02x\t", memoryRead(REG_PC+1)); break;
        case 0xEE: // XOR #
            printf("xor a,%02x\t", memoryRead(REG_PC+1)); break;
        case 0xF6: // OR #
            printf("or a,%02x\t", memoryRead(REG_PC+1)); break;
		case 0xFE: // CP #
			printf("cp %02x\t", memoryRead(REG_PC+1)); break;
			
		case 0x04: // INC rn
		case 0x0C:
		case 0x14:
		case 0x1C:	
		case 0x24:
		case 0x2C:
		case 0x3C:
			printf("inc %c\t\t", rname[dst]); break;			
		case 0x05: // DEC rn
		case 0x0D:
		case 0x15:
		case 0x1D:
		case 0x25:
		case 0x2D:
		case 0x3D:
            printf("dec %c\t\t", rname[dst]); break;	
            	
        case 0x34: // INC (HL)
            printf("inc (hl)\t"); break;	
		case 0x35: // DEC (HL)
			printf("dec (hl)\t"); break;	 		
				
// 16bit Arithmetic
		// ADD HL,rnn - flags: -,0,H,C
		case 0x09: printf("add hl,bc\t");  break;
		case 0x19: printf("add hl,de\t");  break;
		case 0x29: printf("add hl,hl\t");  break;
		case 0x39: printf("add hl,sp\t");  break;			
		case 0xE8: // ADD SP,#				
			printf("add sp,$%2x\t", memoryRead(REG_PC));			
			
		// INC Rnn - no flags affected
		case 0x03: printf("inc bc\t\t");  break;
		case 0x13: printf("inc de\t\t");  break;
		case 0x23: printf("inc hl\t\t");  break;
		case 0x33: printf("inc sp\t\t");  break;
			
		// DEC Rnn
		case 0x0B: printf("dec bc\t\t");  break;
		case 0x1B: printf("dec de\t\t");  break;
		case 0x2B: printf("dec hl\t\t");  break;
		case 0x3B: printf("dec sp\t\t");  break;			
		
		case 0x27: printf("daa\t\t"); break;		
		case 0x2F: printf("cpl\t\t"); break;			
		case 0x3F: printf("ccf\t\t"); break;			
		case 0x37: printf("scf\t\t"); break;			
		case 0x76: printf("halt\t\t"); break;
			
		case 0x10: printf("stop\t"); break;			
		case 0xF3: printf("di\t\t"); break;
		case 0xFB: printf("ei\t\t"); break;			
			
// Rotates & shifts		
		case 0x07: printf("rlca\t"); break;				
		case 0x17: printf("rla\t\t"); break;			
		case 0x0F: printf("rrca\t"); break;			
		case 0x1F: printf("rra\t\t"); break;
			
// Jumps			
		case 0xC3: printf("jp $%04x\t", memoryRead16(REG_PC+1)); break;			
		case 0xC2: printf("jp nz,$%04x\t", memoryRead16(REG_PC+1)); break;
		case 0xCA: printf("jp z,$%04x\t", memoryRead16(REG_PC+1)); break;
		case 0xD2: printf("jp nc,$%04x\t", memoryRead16(REG_PC+1)); break;
		case 0xDA: printf("jp c,$%04x\t", memoryRead16(REG_PC+1)); break;			
        case 0xE9: printf("jp (hl)\t"); break;
		
		case 0x18: printf("jr %04x\t", (REG_PC + 2) + (signed char)memoryRead(REG_PC+1)); break;			
		case 0x20: printf("jr nz,%04x\t", (REG_PC + 2) + (signed char)memoryRead(REG_PC+1)); break;
		case 0x28: printf("jr z,%04x\t", (REG_PC + 2) + (signed char)memoryRead(REG_PC+1)); break;			
        case 0x30: printf("jr nc,%04x\t", (REG_PC + 2) + (signed char)memoryRead(REG_PC+1)); break;
		case 0x38: printf("jr c,%04x\t", (REG_PC + 2) + (signed char)memoryRead(REG_PC+1)); break;			
			
// Calls			
		case 0xCD: printf("call $%04x\t", memoryRead16(REG_PC+1)); break;			
		case 0xC4: printf("call nz,$%04x\t", memoryRead16(REG_PC+1)); break;
		case 0xCC: printf("call z,$%04x\t", memoryRead16(REG_PC+1)); break;			
		case 0xD4: printf("call nc,$%04x\t", memoryRead16(REG_PC+1)); break;
		case 0xDC: printf("call c,$%04x\t", memoryRead16(REG_PC+1)); break;
			
// Restarts
		case 0xC7: // RST n
		case 0xCF:
		case 0xD7:
		case 0xDF:
		case 0xE7:
		case 0xEF:
		case 0xF7:
        case 0xFF:
            printf("rst %02x\t", opcode & 0x38); break;

// Returns	
		case 0xD9: printf("reti\t\t"); break;			
		case 0xC9: printf("ret\t\t"); break;			
		case 0xC0: printf("ret nz\t"); break;	
		case 0xC8: printf("ret z\t"); break;			
		case 0xD0: printf("ret nc\t\t"); break;	
		case 0xD8: printf("ret c\t\t"); break;            
            			
// Miscellaneous
		// SWAP, RLC, RL, RRC, RR, SLA, SRA, SRL, BIT, RST, SET
        case 0xCB:
			opcode = memoryRead(REG_PC + 1);
			src = (opcode & 0x07);
			dst = ((opcode >> 3) & 0x07);
            switch(opcode)
            {
                case 0x00: // RLC Rn
                case 0x01:
                case 0x02:
                case 0x03:
                case 0x04:
                case 0x05:
                case 0x07:
                    printf("rlc %c\t", rname[src]); break;		
                case 0x06: printf("rlc (hl)\t"); break;
                                
                case 0x08: // RRC Rn
                case 0x09:
                case 0x0A:
                case 0x0B:
                case 0x0C:
                case 0x0D:
                case 0x0F:
                    printf("rrc %c\t", rname[src]); break;		
                case 0x0E: printf("rrc (hl)\t"); break;
                    
                case 0x10: // RL Rn
                case 0x11:
                case 0x12:
                case 0x13:
                case 0x14:
                case 0x15:
                case 0x17:
                    printf("rl %c\t\t", rname[src]); break;		
                case 0x16: printf("rl (hl)\t"); break;
                
                case 0x18: // RR Rn
                case 0x19:
                case 0x1A:
                case 0x1B:
                case 0x1C:
                case 0x1D:
                case 0x1F:
                    printf("rr %c\t\t", rname[src]); break;		
                case 0x1E: printf("rr (hl)\t"); break;
                    
                case 0x20: // SLA Rn
                case 0x21:
                case 0x22:
                case 0x23:
                case 0x24:
                case 0x25:
                case 0x27:
                    printf("sla %c\t", rname[src]); break;		
                case 0x26: printf("sla (hl)\t"); break;
                    
                case 0x28: // SRA Rn
                case 0x29:
                case 0x2A:
                case 0x2B:
                case 0x2C:
                case 0x2D:
                case 0x2F:
                    printf("sra %c\t", rname[src]); break;		
                case 0x2E: printf("sra (hl)\t"); break;
                
                case 0x30: // SWAP Rn		
                case 0x31:
                case 0x32:
                case 0x33:
                case 0x34:
                case 0x35:
                case 0x37:
                    printf("swap %c\t", rname[src]); break;			
                case 0x36: printf("swap (hl)\t"); break;
                
                case 0x38: // SRL Rn
                case 0x39:
                case 0x3A:
                case 0x3B:
                case 0x3C:
                case 0x3D:
                case 0x3F:
                    printf("srl %c\t", rname[src]); break;			
                case 0x3E: printf("srl (hl)\t"); break;	
                    
                default:
                    switch(opcode & 0xC0)
                    {
                        case 0x40: printf("bit %u,%c\t\t", dst, rname[src]); break;				
                        case 0x80: printf("res %u,%c\t\t", dst, rname[src]); break;					
                        case 0xC0: printf("set %u,%c\t\t", dst, rname[src]); break;
                    }
                    break;
            }			
			
    }
    printf("\t%u\n", clockcounter);
}
