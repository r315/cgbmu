
TARGET   =gbemu
LIBEMBPATH = $(HOME)/Dropbox/Projects/software/libemb
SRCSPATH =src ../../src ../../dmgcpu $(LIBEMBPATH)/drv/tft $(LIBEMBPATH)/display
OBJSPATH =.
INCSPATH =. ../../inc inc $(LIBEMBPATH)/include
INCSPATH +=SDL2/include

CSRCS    =debug.c disassembler.c tests.c testdata.c main.c readline.c lcd.c display.c fonts.c
CSRCS   +=decoder.c alu.c dmgcpu.c cartridge.c video.c cgbmu.c
LIBS     =-lSDL2main -lSDL2
SYMBOLS  =-D__DEBUG__ -D__EMU__

CC      =arm-linux-gnueabihf-gcc
GCC     =gcc 
RM      =rm -f
CFLAGS  =-Og -g -Wall -gdwarf-2 -gstrict-dwarf $(addprefix -I, $(INCSPATH)) $(SYMBOLS)
OBJS    =$(addprefix $(OBJSPATH)/, $(CSRCS:.c=.o))
VPATH   +=$(SRCSPATH)

all: $(TARGET) 
	
clean: 
	@${RM} $(TARGET) $(OBJSPATH)/*.o


$(TARGET): $(OBJS) $(OBJSPATH)/lcdsdl2.o
	$(GCC) $(OBJS) $(OBJSPATH)/lcdsdl2.o $(LIBS) -o $(TARGET)
#@./$(TARGET) -r ../../../roms/mario.gb
	
$(OBJSPATH)/%.o : %.c
	@echo "-----" $< "---->" $@
	@$(GCC) $(CFLAGS) -c $< -o $@
	
rpi: $(OBJS) $(OBJSPATH)/lcdfb.o 
	$(CC) $(OBJS) $(OBJSPATH)/lcdfb.o -lSDLmain -lSDL -o $(TARGET)
	



