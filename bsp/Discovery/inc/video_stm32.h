#ifndef _video_stm32_h_
#define _video_stm32_h_

#include "video.h"

#define RED     LCD_COLOR_RED
#define YELLOW  LCD_COLOR_YELLOW
#define PINK    LCD_COLOR_ORANGE
#define TOMATO  LCD_COLOR_MAGENTA

void VIDEO_Init(void);
void VIDEO_Update(void);

#endif