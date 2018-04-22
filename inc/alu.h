#ifndef _func_h_
#define _func_h_

void alu(uint8_t op, uint8_t opb);
void daa(void);
//void incR16(char rh, char rl);
//void decR16(char rh, char rl);
void inc(uint8_t *r);
void dec(uint8_t *r);
void rlc(uint8_t *r);
void rrc(uint8_t *r);
void rl(uint8_t *r);
void rr(uint8_t *r);
void sla(uint8_t *r);
void sra(uint8_t *r);
void srl(uint8_t *r);
void swap(uint8_t *r);
void BiT(uint8_t b, uint8_t *r);
void res(uint8_t b, uint8_t *r);
void set(uint8_t b, uint8_t *r);
void addHL(uint16_t v);
uint16_t pop(void);
void push(uint16_t v);
void inc16(uint16_t *r);
void dec16(uint16_t *r);

void CBcodes(uint8_t op);

#endif
