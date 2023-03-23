/*
+---------------
| IE Register
+--------------- FFFF
| High ram
+--------------- FF80
| Free
+--------------- FF4C
| I/O
+--------------- FF00
| FREE
+--------------- FEA0
| OAM
+--------------- FE00
| Echo Ram
+--------------- E000
| Ram 8kB
+--------------- C000
| cartrigbe Ram
+--------------- A000
| VRAM
+--------------- 8000
| cartrigbe
+--------------- 0000
http://verhoeven272.nl/cgi-bin/FSgz?fruttenboel%2FGameboy&Fruttenboel+GameBoy&GBtop&pandocs&GBcontent
http://imrannazar.com/GameBoy-Emulation-in-JavaScript:-Graphics
*/
#ifndef _dmgcpu_h_
#define _dmgcpu_h_

#include <stdint.h>

#define CLOCK_CYCLE     4
#define ONE_CYCLE	    (1 * CLOCK_CYCLE)
#define TWO_CYCLE	    (2 * CLOCK_CYCLE)
#define THREE_CYCLE     (3 * CLOCK_CYCLE)
#define FOUR_CYCLE	    (4 * CLOCK_CYCLE)
#define FIVE_CYCLE	    (5 * CLOCK_CYCLE)
#define SIX_CYCLE	    (6 * CLOCK_CYCLE)

//Flags
#define FZ              (1<<7)
#define FN              (1<<6)
#define FH              (1<<5)
#define FC              (1<<4)

//P1 bits
#define IOP14		    (1<<4)
#define IOP15		    (1<<5)

// LCDC bits
#define LCD_DISPLAY     (1<<7)	// Enable/Disable lcd controller
#define W_MAP           (1<<6)	// Window Tile map 0:9800, 1:9C00
#define W_DISPLAY       (1<<5)	// Enable window
#define BG_W_DATA       (1<<4)	// BG & Window Tile data 0:8800, 1:8000
#define BG_MAP          (1<<3)  // Backgound Tile map 0:9800, 1:9C00
#define OBJ_SIZE        (1<<2)	// Enable 8x16 Sprites
#define OBJ_DISPLAY     (1<<1)  // Enable Sprites
#define W_BG_DISPLAY    (1<<0)  // Enable BG and Window

// STAT bits
#define LYC_LY_IE       (1<<6)
#define OAM_IE          (1<<5)
#define VB_IE           (1<<4)
#define HB_IE           (1<<3)
#define LYC_LY_FLAG     (1<<2)

// IE bits
#define JOYPAD_IE       (1<<4)
#define SERIAL_IE       (1<<3)
#define TIMER_IE        (1<<2)
#define STAT_IE         (1<<1)
#define V_BLANK_IE      (1<<0)

// IF bits
#define JOYPAD_IF       (1<<4)
#define SERIAL_IF       (1<<3)
#define TIMER_IF        (1<<2)
#define LCDC_IF         (1<<1)
#define V_BLANK_IF      (1<<0)

// TAC bits
#define TIMER_STOP      (1<<2)

// Butons bits
#define J_RIGHT     (1<<0)
#define J_LEFT      (1<<1)
#define J_UP        (1<<2)
#define J_DOWN      (1<<3)

#define J_A         (1<<4)
#define J_B         (1<<5)
#define J_SELECT    (1<<6)
#define J_START     (1<<7)

#define IOP1_MASK   0xCF

/*
 15..8  7..0
+-----+-----+
|  B  |  C  |
+-----+-----+
|  D  |  E  |
+-----+-----+
|  H  |  L  |
+-----+-----+
|  A  |  F  |
+-----+-----+
|     SP    |
+-----------+
|     PC    |
+-----------+
*/

/**
 dmgcpu is big endian format, when indexing registers the bit0 must be xor'd with 1
*/
typedef struct _regs{
    union{        
        struct{
            uint8_t C;
            uint8_t B;
        };
        uint16_t BC;        
    };
    union{
        struct{
            uint8_t E;
            uint8_t D;
        };            
        uint16_t DE;
    };
    union{
        struct{
            uint8_t L;
            uint8_t H;
        };            
        uint16_t HL;
    };
    union{
        struct{            
            uint8_t A;
            uint8_t F;  // Z|N|H|C|0|0|0|0
        };            
        uint16_t AF;    // this reg is swaped for litle endian
    };
    uint16_t SP;
    uint16_t PC;
}Regs;

#define REGS_BASE       (&regs)
#define REG_ADDR(r)     ((uint8_t*)REGS_BASE + ((r) ^ 1))   // swap bit0 of r for endian
#define REG_INDEX(r)    *((uint8_t*)REG_ADDR((r)))  

#define PSW             regs.F
#define REG_A           regs.A
#define REG_B           regs.B
#define REG_C           regs.C
#define REG_D           regs.D
#define REG_E           regs.E
#define REG_F           regs.F
#define REG_H           regs.H
#define REG_L           regs.L

#define REG_AF          regs.AF
#define REG_BC          regs.BC
#define REG_DE          regs.DE
#define REG_HL          regs.HL
#define REG_SP          regs.SP
#define REG_PC          regs.PC

#define VRAM_SIZE       0x2000      // 8k
#define OAM_SIZE        160         // 40 x 4
#define DMA_SIZE        0x8C
#define OAM_BASE        0xFE00

extern Regs regs;

extern uint8_t *rom0;
extern uint8_t *rombank;
extern uint8_t vram[VRAM_SIZE];     // 0x8000-0x9FFF
extern uint8_t oam[OAM_SIZE];       // 0xFE00-0xFEBF
//extern uint8_t hram[256];         // 0xFF80-0xFFFE

extern uint8_t instr_cycles;
extern uint8_t halted, stopped;
extern uint8_t IME;                 // interrupt master enable  Set and reset by DI,EI instructions


extern uint8_t IOP1;                // 0xFF00 P1
extern uint8_t IODIV;               // 0xFF04 timer divider
extern uint8_t IOTIMA;              // 0xFF05 timer counter
extern uint8_t IOTMA;               // 0xFF06 timer modulo
extern uint8_t IOTAC;               // 0xFF07 timer control
extern uint8_t IOIF;                // 0xFF0F interrupt flag  0|0|0| P1 | Serial | Timer | Lcdc | V-blank |
extern uint8_t IOLCDC;              // 0xFF40 lcd control
extern uint8_t IOSTAT;              // 0xFF41 lcd status      
extern uint8_t IOSCY;               // 0xFF42 scroll y
extern uint8_t IOSCX;               // 0xFF43 scroll x
extern uint8_t IOLY;                // 0xFF44 LY
extern uint8_t IOLYC;               // 0xFF45 LY Compare
extern uint8_t IODMA;               // 0xFF46
extern uint8_t IOBGP;               // 0xFF47 Background palette
extern uint8_t IOOBP0;              // 0xFF48 object palette 0
extern uint8_t IOOBP1;              // 0xFF49 object palette 1
extern uint8_t IOWY;                // 0xFF4A window Y
extern uint8_t IOWX;                // 0xFF4B window x
extern uint8_t IOIE;                // 0xFFFF Interrupt Enable  0|0|0| P1 | Serial | Timer | Lcdc | V-blank |


uint8_t memoryRead(uint16_t address);
void memoryWrite(uint16_t address, uint8_t data);
uint16_t memoryRead16(uint16_t address);
void memoryWrite16(uint16_t address, uint16_t data);
void initCpu(void);
void interrupts(void);
void timer(void);
uint8_t joyPad(void);
void bootCpu(void);
#endif
