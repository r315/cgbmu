/*


Window Tile Map  (LCDC.6) 0:9800-9BFF, 1:9C00-9FFF
BG Tile Map      (LCDC.3) 0:9800-9BFF, 1:9C00-9FFF
Tile Data        (LCDC.4) 0:8800-97FF, 1:8000-8FFF

  Mode 2  2_____2_____2_____2_____2_____2___________________2____
  Mode 3  _33____33____33____33____33____33__________________3___
  Mode 0  ___000___000___000___000___000___000________________000
  Mode 1  ____________________________________11111111111111_____


*/


#ifndef _video_h_
#define _video_h_

#define TILES_W 20
#define TILES_H 18
#define SCREEN_W 160
#define SCREEN_H 144

void video(void);
void lycIrq(void);
void scanline(void);
void scanOAM(void);
#endif
