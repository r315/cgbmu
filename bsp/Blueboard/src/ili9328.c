//#include <clock.h>
#include <lcd.h>
#include <ili9328.h>

static uint16_t _width, _height;
void (*LCD_WindowRotation)(uint16_t x, uint16_t y, uint16_t w, uint16_t h);

void LCD_Bkl(uint8_t state){
	if(state){
		LCD_BKL_ON;
	}else{
		LCD_BKL_OFF;
	}
}

//--------------------------------------------------------
//
//--------------------------------------------------------				 
#ifdef LCD_USE_ID
uint16_t LCD_ReadReg(uint8_t reg)
{
uint16_t dta = 0;

    LCDRS0;                         /* Set register */
	DATAPORT = 0;
	LCDWR0;LCDWR0;LCDWR1;
	DATAPORT =reg;
	LCDWR0;LCDWR0;LCDWR1;

    DATAPORTDIR &= ~0xFF;          /* change to input */
    DelayMs(3);
	LCDRS1;
	LCDRD0; DelayMs(3); dta = (DATAPORT << 8); LCDRD1;
	LCDRD0; DelayMs(3); dta |= (DATAPORT);     LCDRD1;
	DATAPORTDIR |= 0xFF;          /* change to out      put */
    DelayMs(3);
return dta;
}

uint16_t LCD_ReadID(void){
uint16_t id;
    if (LCD_ReadReg(0x04) == 0x8000) {
        if (LCD_ReadReg(0xD0) == 0x990000) {
          return 0x8357;
        }
    }

    id = LCD_ReadReg(0xD3);
    if (id == 0x9341) {
        return id;
    }
return LCD_ReadReg(0);    
}
#endif
//--------------------------------------------------------
//write data to LCD
//TODO: ver se � possivel alterar 2 sinals simultaneamente
//--------------------------------------------------------
void LCD_Data(uint16_t data)
{
	DATAPORT = data >> 8; // MSB
	LCDWR0;LCDWR0;LCDWR1;
	DATAPORT = data;      // LSB
	LCDWR0;LCDWR0;LCDWR1;	
}
//--------------------------------------------------------
//write instruction to LCD
//--------------------------------------------------------
void LCD_Command(uint8_t ins) 
{
	LCDRS0;
	DATAPORT = 0;
	LCDWR0;LCDWR0;LCDWR1;
	DATAPORT = ins;
	LCDWR0;LCDWR0;LCDWR1;
	LCDRS1;
}
//--------------------------------------------------------
//
//--------------------------------------------------------
void LCD_Fill(uint32_t count, uint16_t color){
	LCDRS1;
	while(count--){
		DATAPORT = color>>8;
		LCDWR0;LCDWR0;LCDWR1;
		DATAPORT = color;	
		LCDWR0;LCDWR0;LCDWR1;
	}
}

//-------------------------------------------------------------------
//	 Creates a write window inside GRAM
//-------------------------------------------------------------------
void LCD_Window(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    LCD_WindowRotation(x,y,w,h);
    LCD_Command(LCD_RW_GRAM);
}

void LCD_Window_N(uint16_t x, uint16_t y, uint16_t w, uint16_t h){
	LCD_Command(LCD_HOR_START_AD);
	LCD_Data(x);
	LCD_Command(LCD_HOR_END_AD);
	LCD_Data(x + w - 1);
	LCD_Command(LCD_VER_START_AD);
	LCD_Data(y);
	LCD_Command(LCD_VER_END_AD);
	LCD_Data(y + h - 1);
    
	LCD_Command(LCD_GRAM_HOR_AD);
	LCD_Data(x);
	LCD_Command(LCD_GRAM_VER_AD);
	LCD_Data(y);
}

