

#include "lcd.h"
#include "debug.h"
#include "dmgcpu.h"
#include "graphics.h"
#include "video.h"
#include "io.h"
#include "disassembler.h"

#define GetTicks() SDL_GetTicks()
#define FPS_ROW 0
#define REGISTERS_ROW 11

uint16_t breakpoint = 0xc300;
uint8_t key;
char _logline = 0;
int fps;

void decode(void);
void init_keyboard(void);
char readLine(char *dst, uint8_t max);

void updateFps(void){
static uint32_t fpsupdatetick = 0;
    fps++;
    
	if(GetTicks() > fpsupdatetick)
	{
		printVal(175,FPS_ROW,"Fps ",fps,10,4);
		fps = 0;
		fpsupdatetick = GetTicks() + 1000;
	}
}


//----------------------------------------------------*/
//
//------------------------------------------------------
int printNum(int x, int y,unsigned int v, char radix, char digitos)
{
unsigned char i=0,c,dig[16];	
	do{
		c = (unsigned char)(v % radix);
		if (c >= 10)c += 7;		
		c += '0';
		v /= radix;
		dig[i++]=c;
	} while(v);
	
	for(c=i;c<digitos;c++)
		x= drawChar(x,y,'0');
	
	while(i--)
	x = drawChar(x,y,dig[i]);		
	return x;		
}
//----------------------------------------------------*/
//
//------------------------------------------------------
int printVal(int x, int y,char *name, int v, char radix, char digitos)
{	
	x = drawString(x,y,name);
	x = printNum(x,y,v,radix,digitos);	 
	return x;
}
//----------------------------------------------------*/
//
//------------------------------------------------------
void dumpRegisters(void)
{
	LCD_Push();
#if 0
	putchar('\n');
	fprintf(stdout,"AF = %.4X\n\r",REG_A << 8| REG_F);
	printf("BC = %.4X\n\r",REG_BC);
	printf("DE = %.4X\n\r",REG_DE);
	printf("HL = %.4X\n\r",REG_HL);
	printf("SP = %.4X\n\r",REG_SP);
	printf("PC = %.4X\n\r",REG_PC);
	printf("Z | N | H | C \n\r");
	printf("--+---+---+--- \n\r");
	printf("%u | %u | %u | %u\n\r",
		(REG_F & FZ) ? 1 : 0,
		(REG_F & FN) ? 1 : 0,
		(REG_F & FH) ? 1 : 0,
		(REG_F & FC) ? 1 : 0
	);
#else	
	setFcolor(YELLOW);
	//setAttribute(g_double);
//	setFont(BOLD);
	printVal(175,0,"af ",REG_A << 8| REG_F ,16,4);
	printVal(175,9,"bc ",REG_BC,16,4);
	printVal(175,18,"de ",REG_DE,16,4);
	printVal(175,27,"hl ",REG_HL,16,4);
	printVal(175,36,"sp ",REG_SP,16,4);
	printVal(175,45,"pc ",REG_PC,16,4);
	printVal(175,64,"LCDC ",(IOLCDC & 8),16,2);
//	printVal(175,74,"P1 ",IOP1,16,4);
	printVal(175,84,"ly ",IOLY,16,4);

	printVal(170,94,"TIMA ",IOTIMA,16,2);
#endif
	LCD_Pop();
}
//----------------------------------------------------*/
//
//------------------------------------------------------
void dumpMemory(unsigned short addr, unsigned short siz)
{
unsigned short i,j,x;
unsigned char p = 160;
#ifdef NO_SDL
	for(i=0;i<siz;i++)
		printf("0x%.4X = 0x%.2X\n",addr+i,memoryRead(addr+i));	
#else
	setFcolor(TOMATO);
	drawString(5*8,p-8," 0 1 2 3 4 5 6 7 8 9 A C B D E F");
	setFcolor(RED);
	for(i=0; i < (siz>>4); i++)
	{
		x = printNum(0,p+(i*8),addr,16,4)+8;
		for(j=0; j < 16; j++)
	 		x = printNum(x,p+(i*9),memoryRead(addr++),16,2);	
	}
#endif	
}
//----------------------------------------------------*/
//
//------------------------------------------------------
void printStackFrame(void)
{
unsigned short x,_sp;	
signed char i;
_sp = regs.SP;
setFcolor(PINK);
	
	for(i=7; i >-1 ; i--)
	{	
		x = printNum(0,i*8,_sp,16,4);
		x = drawChar(x,i*8,':');
		x = printNum(x,i*8,memoryRead(_sp+1),16,2);
		x = printNum(x,i*8,memoryRead(_sp),16,2);
		_sp +=2;	
	}
}
//----------------------------------------------------*/
//
//------------------------------------------------------
void logInfo(char* text)
{
	drawString(0,_logline,text);
	_logline = (_logline+9)%LCD_H;
}
//----------------------------------------------------*/
//avalilable debug commands
//bp <addr hex>
//------------------------------------------------------
void debugCommans(uint8_t *st){
#ifdef __EMU__	
	static char line[10];
	char *cmd;
	if(readLine(line, 10)){
		
		if(*line == '\0' || *line == ' ' ){ //cartridge return pressed
			*st = 3;	   //step
			return;
		}

		cmd = strtok(line, " ");
		if(cmd == NULL) 
			return;		//empty line
		
		if(!strcmp(cmd, "bp")){
			cmd = strtok(NULL, " ");
			breakpoint = strtol(cmd, &cmd, 16);
			*st = 0;	//resume
		}

		if(!strcmp(cmd, "run")){
			*st = 0;	 //resume 
			return;
		}
	}
#endif
}
//----------------------------------------------------*/
//main cpu run
//------------------------------------------------------
void step(){
static uint8_t stepping = 0;
	
	debugCommans(&stepping);
	
	if( REG_PC == breakpoint && !stepping){
		stepping = 2;
	}

	if(stepping){
		if(stepping == 2){
			dumpRegisters();
			disassemble();	        
			stepping = 1;
			return;
		}

		if(key != J_A && stepping == 1){
			SDL_Delay(30);			
			return;
		}			
		else{
			stepping = 2;
		}
	}	
	decode();	
	timer();	
	if(!video()){
		frameReady = 0;
		LCD_Window(0,0,160,144);
	}	
	interrupts();	        
}

