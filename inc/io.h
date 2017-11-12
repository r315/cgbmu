#ifndef _io_h_
#define _io_h_


#define J_RIGHT     (1<<0)
#define J_LEFT      (1<<1)
#define J_UP        (1<<2)
#define J_DOWN      (1<<3)

#define J_A         (1<<0)
#define J_B         (1<<1)
#define J_SELECT    (1<<2)
#define J_START     (1<<3)


#define IOP1_MASK   0x0F


uint8_t joyPad(void);
uint8_t readJoyPad(void);

#endif /* _io_h_ */