void LCD_Window_L(uint16_t x, uint16_t y, uint16_t w, uint16_t h){
	LCD_Command(LCD_VER_START_AD);
	LCD_Data(x);
	LCD_Command(LCD_VER_END_AD);
	LCD_Data(x + w - 1);
	LCD_Command(LCD_HOR_START_AD);
	LCD_Data(y);
	LCD_Command(LCD_HOR_END_AD);
	LCD_Data(y + h - 1);
    
	LCD_Command(LCD_GRAM_VER_AD);
	LCD_Data(x);
	LCD_Command(LCD_GRAM_HOR_AD);
	LCD_Data(y);
}
//--------------------------------------------------------
//
//--------------------------------------------------------
void LCD_Pixel(uint16_t x, uint16_t y, uint16_t c){
	LCD_Window(x,y,1,1);
	LCD_Data(c);
}
//-----------------------------------------------------------------*/
//	 set VLE bit on GATE_SCAN_CTRL2(61h) first
//-------------------------------------------------------------------
void LCD_Scroll(uint16_t y)
{
	while (y < 0)
 		y += 320;
	while (y >= 320)
 		y -= 320;
	LCD_Command(LCD_GATE_SCAN_CTRL3);
 	LCD_Data(y);
}
//--------------------------------------------------------
// initialize lcd
//--------------------------------------------------------
void LCD_Init(void){
	
	LCD_IO_INIT;
	
	LCDCS1;LCDWR1;LCDRD1;
	LCDRST0;		 
	DelayMs(100);
	LCDRST1;
	LCDCS0;

	LCD_Command(0xE5);
	LCD_Data(0x8000); 					//set the internal vcore voltage
	LCD_Command(LCD_START_OSC);
	LCD_Data(0x0001); 					//start oscillator
	DelayMs(50);	

	LCD_Command(LCD_DRIV_OUT_CTRL);
	LCD_Data(SHIFT_DIR);
	LCD_Command(LCD_DRIV_WAV_CTRL);
	LCD_Data(0x0700); 					//set 1 line inversion
	
	LCD_Command(LCD_ENTRY_MOD);
	LCD_Data(VAL_ENTRY_MOD);			//set GRAM write direction, BGR=0

	LCD_Command(LCD_RESIZE_CTRL);
	LCD_Data(0x0000); 					//no resizing

	LCD_Command(LCD_DISP_CTRL2);
	LCD_Data(0x0202); 					//front & back porch periods = 2
	LCD_Command(LCD_DISP_CTRL3);
	LCD_Data(0x0000); 					
	LCD_Command(LCD_DISP_CTRL4);
	LCD_Data(0x0000); 					
	LCD_Command(LCD_RGB_DISP_IF_CTRL1);
	LCD_Data(0x0000); 					//select system interface				
	LCD_Command(LCD_FRM_MARKER_POS);
	LCD_Data(0x0000); 					
	LCD_Command(LCD_RGB_DISP_IF_CTRL2);
	LCD_Data(0x0000);					
	
	LCD_Command(LCD_POW_CTRL1);
	LCD_Data(0x0000);
	LCD_Command(LCD_POW_CTRL2);
	LCD_Data(0x0000); 					
	LCD_Command(LCD_POW_CTRL3);
	LCD_Data(0x0000);
	LCD_Command(LCD_POW_CTRL4);
	LCD_Data(0x0000);
	DelayMs(200);

	LCD_Command(LCD_POW_CTRL1);
	LCD_Data(0x17B0);
	LCD_Command(LCD_POW_CTRL2);
	LCD_Data(0x0137);
	DelayMs(50);

	LCD_Command(LCD_POW_CTRL3);
	LCD_Data(0x013C);
	DelayMs(50);

	LCD_Command(LCD_POW_CTRL4);
	LCD_Data(0x1400);
	LCD_Command(LCD_POW_CTRL7);
	LCD_Data(0x0007);
	DelayMs(50);	

	LCD_Command(LCD_GAMMA_CTRL1);
	LCD_Data(0x0007);
	LCD_Command(LCD_GAMMA_CTRL2);
	LCD_Data(0x0504);
	LCD_Command(LCD_GAMMA_CTRL3);
	LCD_Data(0x0703);
	LCD_Command(LCD_GAMMA_CTRL4);
	LCD_Data(0x0002);
	LCD_Command(LCD_GAMMA_CTRL5);
	LCD_Data(0x0707);
	LCD_Command(LCD_GAMMA_CTRL6);
	LCD_Data(0x0406);
	LCD_Command(LCD_GAMMA_CTRL7);
	LCD_Data(0x0006);
	LCD_Command(LCD_GAMMA_CTRL8);
	LCD_Data(0x0404);
	LCD_Command(LCD_GAMMA_CTRL9);
	LCD_Data(0x0700);
	LCD_Command(LCD_GAMMA_CTRL10);
	LCD_Data(0x0A08);

	LCD_Command(START_ADX);
	LCD_Data(0x0000);
	LCD_Command(END_ADX);
	LCD_Data(LCD_W - 1);
	LCD_Command(START_ADY);
	LCD_Data(0x0000);
	LCD_Command(END_ADY);
	LCD_Data(LCD_H - 1);

	LCD_Command(LCD_GATE_SCAN_CTRL1);
	LCD_Data(VAL_GATE_SCAN);
	LCD_Command(LCD_GATE_SCAN_CTRL2);
	LCD_Data(0x0003);
	LCD_Command(LCD_GATE_SCAN_CTRL3);
	LCD_Data(0x0000);

	LCD_Command(LCD_PART_IMG1_DISP_POS);
	LCD_Data(0x0000);
	LCD_Command(LCD_PART_IMG1_START_AD);
	LCD_Data(0x0000);
	LCD_Command(LCD_PART_IMG1_END_AD);
	LCD_Data(0x0000);
	LCD_Command(LCD_PART_IMG2_DISP_POS);
	LCD_Data(0x0000);
	LCD_Command(LCD_PART_IMG2_START_AD);
	LCD_Data(0x0000);
	LCD_Command(LCD_PART_IMG2_END_AD);
	LCD_Data(0x0000);

	LCD_Command(LCD_PANEL_IF_CTRL1);
	LCD_Data(0x0010);
	LCD_Command(LCD_PANEL_IF_CTRL2);
	LCD_Data(0x0000);
	LCD_Command(LCD_PANEL_IF_CTRL3);
	LCD_Data(0x0003);
	LCD_Command(LCD_PANEL_IF_CTRL4);
	LCD_Data(0x0110);
	LCD_Command(LCD_PANEL_IF_CTRL5);
	LCD_Data(0x0000);
	LCD_Command(LCD_PANEL_IF_CTRL6);
	LCD_Data(0x0000);

	LCD_Command(LCD_DISP_CTRL1);
	LCD_Data(0x0173);
	DelayMs(500);
	LCD_WindowRotation = LCD_Window_N;
}

