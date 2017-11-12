#ifndef _decoder_h_
#define _decoder_h_

void CBcodes(unsigned char op);
void ADD(unsigned char opb);
void ADC(unsigned char opb);
void SUB(unsigned char opb);
void SBC(unsigned char opb);
void AND(unsigned char opb);
void XOR(unsigned char opb);
void OR(unsigned char opb);
void CP(unsigned char opb);
void DAA(void);
void decode(void);
#endif /* _decoder_h_ */
