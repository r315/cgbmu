/*

GBCPUman, page 54

Window Tile Map  (LCDC.6) 0:9800-9BFF, 1:9C00-9FFF
BG Tile Map      (LCDC.3) 0:9800-9BFF, 1:9C00-9FFF
Tile Data        (LCDC.4) 0:8800-97FF, 1:8000-8FFF

  Mode 2  2_____2_____2_____2_____2_____2___________________2____   83
  Mode 3  _33____33____33____33____33____33__________________3___   175
  Mode 0  ___000___000___000___000___000___000________________000	207
  Mode 1  ____________________________________11111111111111_____   4560

  - A cycle through all modes takes 456 cycles
  - Vblank takes 4560 cycles
  - Complete refresh 70224 cycles
  - 144 lines takes 65664 cycles


*/


#ifndef _video_h_
#define _video_h_

#define SCREEN_W 160
#define SCREEN_H 144
#define TILES_W SCREEN_W/8
#define TILES_H SCREEN_H/8

#define V_M0 0
#define V_M1 1
#define V_M2 2
#define V_M3 3

#define V_M0_CYCLE 207
#define V_LINE_CYCLE 456
#define V_M2_CYCLE 83
#define V_M3_CYCLE 175

#define V_MODE_MASK 3

#define VBLANK_LINES 10

void video(void);
void lycIrq(void);
void scanline(void);
void scanOAM(void);
#endif
