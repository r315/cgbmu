#ifndef _video_stm32_h_
#define _video_stm32_h_

#include "video.h"

#define LCD_RED     LCD_COLOR_RED
#define LCD_YELLOW  LCD_COLOR_YELLOW
#define LCD_PINK    LCD_COLOR_ORANGE
#define LCD_TOMATO  LCD_COLOR_MAGENTA

void VIDEO_Init(void);
void VIDEO_Update(void);

#endif