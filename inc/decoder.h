#ifndef _decoder_h_
#define _decoder_h_

typedef uint8_t(*opcode_t)(void);

#define TABLE_DECODER 1

void decode(void);
#endif /* _decoder_h_ */
