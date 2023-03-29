#ifndef _decoder_h_
#define _decoder_h_

#define TABLE_DECODER 1

typedef uint8_t(*opcode_t)(void);

extern uint8_t instr_cycles;

void decode(void);
#endif /* _decoder_h_ */
