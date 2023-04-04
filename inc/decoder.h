#ifndef _decoder_h_
#define _decoder_h_

#include "dmgcpu.h"

#define TABLE_DECODER 1

typedef uint8_t(*opcode_t)(cpu_t *cpu);

void decode(cpu_t *cpu);
#endif /* _decoder_h_ */
