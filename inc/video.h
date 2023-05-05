/*

VRAM
+-----------------------+ A000
+-----------------------+ 9FFF
!                       |
+-----------------------+ 9C00 BG & Window Tile Map 1
+-----------------------+ 9BFF
|                       |
+-----------------------+ 9800 BG & Window Tile Map 0
+-----------------------+ 97FF
|                       |
+-----------------+ 8FFF|
|                 |     |
+-----------------+-----+ 8800 BG & Window Data Base 1
|                 |
+-----------------+ 8000 BG & Window & Obj Data Base 0


GBCPUman, page 54

Window Tile Map  (LCDC.6) 0:9800-9BFF, 1:9C00-9FFF
BG Tile Map      (LCDC.3) 0:9800-9BFF, 1:9C00-9FFF
Tile Data        (LCDC.4) 0:8800-97FF, 1:8000-8FFF

  Mode 2  2_____2_____2_____2_____2_____2___________________2____   80
  Mode 3  _33____33____33____33____33____33__________________3___   172
  Mode 0  ___000___000___000___000___000___000________________000   204
  Mode 1  ____________________________________11111111111111_____   4560

  - A cycle through all modes 2,3,0 takes 456 cycles (one line)
  - Mode 2 scan OAM                 80 cycles
  - Mode 3 scan VRAM                172 cycles
  . Mode 0 Hblank                   204 cycles
  - Mode 1 Vblank                   4560 cycles (10 lines)
  - Complete refresh 70224 cycles
  - The 144 lines takes 65664 cycles

Interrupts:
STAT Int enable : | LYC=LY | OAM | V-Blank | H-Blank | - | - | - |
  LYC = LY  Active on register match
  OAM       Active on Mode 2
  V-Blank   Active on Mode 1
  H-Blank   Active on Mode 0

*/

#ifdef __cplusplus
extern "C" {
#endif

#include "dmgcpu.h"

#ifndef _video_h_
#define _video_h_

#define SCREEN_W             160
#define SCREEN_H             144
#define SCREEN_VBLANK_LINES  10
#define SCREEN_H_TILES       SCREEN_W/8        // Horizontal Tiles
#define SCREEN_V_TILES       SCREEN_H/8        // Vertical Tiles

#define V_M0                 0
#define V_M1                 1
#define V_M2                 2
#define V_M3                 3
#define V_MODE_MASK          (V_M3)

#define V_M0_CYCLE           204
#define V_M2_CYCLE           80
#define V_M3_CYCLE           172
#define V_M1_CYCLE           (V_M0_CYCLE + V_M2_CYCLE + V_M3_CYCLE)
#define V_FRAME_CYCLE        (V_M1_CYCLE * (SCREEN_H + SCREEN_VBLANK_LINES))


#define VBLANK_LINES         8

#define MAX_LINE_OBJECTS     10
#define MAX_OBJECTS          40
#define SPRITE_W             8
#define SPRITE_H             8
#define SPRITE_X_OFFSET      8
#define SPRITE_Y_OFFSET      16
#define SPRITE_8x16_H        16
#define SPRITE_8x16_H_MASK    (SPRITE_8x16_H - 1)
#define SPRITE_8x16_LINE_MASK(_L) ((_L) & SPRITE_8x16_H_MASK)

#define TILE_DATA0_BASE      0x0000
#define TILE_DATA1_BASE      0x0800
#define TILE_DATA1_SIGNED_BASE 0x1000
#define TILE_MAP0_BASE       0x1800
#define TILE_MAP1_BASE       0x1C00
#define TILE_BYTES_SIZE      16
#define TILE_LINE_MASK       7
#define TILE_W               8
#define TILE_H               8
#define TILE_INDEX(x)        (x>>3)
#define TILE_LINE_INDEX(y)   (TILE_INDEX(y) << 5)  // => ((y/8) * 32)
#define TILE_LINE(y)         (y & TILE_LINE_MASK)
#define TILE_PIXEL(x)        (x & 7)

#define BG_H_TILES           32
#define BG_V_TILES           32
#define BG_SIZE_MASK         0x3FF
#define BG_V_LINES_MASK      255
#define BG_H_LINES_MASK      255
// Sprite flag bits
#define OBJECT_FLAG_PRIO     (1<<7)
#define OBJECT_FLAG_YFLIP    (1<<6)
#define OBJECT_FLAG_XFLIP    (1<<5)
#define OBJECT_FLAG_PAL      (1<<4)

uint8_t video(cpu_t *cpu);
void checkLine(cpu_t *cpu);
void scanline(cpu_t *cpu);
void scanOAM(cpu_t *cpu);
void writeLCDC(cpu_t *cpu, uint8_t newlcdc);
void writeSTAT(cpu_t *cpu, uint8_t newstat);
void writeLYC(cpu_t *cpu, uint8_t newstat);

#ifdef __cplusplus
}
#endif

#endif
