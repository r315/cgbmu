#ifndef _decoder_h_
#define _decoder_h_

#ifdef __cplusplus
extern "C" {
#endif

#include "dmgcpu.h"

#define TABLE_DECODER 1

typedef uint8_t(*opcode_t)(cpu_t *cpu);

void decode(cpu_t *cpu);

#ifdef __cplusplus
}
#endif

#endif /* _decoder_h_ */
