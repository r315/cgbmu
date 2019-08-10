
TARGET   =gbemu
LIBEMBPATH =$(HOME)/Dropbox/Projects/software/libemb
SRCSPATH =src ../../src ../../dmgcpu $(LIBEMBPATH)/drv/tft $(LIBEMBPATH)/display
OBJSPATH =.
INCSPATH =. ../../inc inc $(LIBEMBPATH)/include
INCSPATH +=SDL2/include

CSRCS    =debug.c disassembler.c tests.c testdata.c main.c readline.c lcd.c display.c fonts.c lcdsdl2.o
CSRCS   +=decoder.c alu.c dmgcpu.c cartridge.c video.c cgbmu.c
LIBS     =-lSDL2main -lSDL2
SYMBOLS  =-D__DEBUG__ -D__EMU__
CFLAGS  =-Og -g -Wall -gdwarf-2 -gstrict-dwarf $(addprefix -I, $(INCSPATH)) $(SYMBOLS)
LDFLAGS =-LSDL2/lib/x64
OBJS    =$(addprefix $(OBJSPATH)/, $(CSRCS:.c=.o))

CC      =arm-linux-gnueabihf-gcc
GCC     =gcc 
RM      =rm -f
VPATH   +=$(SRCSPATH)

all: $(TARGET) 
	
clean: 
	@${RM} $(TARGET) $(OBJSPATH)/*.o


$(TARGET): $(OBJS)
#	$(GCC) $(LDFLAGS) $(OBJS) $(LIBS) -o $(TARGET)
	$(GCC) $(OBJS) -o $(TARGET) $(shell sdl2-config --cflags --libs)
#@./$(TARGET) -r ../../../roms/mario.gb
	
$(OBJSPATH)/%.o : %.c
	@echo "-----" $< "---->" $@
	@$(GCC) $(CFLAGS) -c $< -o $@
	
rpi: $(OBJS) $(OBJSPATH)/lcdfb.o 
	$(CC) $(OBJS) $(OBJSPATH)/lcdfb.o -lSDLmain -lSDL -o $(TARGET)
	