uint16_t LCD_GetWidth(void){
    return _width;
}

uint16_t LCD_GetHeight(void){
    return _height;
}

void LCD_Rotation(uint8_t m) {

    switch (m) {
        case LCD_PORTRAIT:    
            LCD_Command(LCD_ENTRY_MOD);
            LCD_Data(0x0030	| BGR_BIT); //horizontal writing direction, address counter V:Incremet H:Increment
            LCD_Command(LCD_DRIV_OUT_CTRL);
            LCD_Data(0x0100);           //Source drive S720-S1
            LCD_Command(LCD_GATE_SCAN_CTRL1);
            LCD_Data(0xA700);           //Gate Scan inversion G320-G1
             _height = TFT_H;
             _width = TFT_W;
            LCD_WindowRotation = LCD_Window_N;
            break;
            
        case LCD_LANDSCAPE:
            LCD_Command(LCD_ENTRY_MOD);
            LCD_Data(0x0038	| BGR_BIT); //Vertical writing direction, address counter V:Incremet H:Increment
            LCD_Command(LCD_DRIV_OUT_CTRL);
            LCD_Data(0x0000);           //Source drive S1-S720
            LCD_Command(LCD_GATE_SCAN_CTRL1);
            LCD_Data(0xA700);           //Gate Scan inversion G320-G1
             _height = TFT_W;
             _width = TFT_H;
            LCD_WindowRotation = LCD_Window_L;
            break;
            
        case LCD_REVERSE_PORTRAIT:
            LCD_Command(LCD_ENTRY_MOD);
            LCD_Data(0x0030	| BGR_BIT);
            LCD_Command(LCD_DRIV_OUT_CTRL);
            LCD_Data(0x0000);           //Source drive S1-S720
            LCD_Command(LCD_GATE_SCAN_CTRL1);
            LCD_Data(0x2700);           //Gate Scan normal G1-G320
             _height = TFT_H;
             _width = TFT_W;
            LCD_WindowRotation = LCD_Window_N;
            break;
            
        case LCD_REVERSE_LANDSCAPE:
            LCD_Command(LCD_ENTRY_MOD);
            LCD_Data(0x0038 | BGR_BIT);
            LCD_Command(LCD_DRIV_OUT_CTRL);
            LCD_Data(0x0100);           //Source drive S720-S1
            LCD_Command(LCD_GATE_SCAN_CTRL1);
            LCD_Data(0x2700);
             _height = TFT_W;
             _width = TFT_H; 
            LCD_WindowRotation = LCD_Window_L;
            break;

        default:
        	return;
    }  
}