extern uint16_t machine_cycles;
void debug(void){	
	//machine_cycles = 2; 
	while(key != 255){
		step();		
		key = readJoyPad();		
	}
}




//-----------------------------------------
//
//-----------------------------------------
void oneFrame(void)
{
	LCD_Window(160,96,160,144);

#if 1	
	frameReady = 0;	
	do{
		cycles = 0;
		interrupts();		     
    	decode();
		timer();	
	}while(!video());	
#else
	for (IOLY = 0; IOLY < 144; IOLY++)
	{		
		IOSTAT |= 2;  // mode 2
	    lycIrq();
	    runCpu(80);

	    IOSTAT |= 3;  // mode 3
	    runCpu(172);
	    _scanline();
    
	    IOSTAT &= 0xFC; // mode 0
	   	lycIrq();
	    runCpu(204);
	}	
	
	IOSTAT |= 1;  // mode 1
       
	if(IOIE & V_BLANK_IE)
		IOIF |= V_BLANK_IF;
						
	if((IOIE & STAT_IE) && (IOSTAT & VB_IE))
		IOIF |= STAT_IF;
	
	for (IOLY = 144; IOLY < 154; IOLY++)
	{
    	lycIrq();
	    runCpu(456);			    
	}

#endif
}
//-----------------------------------------
//
//-----------------------------------------
void runCpu(int nTicks)
{
	while (cycles < nTicks)
	{
		interrupts();
    	decode();    	
    } 
    cycles -= nTicks;
    timer();
}
