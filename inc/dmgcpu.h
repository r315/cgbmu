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
#define SERIAL_CYCLE	512

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

// SC bits
#define SC_TRF		(1<<7)
#define SC_CLKI		(1<<0)

#define VRAM_SIZE       0x2000      // 8k
#define IRAM_SIZE       0x2000      // 8k
#define OAM_SIZE        160         // 40 x 4
#define DMA_SIZE        160			
#define OAM_BASE        0xFE00

/**
 dmgcpu is big endian format, when indexing registers the bit0 must be xor'd with 1
*/
#define REG_ADDR(r)     ((uint8_t*)cpu + ((r) ^ 1))   // swap bit0 of r for endian
#define REG_INDEX(r)    *((uint8_t*)REG_ADDR(r))
#define DST_REG(op)     ((op >> 3) & 0x07)
#define SRC_REG(op)     (op & 0x07)


#define REG_A           cpu->A
#define REG_B           cpu->B
#define REG_C           cpu->C
#define REG_D           cpu->D
#define REG_E           cpu->E
#define REG_H           cpu->H
#define REG_L           cpu->L
#define REG_BC          cpu->BC
#define REG_DE          cpu->DE
#define REG_HL          cpu->HL
#define REG_SP          cpu->SP
#define REG_PC          cpu->PC
#define PSW             cpu->F


typedef struct obj_s{
    uint8_t y;
    uint8_t x;
    uint8_t pattern;
    uint8_t flags;      // |priority | Y Flip | X Flip | Palette number | - | - | - | - | 
}obj_t;

typedef struct tiledata_s {
	struct {
		uint8_t lsb;
		uint8_t msb;
	}line[8];
}tiledata_t;

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

typedef struct cpu_s{
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

    uint8_t IOP1;                // 0xFF00 P1
    uint8_t IOSB;                // 0xFF01 Serial transfer data
    uint8_t IOSC;                // 0xFF02 SIO Control
    uint8_t IODIV;               // 0xFF04 timer divider
    uint8_t IOTIMA;              // 0xFF05 timer counter
    uint8_t IOTMA;               // 0xFF06 timer modulo
    uint8_t IOTAC;               // 0xFF07 timer control
    uint8_t IOIF;                // 0xFF0F interrupt flag  0|0|0| P1 | Serial | Timer | Lcdc | V-blank |
    uint8_t IOLCDC;              // 0xFF40 lcd control
    uint8_t IOSTAT;              // 0xFF41 lcd status      
    uint8_t IOSCY;               // 0xFF42 scroll y
    uint8_t IOSCX;               // 0xFF43 scroll x
    uint8_t IOLY;                // 0xFF44 LY
    uint8_t IOLYC;               // 0xFF45 LY Compare
    uint8_t IODMA;               // 0xFF46
    uint8_t IOBGP;               // 0xFF47 Background palette
    uint8_t IOOBP0;              // 0xFF48 object palette 0
    uint8_t IOOBP1;              // 0xFF49 object palette 1
    uint8_t IOWY;                // 0xFF4A window Y
    uint8_t IOWX;                // 0xFF4B window x
    uint8_t IOIE;                // 0xFFFF Interrupt Enable  0|0|0| P1 | Serial | Timer | Lcdc | V-blank |

    const uint8_t *rom0;
    const uint8_t *rombank;

    uint8_t halt;
    uint8_t stopped;
    uint8_t IME;                 // interrupt master enable  Set and reset by DI,EI instructions
    
    uint32_t instr_cycles;
    uint32_t video_cycles;
    uint32_t timer_cycles;
    uint32_t timer_ovf;
	uint32_t div_cycles;
    uint32_t serial_cycles;
    uint32_t serial_bit;

	obj_t **visible_objs;       // use pointers to avoid 
	uint8_t *screen_line;       // out of bound offsets
	uint8_t *oam;               // when indexing far structure members
	uint8_t *hram;
	uint8_t *vram;
	uint8_t *iram;

    void *cartridge_data;
    uint8_t(*cartridgeRead)(struct cpu_s *cpu, uint16_t address);
    void(*cartridgeWrite)(struct cpu_s *cpu, uint16_t address, uint8_t data);

	uint8_t bgpal[4];
	uint8_t obj0pal[4];
	uint8_t obj1pal[4];

    obj_t *_visible_objs[11 + 1];
    uint8_t _screen_line[160];    // one line of pixels
    uint8_t _oam[OAM_SIZE];       // 0xFE00-0xFEBF
    uint8_t _hram[256];           // 0xFF80-0xFFFE
    uint8_t _vram[VRAM_SIZE];     // 0x8000-0x9FFF
    uint8_t _iram[IRAM_SIZE];     // 0xC000-0xBFFF
	uint8_t id;
}cpu_t;


uint8_t memoryRead(cpu_t *cpu, uint16_t address);
void memoryWrite(cpu_t *cpu, uint16_t address, uint8_t data);
uint16_t memoryRead16(cpu_t *cpu, uint16_t address);
void memoryWrite16(cpu_t *cpu, uint16_t address, uint16_t data);
void initCpu(cpu_t *cpu);
void interrupts(cpu_t *cpu);
void timer(cpu_t *cpu);
uint8_t joyPad(cpu_t *cpu);
void bootCpu(cpu_t *cpu);
void setInt(cpu_t *cpu, uint8_t irq);
void serial(cpu_t *cpu);
#endif
