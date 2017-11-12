

#include <stdint.h>

#ifdef WIN32
#include <conio.h>

char readLine(char *dst, uint8_t max){
static char *p;
int c;
	if(!p) p = dst;

	if(_kbhit()){
		c = getchar();
		if( c == '\n'){
            p = dst;
			return 1;
        }
        if(c >= ' ' && c <= 'z'){
            *(p++) = (char) c;
        }
	}
	return 0;
}

#else
#include <SDL2/SDL.h>

#include <termios.h>
#include <unistd.h>    
#include <sys/time.h>

static struct termios initial_settings, new_settings;
static int peek_character = -1, init = 0;
void init_keyboard(void){
    tcgetattr(0,&initial_settings);
    new_settings = initial_settings;
    new_settings.c_lflag &= ~ICANON;  //disable cannonical mode    
    new_settings.c_cc[VMIN] = 0;
    new_settings.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &new_settings);
    init = 1;
}

void close_keyboard(void){
    tcsetattr(0, TCSANOW, &initial_settings);
}

int kbhit(void){

unsigned char ch;
int nread;
    if (peek_character != -1) 
        return 1;

    if(!init) 
        init_keyboard();
        
    new_settings.c_cc[VMIN]=0;
    tcsetattr(0, TCSANOW, &new_settings);
    nread = read(0,&ch,1);
    new_settings.c_cc[VMIN]=1;
    tcsetattr(0, TCSANOW, &new_settings);
    if(nread == 1){
        peek_character = ch;
        return 1;
    }
   return 0;
}

int getch(void){
char ch;
    if(peek_character != -1)
    {
        ch = peek_character;
        peek_character = -1;
        return ch;
    }
    read(0,&ch,1);
    return ch;
}

char readLine(char *dst, uint8_t max) {
	static char *p;
	int c;
	if (!p) p = dst;

	if (kbhit()) {
		c = getch();
		if (c == '\n') {
            *p = '\0';
			p = dst;
			return 1;
		}
		if (c >= ' ' && c <= 'z') {
			*(p++) = (char)c;
		}
	}
	return 0;
}
#endif

